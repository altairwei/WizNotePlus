#ifndef GUI_CATEGORYVIEWER_WIZCATEGORYVIEWGROUPITEM_H
#define GUI_CATEGORYVIEWER_WIZCATEGORYVIEWGROUPITEM_H

#include "gui/categoryviewer/WizCategoryViewItemBase.h"


class WizCategoryViewGroupItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewGroupItem(WizExplorerApp& app, const WIZTAGDATA& tag, const QString& strKbGUID);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const;
    virtual bool acceptDrop(const WIZDOCUMENTDATA& data) const;
    virtual bool acceptDrop(const QString& urls) const;
    virtual bool dragAble() const { return true; }
    virtual void drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy = false);

    virtual QString id() const;

    virtual bool operator<(const QTreeWidgetItem &other) const;

    void reload(WizDatabase& db);
    void setTagPosition(int nPos);
    const WIZTAGDATA& tag() const { return m_tag; }

    virtual int getSortOrder() const { return 11; }

private:
    WIZTAGDATA m_tag;
};


class WizCategoryViewGroupsRootItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewGroupsRootItem(WizExplorerApp& app, const QString& strName);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);

    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
//    virtual bool acceptDrop(const CWizCategoryViewItemBase* pItem) const;
    virtual QString getSectionName();

};

class WizCategoryViewBizGroupRootItem : public WizCategoryViewGroupsRootItem
{
public:
    WizCategoryViewBizGroupRootItem(WizExplorerApp& app,
                                     const WIZBIZDATA& biz);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    //
    const WIZBIZDATA biz() const { return m_biz; }
    virtual int getSortOrder() const { return 30; }    
    //
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual void drawExtraBadge(QPainter* p, const QStyleOptionViewItem* vopt) const;

    //
    virtual bool acceptMousePressedInfo() { return true; }
    virtual void mousePressed(const QPoint& pos);
    virtual void mouseReleased(const QPoint& pos);

    //
    bool isExtraButtonUseable() const;
    bool isUnreadButtonUseable() const;
    void updateUnreadCount();
    QString unreadString() const;
    bool hitTestUnread();
    virtual QString getExtraButtonToolTip() const;
    virtual QRect getExtraButtonRect(const QRect &itemBorder, bool ignoreIconExist = false) const;
    //
    bool isOwner();
    bool isAdmin();
    bool isHr();


private:
    WIZBIZDATA m_biz;
    int m_unReadCount;
    QSize m_szUnreadSize;
};

class WizCategoryViewOwnGroupRootItem : public WizCategoryViewGroupsRootItem
{
public:
    WizCategoryViewOwnGroupRootItem(WizExplorerApp& app);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);

    virtual int getSortOrder() const { return 31; }
};

class WizCategoryViewJionedGroupRootItem : public WizCategoryViewGroupsRootItem
{
public:
    WizCategoryViewJionedGroupRootItem(WizExplorerApp& app);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual int getSortOrder() const { return 32; }
};

class WizCategoryViewGroupRootItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewGroupRootItem(WizExplorerApp& app,
                                  const WIZGROUPDATA& group);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual bool acceptDrop(const WIZDOCUMENTDATA& data) const;
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const;
    virtual bool acceptDrop(const QString& urls) const;
    virtual void drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy = false);
    virtual void drawExtraBadge(QPainter* p, const QStyleOptionViewItem* vopt) const;
    void reload(WizDatabase& db);
    //
    virtual bool acceptMousePressedInfo() { return true; }
    virtual void mousePressed(const QPoint& pos);
    virtual void mouseReleased(const QPoint& pos);    
    //
    virtual bool operator<(const QTreeWidgetItem &other) const;
    //
    bool isAdmin(WizDatabase& db);
    bool isOwner(WizDatabase& db);

    bool isBizGroup() const;
    QString bizGUID() const;
    //
    void setUnreadCount(int nCount);
    int getUnreadCount();
    QString unreadString() const;
    bool hitTestUnread();
    virtual QString getExtraButtonToolTip() const;
    virtual QRect getExtraButtonRect(const QRect &itemBorder, bool ignoreIconExist = false) const;

private:
    WIZGROUPDATA m_group;
    int m_nUnread;
    QSize m_szUnreadSize;
};

class WizCategoryViewGroupNoTagItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewGroupNoTagItem(WizExplorerApp& app, const QString& strKbGUID);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos) {Q_UNUSED(pCtrl); Q_UNUSED(pos);}
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual int getSortOrder() const { return 10; }
};

#endif // GUI_CATEGORYVIEWER_WIZCATEGORYVIEWGROUPITEM_H