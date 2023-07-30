#include "JSRepl.h"

#include <QVBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QJSEngine>
#include <QQmlEngine>
#include <QKeyEvent>


JSRepl::JSRepl(QHash<QString, QObject *> objects, QWidget *parent)
    : QWidget{parent}
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("JavaScript Console"));

    QVBoxLayout *layout = new QVBoxLayout(this);
    m_textEdit = new QTextEdit(this);
    m_lineEdit = new JSLineEdit(this);
    layout->addWidget(m_textEdit);
    layout->addWidget(m_lineEdit);

    m_engine = new QJSEngine(this);
    m_engine->installExtensions(QJSEngine::TranslationExtension | QJSEngine::GarbageCollectionExtension);

    JSConsole *console = new JSConsole(m_textEdit, this);
    QJSValue consoleObject = m_engine->newQObject(console);
    m_engine->globalObject().setProperty("console", consoleObject);

    auto i = objects.constBegin();
    while (i != objects.constEnd()) {
        QQmlEngine::setObjectOwnership(i.value(), QQmlEngine::CppOwnership);
        m_engine->globalObject().setProperty(i.key(), m_engine->newQObject(i.value()));
        ++i;
    }

    connect(m_lineEdit, &QLineEdit::returnPressed, this, &JSRepl::execute);

    m_lineEdit->setFocus();
}

QSize JSRepl::sizeHint() const
{
    return QSize(600, 400);
}

void JSRepl::execute()
{
    QString code = m_lineEdit->text();
    appendLog(">> " + code);
    QJSValue result = m_engine->evaluate(code);
    appendLog(result.toString());
    m_lineEdit->clear();
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

JSConsole::JSConsole(QTextEdit *textEdit, QObject *parent)
    : QObject(parent)
    , m_textEdit(textEdit)
{
}

void JSConsole::log(QString message)
{
    m_textEdit->append(message);
}
