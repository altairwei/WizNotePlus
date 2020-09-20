#include "ApiWizCategoryCtrl.h"

#include "gui/categoryviewer/WizCategoryView.h"

ApiWizCategoryCtrl::ApiWizCategoryCtrl(WizCategoryView* cv, QObject* parent)
    : QObject(parent)
    , m_categoryView(cv)
{

}

WizFolder* ApiWizCategoryCtrl::SelectedFolder()
{
    return m_categoryView->SelectedFolder();
}
