#ifndef API_APIWIZDOCUMENTLISTCTRL_H
#define API_APIWIZDOCUMENTLISTCTRL_H

#include <QObject>
#include <QStringList>

class WizDocumentListView;

class ApiWizDocumentListCtrl : public QObject
{
    Q_OBJECT
    
private:
    WizDocumentListView* m_documentListView;

public:
    ApiWizDocumentListCtrl(WizDocumentListView* docListView, QObject* parent);

    Q_INVOKABLE void SetDocuments(QStringList documentGUIDs);
};

#endif // API_APIWIZDOCUMENTLISTCTRL_H
