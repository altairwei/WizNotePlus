#include "ApiWizDocumentListCtrl.h"

#include <QVariant>
#include <QDebug>

#include "gui/doclistviewer/WizDocumentListView.h"
#include "database/WizDatabase.h"

ApiWizDocumentListCtrl::ApiWizDocumentListCtrl(WizDocumentListView* docListView, QObject* parent)
    : QObject(parent)
    , m_documentListView(docListView)
{

}

/**
 * @brief Show documents in list view.
 * 
 * @param documentGUIDs 
 */
void ApiWizDocumentListCtrl::SetDocuments(QStringList documentGUIDs)
{
    if (documentGUIDs.isEmpty())
        return;
    m_documentListView->setDocuments(documentGUIDs);
}
