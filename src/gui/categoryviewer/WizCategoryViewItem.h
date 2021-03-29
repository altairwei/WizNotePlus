#ifndef WIZCATEGORYVIEWITEM_H
#define WIZCATEGORYVIEWITEM_H

#include <QTreeWidgetItem>

#include "share/WizObject.h"

#include "gui/categoryviewer/WizCategoryViewItemBase.h"
#include "gui/categoryviewer/WizCategoryViewSectionItem.h"
#include "gui/categoryviewer/WizCategoryViewMessageItem.h"
#include "gui/categoryviewer/WizCategoryViewShortcutItem.h"
#include "gui/categoryviewer/WizCategoryViewSearchItem.h"
#include "gui/categoryviewer/WizCategoryViewFolderItem.h"
#include "gui/categoryviewer/WizCategoryViewTagItem.h"
#include "gui/categoryviewer/WizCategoryViewGroupItem.h"


class WizCategoryViewStyleRootItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewStyleRootItem(WizExplorerApp& app, const QString& strName);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual void getDocuments(WizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(arrayDocument); }
    virtual QString getSectionName();
    virtual int getSortOrder() const { return 22; }
};


class WizCategoryViewLinkItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewLinkItem(WizExplorerApp& app, const QString& strName, int commandId)
        : WizCategoryViewItemBase(app, strName)
        , m_commandId(commandId) { setFlags(Qt::NoItemFlags); setText(0, strName); }

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }
    virtual void getDocuments(WizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(arrayDocument); }
    int commandId() const { return m_commandId; }

    virtual void drawItemBody(QPainter* p, const QStyleOptionViewItem* vopt) const;

protected:
    int m_commandId;
};

class WizCategoryViewCreateGroupLinkItem : public WizCategoryViewLinkItem
{
public:
    WizCategoryViewCreateGroupLinkItem(WizExplorerApp& app, const QString& strName, int commandId)
        : WizCategoryViewLinkItem(app, strName, commandId) {}
    //
    virtual QString getSectionName();
    virtual int getSortOrder() const { return 29; }


};


class WizCategoryViewTrashItem : public WizCategoryViewFolderItem
{
public:
    WizCategoryViewTrashItem(WizExplorerApp& app, const QString& strKbGUID);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual bool acceptDrop(const WIZDOCUMENTDATA& data) const;
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const;
    virtual bool dragAble() const { return false; }
    virtual int getSortOrder() const { return 12; }
    //
    virtual void drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy = false);
};

#endif // WIZCATEGORYVIEWITEM_H
