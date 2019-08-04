#include "IWizDocumentListCtrl.h"

#include <QVariant>
#include <QDebug>

#include "WizDocumentListView.h"
#include "share/WizDatabase.h"

IWizDocumentListCtrl::IWizDocumentListCtrl(WizDocumentListView* docListView, QObject* parent)
    : QObject(parent)
    , m_documentListView(docListView)
{

}

/**
 * @brief Show documents in list view.
 * 
 * @param documentGUIDs 
 */
void IWizDocumentListCtrl::SetDocuments(QStringList documentGUIDs)
{
    if (documentGUIDs.isEmpty())
        return;
    m_documentListView->setDocuments(documentGUIDs);
}
