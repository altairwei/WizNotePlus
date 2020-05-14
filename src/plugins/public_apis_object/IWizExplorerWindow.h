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

    Q_INVOKABLE QObject *CurrentDocument();
    Q_INVOKABLE QObject* CurrentDocumentBrowserObject();
    Q_INVOKABLE void ViewDocument(QObject *pWizDocument, bool vbOpenInNewTab = true);
    Q_INVOKABLE void ViewAttachment(QObject *pWizDocumentAttachment);

};

#endif // WIZEXPLORERWINDOW_H
