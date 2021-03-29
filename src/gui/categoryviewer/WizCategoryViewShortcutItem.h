#ifndef GUI_CATEGORYVIEWER_WIZCATEGORYVIEWSHORTCUTITEM_H
#define GUI_CATEGORYVIEWER_WIZCATEGORYVIEWSHORTCUTITEM_H

#include "gui/categoryviewer/WizCategoryViewItemBase.h"


class WizCategoryViewShortcutItem;
class WizCategoryViewShortcutRootItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewShortcutRootItem(WizExplorerApp& app, const QString& strName);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual void getDocuments(WizDatabase& db,
                              CWizDocumentDataArray& arrayDocument);

    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual void drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy = false);
    virtual void drop(const WizCategoryViewItemBase* pItem);
    virtual bool acceptDrop(const WIZDOCUMENTDATA& /*data*/) const {return true;}
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const;

    WizCategoryViewShortcutItem* addItemToShortcuts(const WizCategoryViewItemBase* pItem);
    WizCategoryViewShortcutItem* addDocumentToShortcuts(const WIZDOCUMENTDATA& document);

    virtual QString getSectionName();
    virtual int getSortOrder() const { return 11; }

    void addPlaceHoldItem();
    bool isContainsPlaceHoldItem();
    void removePlaceHoldItem();
};

class WizCategoryViewShortcutPlaceHoldItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewShortcutPlaceHoldItem(WizExplorerApp& app, const QString& strName);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos) {}
    virtual void getDocuments(WizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(arrayDocument); }

    virtual int getItemHeight(int hintHeight) const;

    virtual void drawItemBody(QPainter* p, const QStyleOptionViewItem* vopt) const;
};

class WizCategoryViewShortcutItem : public WizCategoryViewItemBase
{
public:
    enum ShortcutType
    {
        Document,
        PersonalFolder,
        PersonalTag,
        GroupTag
    };
    //
    WizCategoryViewShortcutItem(WizExplorerApp& app, const QString& strName, ShortcutType type,
                                 const QString& strKbGuid, const QString& strGuid, const QString& location, bool bEncrypted = false);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(WizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(arrayDocument); }

    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);

    QString guid() const {return m_strGuid;}
    QString location() const { return m_location; }
    ShortcutType shortcutType() const { return m_type; }

private:
    QString m_strGuid;
    QString m_location;
    ShortcutType m_type;
};

#endif // GUI_CATEGORYVIEWER_WIZCATEGORYVIEWSHORTCUTITEM_H