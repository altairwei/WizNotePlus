#include "IWizDocumentListCtrl.h"

#include "WizDocumentListView.h"

IWizDocumentListCtrl::IWizDocumentListCtrl(WizDocumentListView* docListView, QObject* parent)
    : QObject(parent)
    , m_documentListView(docListView)
{

}
