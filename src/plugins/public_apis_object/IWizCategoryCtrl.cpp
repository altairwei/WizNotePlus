#include "IWizCategoryCtrl.h"

#include "gui/categoryviewer/WizCategoryView.h"

IWizCategoryCtrl::IWizCategoryCtrl(WizCategoryView* cv, QObject* parent)
    : QObject(parent)
    , m_categoryView(cv)
{

}

WizFolder* IWizCategoryCtrl::SelectedFolder()
{
    return m_categoryView->SelectedFolder();
}
