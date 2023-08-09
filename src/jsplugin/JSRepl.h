#ifndef JSREPL_H
#define JSREPL_H

#include <QWidget>
#include <QLineEdit>
#include <QJSValueList>

class QTextEdit;
class QLineEdit;
class QJSEngine;
class QKeyEvent;

class JSLineEdit;
class JSReplSearchBox;
class JSRepl : public QWidget
{
    Q_OBJECT
public:
    explicit JSRepl(QWidget *parent = nullptr) : JSRepl({}, parent) {};
    JSRepl(QHash<QString, QObject *> objects, QWidget *parent = nullptr);

    QSize sizeHint() const override;

private:
    QString stringify(const QJSValue &value);

public slots:
    void execute();
    void loadScript();
    void appendLog(const QString &message, bool addBackground = false);

private slots:
    void toggleSearchBox();
    void printResult(const QJSValue &value);
    void printResult(const QJSValueList &vlist);

private:
    QJSEngine *m_engine;
    QTextEdit *m_textEdit;
    JSLineEdit *m_lineEdit;
    JSReplSearchBox *m_searchBox;
};

class JSLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    JSLineEdit(QWidget *parent = nullptr);

    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onReturnPressed();

private:
    QStringList history;
    int currentIndex;
};

class JSGlobalFunction : public QObject
{
    Q_OBJECT

public:
    JSGlobalFunction(QTextEdit *textEdit, QObject *parent = nullptr);

public slots:
    void print(QJSValue args);
    void help();

signals:
    void printRequested(const QJSValue &value);
    void printRequested(const QJSValueList &vlist);
    void logRequested(const QString &msg, bool add);

private:
    QTextEdit *m_textEdit;
};

class JSReplSearchBox : public QWidget
{
    Q_OBJECT

public:
    JSReplSearchBox(QTextEdit *parentTextEdit, QWidget *parent = nullptr);
    void setFocusOnLineEdit();

public slots:
    void findNext();
    void findPrevious();

private:
    void highlight(const QTextCursor &cursor);

    QTextEdit *textEdit;
    QLineEdit *lineEdit;
};

#endif // JSREPL_H
