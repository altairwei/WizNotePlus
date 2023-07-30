#include "ApiWizExplorerWindow.h"
#include "WizMainWindow.h"
#include "gui/documentviewer/WizDocumentView.h"
#include "database/WizDatabaseManager.h"
#include "database/WizDatabase.h"

ApiWizExplorerWindow::ApiWizExplorerWindow(WizMainWindow* mw, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mw)
{

}

QObject* ApiWizExplorerWindow::CurrentDocumentBrowserObject()
{
    return m_mainWindow->CurrentDocumentBrowserObject();
}

QObject *ApiWizExplorerWindow::CurrentDocument()
{
    WizDocumentView *docView = m_mainWindow->documentView();
    if (!docView) 
        return nullptr;

    WizDatabase &db = WizDatabaseManager::instance()->db(
        docView->note().strKbGUID
    );
    WIZDOCUMENTDATA data;
    if (db.documentFromGuid(docView->note().strGUID, data)) {
        return new WizDocument(db, data, this);
    } else {
        return nullptr;
    }
}

void ApiWizExplorerWindow::ViewDocument(QObject *pWizDocument, bool vbOpenInNewTab /* = true */)
{
    WizDocument *doc = qobject_cast<WizDocument *>(pWizDocument);
    if (doc) {
        WIZDOCUMENTDATAEX docData(doc->data());
        m_mainWindow->viewDocument(docData);
    }
}

void ApiWizExplorerWindow::ViewAttachment(QObject *pWizDocumentAttachment)
{
    WizDocumentAttachment *att = qobject_cast<WizDocumentAttachment *>(pWizDocumentAttachment);
    if (att) {
        m_mainWindow->viewAttachment(att->data());
    }
}

void ApiWizExplorerWindow::OpenMessageConsole()
{
    m_mainWindow->on_actionConsole_triggered();
}
