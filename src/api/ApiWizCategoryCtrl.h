#ifndef API_APIWIZCATEGORYCTRL_H
#define API_APIWIZCATEGORYCTRL_H

#include <QObject>

class WizFolder;
class WizCategoryView;

class ApiWizCategoryCtrl : public QObject
{
    Q_OBJECT
private:
    WizCategoryView* m_categoryView;

public:
    ApiWizCategoryCtrl(WizCategoryView* cv, QObject* parent);

    // for the present, just noly one api.
    Q_INVOKABLE WizFolder* SelectedFolder();
};

#endif // API_APIWIZCATEGORYCTRL_H
