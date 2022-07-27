﻿#ifndef CORE_TITLEEDIT_H
#define CORE_TITLEEDIT_H

#include <QLineEdit>

class QCompleter;
class QModelIndex;
class QInputMethodEvent;

class WizDocumentView;

class WizTitleEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit WizTitleEdit(QWidget *parent);
    void resetTitle(const QString& strTitle);
    void setReadOnly(bool b);

    void setCompleter(QCompleter* completer);
    QCompleter* completer() const { return c; }

public slots:
    void onTitleEditingFinished();
    void setText(const QString &text);

signals:
    void titleEdited(QString strTitle);
    void newTitleRequest(const QString &newTitle);

protected:
    QSize sizeHint() const;
    virtual void keyPressEvent(QKeyEvent* e);
    virtual void contextMenuEvent(QContextMenuEvent* e);

private:
    QCompleter* c;
    QChar m_separator;

    void updateCompleterPopupItems(const QString& completionPrefix);
    QString textUnderCursor();
    QChar charBeforeCursor();

private Q_SLOTS:
    void onInsertCompletion(const QModelIndex &index);    
    void onTextEdit(const QString & text);
    void onTextChanged(const QString & text);
};


#endif // CORE_TITLEEDIT_H
