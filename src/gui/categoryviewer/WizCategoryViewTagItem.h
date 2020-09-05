#ifndef GUI_CATEGORYVIEWER_WIZCATEGORYVIEWTAGITEM_H
#define GUI_CATEGORYVIEWER_WIZCATEGORYVIEWTAGITEM_H

#include "gui/categoryviewer/WizCategoryViewItemBase.h"


class WizCategoryViewAllTagsItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewAllTagsItem(WizExplorerApp& app, const QString& strName, const QString& strKbGUID);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const;
    virtual QString getSectionName();
    virtual int getSortOrder() const { return 21; }
};

class WizCategoryViewTagItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewTagItem(WizExplorerApp& app, const WIZTAGDATA& tag, const QString& strKbGUID);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual bool acceptDrop(const WIZDOCUMENTDATA& data) const;
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const;
    virtual bool dragAble() const { return true; }
    virtual void drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy = false);
    virtual void drop(const WizCategoryViewItemBase* pItem);

    virtual QTreeWidgetItem *clone() const;

    void reload(WizDatabase& db);
    void setTagPosition(int nPos);
    const WIZTAGDATA& tag() const { return m_tag; }

private:
    WIZTAGDATA m_tag;
};


#endif // GUI_CATEGORYVIEWER_WIZCATEGORYVIEWTAGITEM_H