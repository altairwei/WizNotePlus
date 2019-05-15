#ifndef IWIZEXPLORERWINDOW_H
#define IWIZEXPLORERWINDOW_H

#include <QObject>

class WizMainWindow;

class IWizExplorerWindow : public QObject
{
    Q_OBJECT
private:
    WizMainWindow* m_mainWindow;

public:
    IWizExplorerWindow(WizMainWindow* mw, QObject* parent);
};

#endif // WIZEXPLORERWINDOW_H
