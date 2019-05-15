#ifndef IWIZCATEGORYCTRL_H
#define IWIZCATEGORYCTRL_H

#include <QObject>

class WizFolder;
class WizCategoryView;

class IWizCategoryCtrl : public QObject
{
    Q_OBJECT
private:
    WizCategoryView* m_categoryView;

public:
    IWizCategoryCtrl(WizCategoryView* cv, QObject* parent);

    // for the present, just noly one api.
    Q_INVOKABLE WizFolder* SelectedFolder();
};

#endif // WIZCATEGORYCTRL_H
