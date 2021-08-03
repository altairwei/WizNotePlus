#include "WizDocumentListViewSectionItem.h"

#include "utils/WizStyleHelper.h"

#include "WizDocumentListView.h"

WizDocumentListViewSectionItem::WizDocumentListViewSectionItem(const WizDocumentListViewSectionData& data,
                                                                 const QString& text, int docCount)
    : WizDocumentListViewBaseItem(0, WizDocumentListType_Section)
    , m_data(data)
    , m_text(text)
    , m_documentCount(docCount)
{

}

bool WizDocumentListViewSectionItem::operator<(const QListWidgetItem& other) const
{
//    qDebug() << "compare text : " << text() << "  with ; " << other.text();

    if (other.type() == WizDocumentListType_Document)
    {
        const WizDocumentListViewDocumentItem* docItem = dynamic_cast<const WizDocumentListViewDocumentItem*>(&other);
        if(docItem->document().isAlwaysOnTop())
            return false;

        return compareWithDocumentItem(docItem);
    }
    else
    {
        const WizDocumentListViewSectionItem* secItem = dynamic_cast<const WizDocumentListViewSectionItem*>(&other);
        switch (m_nSortingType) {
        case SortingByCreatedTime:
        case SortingByModifiedTime:
        case SortingByAccessedTime:
            return m_data.date > secItem->sectionData().date;
        case -SortingByCreatedTime:
        case -SortingByModifiedTime:
        case -SortingByAccessedTime:
            return m_data.date < secItem->sectionData().date;
        case SortingByTitle:
//        case SortingByLocation:
        {
            if (m_data.strInfo.startsWith(secItem->sectionData().strInfo) &&
                    m_data.strInfo.length() > secItem->sectionData().strInfo.length())
            {
//                qDebug() << "compare sec and sec 1.1 , loc : " << m_data.strInfo << "  other loc : " << secItem->sectionData().strInfo << "result : " << false;
                return false;
            }
            bool result = (m_data.strInfo.localeAwareCompare(secItem->sectionData().strInfo) > 0);
//            qDebug() << "compare sec and sec 1.1 , loc : " << m_data.strInfo << "  other loc : " << secItem->sectionData().strInfo << "result : " << result;
            return result;
        }
        case -SortingByTitle:
        case -SortingByLocation:
        case SortingByLocation:     //NOTE: 按文件夹排序目前只提供一种排序方向，降序排列需要重新整理算法
        {
//            if (m_data.strInfo.startsWith(secItem->sectionData().strInfo) &&
//                    m_data.strInfo.length() > secItem->sectionData().strInfo.length())
//            {
//                qDebug() << "compare sec and sec 2 , loc : " << m_data.strInfo << "  other loc : " << secItem->sectionData().strInfo << "result : " << 1;
//                return true;
//            }

            bool result = (m_data.strInfo.localeAwareCompare(secItem->sectionData().strInfo) < 0);
//            qDebug() << "compare sec and sec 2 , loc : " << m_data.strInfo << "  other loc : " << secItem->sectionData().strInfo << "result : " << result;
            return result;
        }
        case SortingBySize:
        {
            bool result = sectionData().sizePair.second > secItem->sectionData().sizePair.second;
            return result;
        }
        case -SortingBySize:
        {
            bool result = sectionData().sizePair.first < secItem->sectionData().sizePair.first;
            return result;
        }
        default:
            Q_ASSERT(0);
        }
    }

    return true;
}

void WizDocumentListViewSectionItem::draw(QPainter* p, const QStyleOptionViewItem* vopt, int nViewType) const
{
    p->save();
    p->fillRect(vopt->rect, Utils::WizStyleHelper::listViewSectionItemBackground());

    p->setPen(Utils::WizStyleHelper::listViewSectionItemText());
    QFont font;
    font.setPixelSize(WizSmartScaleUI(12));
    p->setFont(font);
    QRect rc = vopt->rect;
    rc.setLeft(rc.x() + Utils::WizStyleHelper::listViewItemHorizontalPadding());
    p->drawText(rc, Qt::AlignLeft | Qt::AlignVCenter, m_text);

    rc = vopt->rect;
    rc.setRight(rc.right() - Utils::WizStyleHelper::listViewItemHorizontalPadding());
    p->drawText(rc, Qt::AlignRight | Qt::AlignVCenter, QString::number(m_documentCount));

    p->setPen(Utils::WizStyleHelper::listViewItemSeperator());
    p->drawLine(vopt->rect.x(), vopt->rect.bottom(), vopt->rect.right(), vopt->rect.bottom());
    p->restore();
}

bool WizDocumentListViewSectionItem::compareWithDocumentItem(const WizDocumentListViewDocumentItem* docItem) const
{
    switch (m_nSortingType) {
    case SortingByCreatedTime:
        // default compare use create time     //There is a bug in Qt sort. if two items have same time, use title to sort.
        return compareYearAndMothOfDate(docItem->document().tCreated.date(), sectionData().date) <= 0;
    case -SortingByCreatedTime:
        return compareYearAndMothOfDate(docItem->document().tCreated.date(), sectionData().date) >= 0;
    case SortingByModifiedTime:
        return compareYearAndMothOfDate(docItem->document().tDataModified.date(), sectionData().date) <= 0;
    case -SortingByModifiedTime:
        return compareYearAndMothOfDate(docItem->document().tDataModified.date(), sectionData().date) >= 0;
    case SortingByAccessedTime:
        return compareYearAndMothOfDate(docItem->document().tAccessed.date(), sectionData().date) <= 0;
    case -SortingByAccessedTime:
        return compareYearAndMothOfDate(docItem->document().tAccessed.date(), sectionData().date) >= 0;
    case SortingByTitle:
        if (docItem->document().strTitle.toUpper().trimmed().startsWith(sectionData().strInfo))
            return true;
        return docItem->document().strTitle.localeAwareCompare(sectionData().strInfo) <= 0;
    case -SortingByTitle:
        if (docItem->document().strTitle.toUpper().trimmed().startsWith(sectionData().strInfo))
            return true;
        return docItem->document().strTitle.localeAwareCompare(sectionData().strInfo) >= 0;
//    case SortingByLocation:
//    {
//        if (docItem->documentLocation().startsWith(sectionData().strInfo) &&
//                docItem->documentLocation().length() > sectionData().strInfo.length())
//        {
//            qDebug() << "compare sec and doc 1.1 , loc : " << m_data.strInfo << "  other loc : " << docItem->documentLocation() << "result : " << true;
//            return false;
//        }
//        bool result = docItem->documentLocation().localeAwareCompare(sectionData().strInfo) <= 0;
//        qDebug() << "compare sec and doc 1.2 , loc : " << m_data.strInfo << "  other loc : " << docItem->documentLocation() << "result : " << result;
//        return result;
//    }
        //按照路径排序的时候子文件夹排在父文件夹的后面
    case SortingByLocation:
    case -SortingByLocation:
    {
//        if (docItem->documentLocation().startsWith(sectionData().strInfo))
//        {
//            qDebug() << "compare sec and doc 2 , loc : " << m_data.strInfo << "  other loc : " << docItem->documentLocation() << "result : " << -1;
//            return false;
//        }
        bool result = docItem->documentLocation().localeAwareCompare(sectionData().strInfo) >= 0;
//        qDebug() << "compare sec and doc 2 , loc : " << m_data.strInfo << "  other loc : " << docItem->documentLocation() << "result : " << result;
        return result;
    }
    case SortingBySize:
    {
        bool result = docItem->documentSize() <= sectionData().sizePair.second;
        return result;
    }
    case -SortingBySize:
    {
        bool result = docItem->documentSize() > sectionData().sizePair.first;
        return result;
    }
    default:
        Q_ASSERT(0);
    }

    return true;
}