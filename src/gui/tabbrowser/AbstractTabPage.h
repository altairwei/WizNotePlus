#ifndef GUI_TABBROWSER_ABSTRACTTABPAGE_H
#define GUI_TABBROWSER_ABSTRACTTABPAGE_H

#include <QWidget>
#include <QString>
#include <QKeyEvent>
#include <QVector>
#include <QList>

class AbstractTabPage : public QWidget
{
    Q_OBJECT

public:
    AbstractTabPage(QWidget *parent = nullptr) : QWidget(parent) { }
    virtual ~AbstractTabPage() = 0;

    virtual void RequestClose() = 0;
    virtual QString Title() = 0;
    virtual QList<QAction *> TabContextMenuActions();

signals:
    void pageCloseRequested();
    void titleChanged(const QString &title);

};

#endif // GUI_TABBROWSER_ABSTRACTTABPAGE_H