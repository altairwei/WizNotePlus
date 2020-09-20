#ifndef GUI_CATEGORYVIEWER_WIZCATEGORYVIEWSECTIONITEM_H
#define GUI_CATEGORYVIEWER_WIZCATEGORYVIEWSECTIONITEM_H

#include "gui/categoryviewer/WizCategoryViewItemBase.h"


class WizCategoryViewSectionItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewSectionItem(WizExplorerApp& app, const QString& strName, int sortOrder);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos) { Q_UNUSED(pCtrl); Q_UNUSED(pos); }
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument) {  Q_UNUSED(arrayDocument); }
    virtual int getItemHeight(int nHeight) const;
    virtual int getSortOrder() const { return m_sortOrder; }
    void reset(const QString& sectionName, int sortOrder);

    virtual void drawItemBody(QPainter* p, const QStyleOptionViewItem* vopt) const;
    virtual void drawExtraBadge(QPainter* p, const QStyleOptionViewItem* vopt) const;
protected:
    int m_sortOrder;
};

#endif // GUI_CATEGORYVIEWER_WIZCATEGORYVIEWSECTIONITEM_H