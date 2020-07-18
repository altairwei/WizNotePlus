#ifndef IWIZDOCUMENTLISTCTRL_H
#define IWIZDOCUMENTLISTCTRL_H

#include <QObject>
#include <QStringList>

class WizDocumentListView;

class IWizDocumentListCtrl : public QObject
{
    Q_OBJECT
    
private:
    WizDocumentListView* m_documentListView;

public:
    IWizDocumentListCtrl(WizDocumentListView* docListView, QObject* parent);

    Q_INVOKABLE void SetDocuments(QStringList documentGUIDs);
};

#endif // WIZDOCUMENTLISTCTRL_H
