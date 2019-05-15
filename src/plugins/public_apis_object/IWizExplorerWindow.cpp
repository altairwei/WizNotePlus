#include "IWizExplorerWindow.h"

IWizExplorerWindow::IWizExplorerWindow(WizMainWindow* mw, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mw)
{

}
