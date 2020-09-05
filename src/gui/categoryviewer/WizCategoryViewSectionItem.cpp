#include "WizCategoryViewSectionItem.h"

#include "utils/WizStyleHelper.h"


/* ------------------------------ CWizCategoryViewSectionItem ------------------------------ */

WizCategoryViewSectionItem::WizCategoryViewSectionItem(WizExplorerApp& app, const QString& strName, int sortOrder)
    : WizCategoryViewItemBase(app, strName, "", Category_SectionItem)
    , m_sortOrder(sortOrder)
{
    setFlags(Qt::NoItemFlags); // user can not interact with it.
    setText(0, strName);
}

int WizCategoryViewSectionItem::getItemHeight(int /*nHeight*/) const
{    
    return WizSmartScaleUI(32);
}
void WizCategoryViewSectionItem::reset(const QString& sectionName, int sortOrder)
{
    m_strName = sectionName;
    m_sortOrder = sortOrder;
    //
    setText(0, sectionName);
}

void WizCategoryViewSectionItem::drawItemBody(QPainter *p, const QStyleOptionViewItem *vopt) const
{
    QString str = vopt->text;
    QRect rc(vopt->rect);
    rc.adjust(-12, 2, 0, 0);
    QFont font = p->font();
    Utils::WizStyleHelper::fontSection(font);
    Utils::WizStyleHelper::drawSingleLineText(p, rc, str, Qt::AlignVCenter, Utils::WizStyleHelper::treeViewSectionItemText(), font);
}

void WizCategoryViewSectionItem::drawExtraBadge(QPainter* p, const QStyleOptionViewItem* vopt) const
{
//    QRect rc = vopt->rect;
//    rc.setTop(rc.bottom());
//    p->fillRect(rc, Utils::StyleHelper::treeViewItemBottomLine());

    WizCategoryViewItemBase::drawExtraBadge(p, vopt);
}