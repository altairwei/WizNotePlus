#include "IWizCategoryCtrl.h"

#include "WizCategoryView.h"

IWizCategoryCtrl::IWizCategoryCtrl(WizCategoryView* cv, QObject* parent)
    : QObject(parent)
    , m_categoryView(cv)
{

}

WizFolder* IWizCategoryCtrl::SelectedFolder()
{
    return m_categoryView->SelectedFolder();
}
