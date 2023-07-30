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

JSRepl::JSRepl(QHash<QString, QObject *> objects, QWidget *parent)
    : QWidget{parent}
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("JavaScript Console"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_textEdit = new QTextEdit(this);
    layout->addWidget(m_textEdit);

    QHBoxLayout *hLayout = new QHBoxLayout();
    m_lineEdit = new JSLineEdit(this);
    QPushButton *addButton = new QPushButton("+", this);
    hLayout->addWidget(m_lineEdit);
    hLayout->addWidget(addButton);
    layout->addLayout(hLayout);

    m_engine = new QJSEngine(this);
    m_engine->installExtensions(QJSEngine::AllExtensions);

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
    appendLog(">> " + code);
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
            appendLog(">> Execute " + fileName);
            QJSValue result = m_engine->evaluate(code);
            printResult(result);
        }
    }
}

void JSRepl::appendLog(const QString &message)
{
    m_textEdit->append(message);
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_textEdit->setTextCursor(cursor);
    m_textEdit->ensureCursorVisible();
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

void JSPrintFunction::print(QVariantList args)
{
    QString message;
    for (const QVariant &arg : args) {
        if (!message.isEmpty())
            message += " ";
        message += arg.toString();
    }

    m_textEdit->append(message);
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_textEdit->setTextCursor(cursor);
    m_textEdit->ensureCursorVisible();
}
