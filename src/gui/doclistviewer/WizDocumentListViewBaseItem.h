#ifndef GUI_DOCLISTVIEWER_WIZDOCUMENTLISTVIEWBASEITEM_H
#define GUI_DOCLISTVIEWER_WIZDOCUMENTLISTVIEWBASEITEM_H

#include <QObject>
#include <QListWidgetItem>

#include "share/WizObject.h"

enum WizDocumentListItemType
{
    WizDocumentListType_Document = QListWidgetItem::UserType + 1,
    WizDocumentListType_Section
};

struct WizDocumentListViewItemData
{
    int nType;
    WIZDOCUMENTDATAEX doc;
    WIZABSTRACT thumb;

    QString location;    // use to sort by location
    QStringList infoList; // for second line info drawing (auto change when sorting type change)

    // only used for group or message document
    qint64 nMessageId;
    int nReadStatus;    // 0: not read 1: read
    QString strAuthorId; // for request author avatar
};

struct WizDocumentListViewSectionData
{
    QDate date;
    QPair<int, int> sizePair;
    QString strInfo;
};


class WizDocumentListViewBaseItem : public QObject, public QListWidgetItem
{
    Q_OBJECT

public:
    explicit WizDocumentListViewBaseItem(QObject* parent, WizDocumentListItemType type);

    virtual void setSortingType(int type);
    virtual void setLeadInfoState(int state);

    // drawing
    virtual void draw(QPainter* p, const QStyleOptionViewItem* vopt, int nViewType) const {}

protected:
    int m_nSortingType;      // upercase : -  decrease : +
    int m_nLeadInfoState;
};

#endif // GUI_DOCLISTVIEWER_WIZDOCUMENTLISTVIEWBASEITEM_H