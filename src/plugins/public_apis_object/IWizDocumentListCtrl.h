#ifndef IWIZDOCUMENTLISTCTRL_H
#define IWIZDOCUMENTLISTCTRL_H

#include <QObject>
#include <QVariantList>

class WizDocumentListView;

class IWizDocumentListCtrl : public QObject
{
    Q_OBJECT
    
private:
    WizDocumentListView* m_documentListView;

public:
    IWizDocumentListCtrl(WizDocumentListView* docListView, QObject* parent);

    Q_INVOKABLE void SetDocuments(QVariantList documents);
};

#endif // WIZDOCUMENTLISTCTRL_H
