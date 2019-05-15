#ifndef IWIZDOCUMENTLISTCTRL_H
#define IWIZDOCUMENTLISTCTRL_H

#include <QObject>

class WizDocumentListView;

class IWizDocumentListCtrl : public QObject
{
    Q_OBJECT
private:
    WizDocumentListView* m_documentListView;
public:
    IWizDocumentListCtrl(WizDocumentListView* docListView, QObject* parent);
};

#endif // WIZDOCUMENTLISTCTRL_H
