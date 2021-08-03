#include "WizDocumentListViewBaseItem.h"

#include "WizDocumentListView.h"

int compareYearAndMothOfDate(const QDate& dateleft, const QDate& dateRight)
{
   if (dateleft.year() != dateRight.year())
       return dateleft.year() > dateRight.year() ? 1 : -1;

   if (dateleft.month() != dateRight.month())
       return dateleft.month() > dateRight.month() ? 1 : -1;

   return 0;
}

WizDocumentListViewBaseItem::WizDocumentListViewBaseItem(QObject* parent, WizDocumentListItemType type)
    : QListWidgetItem(0, type)
    , QObject(parent)
    , m_nSortingType(SortingByCreatedTime)
    , m_nLeadInfoState(0)
{

}

void WizDocumentListViewBaseItem::setSortingType(int type)
{ 
    m_nSortingType = type;
}

void WizDocumentListViewBaseItem::setLeadInfoState(int state)
{
    m_nLeadInfoState = state;
}
