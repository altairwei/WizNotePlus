#ifndef GUI_TABBROWSER_ABSTRACTTABPAGE_H
#define GUI_TABBROWSER_ABSTRACTTABPAGE_H

#include <QWidget>
#include <QString>

class AbstractTabPage : public QWidget
{
    Q_OBJECT

public:
    AbstractTabPage(QWidget *parent = nullptr) : QWidget(parent) { }
    virtual ~AbstractTabPage() = 0 { }

    virtual void RequestClose() = 0;
    virtual QString Title() = 0;

signals:
    void pageCloseRequested();
    void titleChanged(const QString &title);

};

#endif // GUI_TABBROWSER_ABSTRACTTABPAGE_H