#include "JSRepl.h"

#include <QVBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QJSEngine>
#include <QQmlEngine>
#include <QKeyEvent>
#include <QPushButton>
#include <QFileDialog>
#include <QTextStream>
#include <QShortcut>
#include <QTextFrame>

JSRepl::JSRepl(QHash<QString, QObject *> objects, QWidget *parent)
    : QWidget{parent}
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("JavaScript Console"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    //m_textEdit->document()->setDocumentMargin(0);
    layout->addWidget(m_textEdit);

    m_searchBox = new JSReplSearchBox(m_textEdit, this);
    layout->addWidget(m_searchBox);

    QHBoxLayout *hLayout = new QHBoxLayout();
    m_lineEdit = new JSLineEdit(this);
    QPushButton *addButton = new QPushButton("+", this);
    hLayout->addWidget(m_lineEdit);
    hLayout->addWidget(addButton);
    layout->addLayout(hLayout);

    m_engine = new QJSEngine(this);
    m_engine->installExtensions(QJSEngine::AllExtensions);
    m_engine->globalObject().setProperty("global", m_engine->globalObject());

    JSPrintFunction *printFunction = new JSPrintFunction(m_textEdit, this);
    QJSValue printObject = m_engine->newQObject(printFunction);
    m_engine->globalObject().setProperty("_printObject", printObject);
    m_engine->evaluate("function print(...args) { _printObject.print(args); }");

    auto i = objects.constBegin();
    while (i != objects.constEnd()) {
        QQmlEngine::setObjectOwnership(i.value(), QQmlEngine::CppOwnership);
        m_engine->globalObject().setProperty(i.key(), m_engine->newQObject(i.value()));
        ++i;
    }

    connect(m_lineEdit, &QLineEdit::returnPressed, this, &JSRepl::execute);
    connect(addButton, &QPushButton::clicked, this, &JSRepl::loadScript);
    addButton->setToolTip(tr("Select a JavaScript file to execute."));

    QShortcut *findShortcut = new QShortcut(QKeySequence::Find, this);
    connect(findShortcut, &QShortcut::activated, this, &JSRepl::toggleSearchBox);

    connect(printFunction, &JSPrintFunction::printRequested, this, &JSRepl::appendLog);

    m_lineEdit->setFocus();
}

QSize JSRepl::sizeHint() const
{
    return QSize(600, 400);
}

void JSRepl::printResult(const QJSValue &value)
{
    if (value.isError()) {
        appendLog("Uncaught exception at line " +
                  value.property("lineNumber").toString() + ":\n" +
                  value.toString());
    } else {
        appendLog(value.toString());
    }
}

void JSRepl::execute()
{
    QString code = m_lineEdit->text();
    appendLog(">> " + code, true);
    QJSValue result = m_engine->evaluate(code);
    printResult(result);
    m_lineEdit->clear();
}

void JSRepl::loadScript()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open JavaScript File"), QString(), "JavaScript Files (*.js)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&file);
            QString code = in.readAll();
            appendLog(">> Execute " + fileName, true);
            QJSValue result = m_engine->evaluate(code);
            printResult(result);
        }
    }
}

void JSRepl::appendLog(const QString &message, bool addBackground)
{
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);

    QTextBlockFormat blockFormat;
    if (addBackground) {
        blockFormat.setBackground(QColor(240, 240, 240));
        cursor.setBlockFormat(blockFormat);
    } else {
        cursor.setBlockFormat(blockFormat);
    }

    cursor.insertText(message);
    cursor.insertBlock();

    m_textEdit->setTextCursor(cursor);
    m_textEdit->ensureCursorVisible();
}

void JSRepl::toggleSearchBox()
{
    if (m_searchBox->isVisible()) {
        m_searchBox->hide();
    } else {
        m_searchBox->show();
        m_searchBox->setFocusOnLineEdit();
    }
}

JSLineEdit::JSLineEdit(QWidget *parent)
    : QLineEdit(parent)
    , currentIndex(-1)
{
    connect(this, &QLineEdit::returnPressed, this, &JSLineEdit::onReturnPressed);
}

void JSLineEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Up) {
        if (currentIndex > 0) {
            currentIndex--;
            setText(history.at(currentIndex));
        }
    } else if (event->key() == Qt::Key_Down) {
        if (currentIndex < history.size() - 1) {
            currentIndex++;
            setText(history.at(currentIndex));
        } else {
            currentIndex = history.size();
            clear();
        }
    } else {
        QLineEdit::keyPressEvent(event);
    }
}

void JSLineEdit::onReturnPressed()
{
    history.append(text());
    currentIndex = history.size();
}

JSPrintFunction::JSPrintFunction(QTextEdit *textEdit, QObject *parent)
    : QObject(parent)
    , m_textEdit(textEdit)
{
}

void JSPrintFunction::print(QJSValue args)
{
    if (!args.isArray()) {
        Q_EMIT printRequested(args.toString());
        return;
    }

    QString message;
    const int length = args.property("length").toInt();
    for (int i = 0; i < length; ++i) {
        if (!message.isEmpty())
            message += " ";
        message += args.property(i).toString();
    }

    Q_EMIT printRequested(message);
}

JSReplSearchBox::JSReplSearchBox(QTextEdit *parentTextEdit, QWidget *parent)
    : QWidget(parent), textEdit(parentTextEdit)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    lineEdit = new QLineEdit(this);
    QPushButton *findNextButton = new QPushButton(tr("Find Next"), this);
    QPushButton *findPreviousButton = new QPushButton(tr("Find Previous"), this);
    QPushButton *closeButton = new QPushButton(tr("Close"), this);
    layout->addWidget(lineEdit);
    layout->addWidget(findNextButton);
    layout->addWidget(findPreviousButton);
    layout->addWidget(closeButton);

    layout->setContentsMargins(0, 0, 0, 0);

    connect(lineEdit, &QLineEdit::returnPressed, this, &JSReplSearchBox::findNext);
    connect(findNextButton, &QPushButton::clicked, this, &JSReplSearchBox::findNext);
    connect(findPreviousButton, &QPushButton::clicked, this, &JSReplSearchBox::findPrevious);
    connect(closeButton, &QPushButton::clicked, this, &JSReplSearchBox::hide);

    QShortcut *findPreviousShortcut = new QShortcut(QKeySequence("Shift+Return"), this);
    connect(findPreviousShortcut, &QShortcut::activated, this, &JSReplSearchBox::findPrevious);

    QShortcut *closeShortcut = new QShortcut(QKeySequence("Escape"), this);
    connect(closeShortcut, &QShortcut::activated, this, &JSReplSearchBox::hide);

    hide();
}

void JSReplSearchBox::findNext()
{
    QString text = lineEdit->text();
    bool found = textEdit->find(text);
    if (!found) {
        // If not found, move the cursor to the start of the document and try again
        QTextCursor cursor = textEdit->textCursor();
        cursor.movePosition(QTextCursor::Start);
        textEdit->setTextCursor(cursor);
        textEdit->find(text);
    }
}

void JSReplSearchBox::findPrevious()
{
    QString text = lineEdit->text();
    bool found = textEdit->find(text, QTextDocument::FindBackward);
    if (!found) {
        // If not found, move the cursor to the end of the document and try again
        QTextCursor cursor = textEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        textEdit->setTextCursor(cursor);
        textEdit->find(text, QTextDocument::FindBackward);
    }
}

void JSReplSearchBox::setFocusOnLineEdit()
{
    lineEdit->setFocus();
}