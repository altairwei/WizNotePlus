#ifndef API_APIWIZEXPLORERWINDOW_H
#define API_APIWIZEXPLORERWINDOW_H

#include <QObject>

class WizMainWindow;

class ApiWizExplorerWindow : public QObject
{
    Q_OBJECT
private:
    WizMainWindow* m_mainWindow;

public:
    ApiWizExplorerWindow(WizMainWindow* mw, QObject* parent);

    Q_INVOKABLE QObject *CurrentDocument();
    Q_INVOKABLE QObject* CurrentDocumentBrowserObject();
    Q_INVOKABLE void ViewDocument(QObject *pWizDocument, bool vbOpenInNewTab = true);
    Q_INVOKABLE void ViewAttachment(QObject *pWizDocumentAttachment);
    Q_INVOKABLE void OpenMessageConsole();

};

#endif // API_APIWIZEXPLORERWINDOW_H
