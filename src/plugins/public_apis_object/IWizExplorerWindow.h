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

    QObject* CurrentDocumentBrowserObject();
    Q_PROPERTY(QObject* CurrentDocumentBrowserObject READ CurrentDocumentBrowserObject NOTIFY CurrentDocumentBrowserObjectChanged)

    Q_INVOKABLE QObject *CurrentDocument();
    Q_INVOKABLE void ViewDocument(QObject *pWizDocument, bool vbOpenInNewTab = true);

signals:
    void CurrentDocumentBrowserObjectChanged();
};

#endif // WIZEXPLORERWINDOW_H
