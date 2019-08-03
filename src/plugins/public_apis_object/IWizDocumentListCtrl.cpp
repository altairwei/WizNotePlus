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

void IWizDocumentListCtrl::SetDocuments(QVariantList documents)
{
    // Check WizDocument exists
    if (documents.isEmpty())
        return;
    // Get document data
    CWizDocumentDataArray arrayDocument;
    for (const QVariant &docVar : documents) {
        if (QObject *object = docVar.value<QObject *>()) {
            WizDocument *doc = qobject_cast<WizDocument *>(object);
            if (doc) {
                arrayDocument.push_back(doc->data());
            } else {
                //TODO: Warning
                qWarning() << "SetDocuments: Arguments can not be convert to WizDocument";
            }
        } else {
            //TODO: Warning
            // QObject of Array in JavaScript cant not be unwrapped in C++ side. 
            qWarning() << "SetDocuments: No QObject in arguments";
        }
    }
    // Show document in list
    m_documentListView->setDocuments(arrayDocument);
}
