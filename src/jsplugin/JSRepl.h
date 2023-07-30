#ifndef JSREPL_H
#define JSREPL_H

#include <QWidget>
#include <QLineEdit>

class QTextEdit;
class QLineEdit;
class QJSEngine;
class QKeyEvent;

class JSLineEdit;
class JSRepl : public QWidget
{
    Q_OBJECT
public:
    explicit JSRepl(QWidget *parent = nullptr) : JSRepl({}, parent) {};
    JSRepl(QHash<QString, QObject *> objects, QWidget *parent = nullptr);

    QSize sizeHint() const override;

public slots:
    void execute();

private:
    QJSEngine *m_engine;
    QTextEdit *m_textEdit;
    JSLineEdit *m_lineEdit;
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

#endif // JSREPL_H
