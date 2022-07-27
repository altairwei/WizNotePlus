#include "WizCategoryViewGroupItem.h"

#include "WizDef.h"
#include "utils/WizPinyin.h"
#include "utils/WizStyleHelper.h"
#include "share/WizObjectOperator.h"
#include "database/WizDatabaseManager.h"
#include "share/WizSettings.h"
#include "share/WizMisc.h"

#include "gui/categoryviewer/WizCategoryView.h"


/* ------------------------------ CWizCategoryViewGroupItem ------------------------------ */

WizCategoryViewGroupItem::WizCategoryViewGroupItem(WizExplorerApp& app,
                                                     const WIZTAGDATA& tag,
                                                     const QString& strKbGUID)
    : WizCategoryViewItemBase(app, tag.strName, strKbGUID, Category_GroupItem)
    , m_tag(tag)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_folder", QSize());
    setIcon(0, icon);
    setText(0, WizDatabase::tagNameToDisplayName(tag.strName));
}

void WizCategoryViewGroupItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showGroupContextMenu(pos);
    }
}

void WizCategoryViewGroupItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.getDocumentsByTag(m_tag, arrayDocument);
}

bool WizCategoryViewGroupItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (db.isInDeletedItems(data.strLocation))
        return false;

    QString strTagGUIDs = db.getDocumentTagGuidsString(data.strGUID);
    if (strTagGUIDs.indexOf(m_tag.strGUID) != -1 && data.strKbGUID == kbGUID())
        return true;

    return false;
}

bool WizCategoryViewGroupItem::acceptDrop(const WizCategoryViewItemBase* pItem) const
{
//    return pItem->type() == Category_FolderItem | pItem->type() == Category_GroupItem;

    return pItem->kbGUID() == kbGUID();
}

bool WizCategoryViewGroupItem::acceptDrop(const WIZDOCUMENTDATA& data) const
{
    Q_UNUSED(data);

    WizDatabase& db = WizDatabaseManager::instance()->db(kbGUID());
    if (WIZ_USERGROUP_AUTHOR >= db.permission()) {
        return true;
    }

    return false;
}

bool WizCategoryViewGroupItem::acceptDrop(const QString& urls) const
{
    Q_UNUSED(urls);
    WizDatabase& db = m_app.databaseManager().db(kbGUID());

    return WIZ_USERGROUP_AUTHOR >= db.permission();
}

void WizCategoryViewGroupItem::drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy)
{
    CWizDocumentDataArray arrayOp;
    bool needCopy = false;
    for (WIZDOCUMENTDATA doc : arrayDocument)
    {
        if (!acceptDrop(doc))
            continue;

        if (forceCopy || doc.strKbGUID != m_strKbGUID)
            needCopy = true;

        arrayOp.push_back(doc);
    }

    if (arrayOp.empty())
        return;


    WizDocumentOperator documentOperator(m_app.databaseManager());

    if (needCopy)
    {
        documentOperator.copyDocumentsToGroupFolder(arrayOp, m_tag, false, true);
    }
    else
    {
        documentOperator.moveDocumentsToGroupFolder(arrayOp, m_tag, true);
    }
}

QString WizCategoryViewGroupItem::id() const
{
    return ::WizMd5StringNoSpaceJava(QString(text(0) + m_tag.strGUID).toUtf8());
}

bool WizCategoryViewGroupItem::operator<(const QTreeWidgetItem& other) const
{
    if (m_app.userSettings().isManualSortingEnabled())
    {
        const WizCategoryViewGroupItem* pOther = dynamic_cast<const WizCategoryViewGroupItem*>(&other);
        if (pOther)
        {
            if (m_tag.nPosition == pOther->m_tag.nPosition || m_tag.nPosition == 0 || pOther->m_tag.nPosition == 0)
                return WizCategoryViewItemBase::operator <(other);

            return m_tag.nPosition < pOther->m_tag.nPosition;
        }
    }

    return WizCategoryViewItemBase::operator <(other);
}

void WizCategoryViewGroupItem::reload(WizDatabase& db)
{
    db.tagFromGuid(m_tag.strGUID, m_tag);
    setText(0, m_tag.strName);
    m_strName = m_tag.strName;
}

void WizCategoryViewGroupItem::setTagPosition(int nPos)
{
    m_tag.nPosition = nPos;
}


/* ---------------------------- CWizCategoryViewGroupsRootItem ---------------------------- */

WizCategoryViewGroupsRootItem::WizCategoryViewGroupsRootItem(WizExplorerApp& app, const QString& strName)
    : WizCategoryViewItemBase(app, strName, "", Category_GroupsRootItem)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_group");
    setIcon(0, icon);
    setText(0, strName);
}

void WizCategoryViewGroupsRootItem::showContextMenu(WizCategoryBaseView *pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showNormalGroupRootContextMenu(pos);
    }
}

void WizCategoryViewGroupsRootItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    Q_UNUSED(db);

    for (int i = 0; i < childCount(); i++) {
        WizCategoryViewGroupRootItem* pGroup = dynamic_cast<WizCategoryViewGroupRootItem*>(child(i));
        Q_ASSERT(pGroup);
        if (!pGroup)
            return;

        WizDatabase& db = WizDatabaseManager::instance()->db(pGroup->kbGUID());

        CWizDocumentDataArray arrayDoc;
        if (db.getDocumentsByTime(QDateTime::currentDateTime().addDays(-3), arrayDocument)) {
            arrayDocument.insert(arrayDocument.begin(), arrayDoc.begin(), arrayDoc.end());
        }
    }
}

bool WizCategoryViewGroupsRootItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    Q_UNUSED(db);

    QDateTime t(QDateTime::currentDateTime().addDays(-3));
    for (int i = 0; i < childCount(); i++) {
        WizCategoryViewGroupRootItem* pGroup = dynamic_cast<WizCategoryViewGroupRootItem*>(child(i));
        Q_ASSERT(pGroup);
        if (!pGroup)
            continue;

        if (pGroup->kbGUID() == data.strKbGUID) {
            if (data.tDataModified > t)
                return true;
        }
    }

    return false;
}

//bool CWizCategoryViewGroupsRootItem::acceptDrop(const CWizCategoryViewItemBase* pItem) const
//{
//    const CWizCategoryViewGroupItem* item = dynamic_cast<const CWizCategoryViewGroupItem*>(pItem);
//    return NULL != item;
//}
QString WizCategoryViewGroupsRootItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GROUPS;
}


/* ------------------------------ CWizCategoryViewGroupRootItem ------------------------------ */
WizCategoryViewBizGroupRootItem::WizCategoryViewBizGroupRootItem(WizExplorerApp& app,
                                                                   const WIZBIZDATA& biz)
    : WizCategoryViewGroupsRootItem(app, biz.bizName)
    , m_biz(biz)
    , m_unReadCount(0)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_biz");
    setIcon(0, icon);
}

void WizCategoryViewBizGroupRootItem::showContextMenu(WizCategoryBaseView *pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        if(isHr())
        {
            view->showAdminBizGroupRootContextMenu(pos);
        }
        else
        {
//            view->showNormalBizGroupRootContextMenu(pos);
            view->showAdminBizGroupRootContextMenu(pos, false);
        }
    }
}

void WizCategoryViewBizGroupRootItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    if (isUnreadButtonUseable() && hitTestUnread())
    {
        for (int i = 0; i < childCount(); i++)
        {
            WizCategoryViewGroupRootItem* pGroup = dynamic_cast<WizCategoryViewGroupRootItem*>(child(i));
            Q_ASSERT(pGroup);
            if (!pGroup)
                return;

            WizDatabase& db = WizDatabaseManager::instance()->db(pGroup->kbGUID());

            CWizDocumentDataArray arrayDoc;
            if (db.getGroupUnreadDocuments(arrayDoc))
            {
                arrayDocument.insert(arrayDocument.begin(), arrayDoc.begin(), arrayDoc.end());
            }
        }
        updateUnreadCount();
    }
    else
    {
        WizCategoryViewGroupsRootItem::getDocuments(db, arrayDocument);
    }
}

void WizCategoryViewBizGroupRootItem::drawExtraBadge(QPainter* p, const QStyleOptionViewItem* vopt) const
{
    if (isUnreadButtonUseable())
    {
        //
        QString text = unreadString();
        if (text.isEmpty())
            return;

        p->save();
        //
        QRect rcb = getExtraButtonRect(vopt->rect, true);
        p->setRenderHint(QPainter::Antialiasing);
        drawClickableUnreadButton(p, rcb, text, m_extraButtonIconPressed);
        //
        p->restore();
    }
    else
    {
        WizCategoryViewGroupsRootItem::drawExtraBadge(p, vopt);
    }
}

void WizCategoryViewBizGroupRootItem::mousePressed(const QPoint& pos)
{
    QRect rcBorder = treeWidget()->visualItemRect(this);
    QRect rcIcon = getExtraButtonRect(rcBorder, true);
    if (rcIcon.contains(pos))
    {
        m_extraButtonIconPressed = true;
    }
}

void WizCategoryViewBizGroupRootItem::mouseReleased(const QPoint& pos)
{
    m_extraButtonIconPressed = false;
}

QString WizCategoryViewBizGroupRootItem::getExtraButtonToolTip() const
{
    if (m_unReadCount > 0 && isUnreadButtonUseable())
        return QObject::tr("You have %1 unread notes").arg(m_unReadCount);

    if (m_extraButtonIcon.isNull() || !isExtraButtonUseable())
        return "";

    return QObject::tr("Your enterprise services has expired");
}

QRect WizCategoryViewBizGroupRootItem::getExtraButtonRect(const QRect& itemBorder, bool ignoreIconExist) const
{
    if (!isUnreadButtonUseable())
        return WizCategoryViewItemBase::getExtraButtonRect(itemBorder, ignoreIconExist);

    if (!m_unReadCount && !ignoreIconExist)
        return QRect();

    int nButtonWidth = 26;
    int nButtonHeight = 14;
    QRect rc(itemBorder.right() - EXTRABUTTONRIGHTMARGIN - nButtonWidth, itemBorder.y() + (itemBorder.height() - nButtonHeight) / 2,
             nButtonWidth, nButtonHeight);
    return rc;
}

bool WizCategoryViewBizGroupRootItem::isExtraButtonUseable() const
{
    return !isUnreadButtonUseable();
}

bool WizCategoryViewBizGroupRootItem::isUnreadButtonUseable() const
{
     bool bUseable = (!isExpanded()) && (m_unReadCount > 0);
     return bUseable;
}

void WizCategoryViewBizGroupRootItem::updateUnreadCount()
{
    m_unReadCount = 0;
    int nChildCount = childCount();
    for (int i = 0; i < nChildCount; i++)
    {
        WizCategoryViewGroupRootItem* childItem = dynamic_cast<WizCategoryViewGroupRootItem*>(child(i));
        if (childItem)
        {
            m_unReadCount += childItem->getUnreadCount();
        }
    }

    WizCategoryBaseView* view = dynamic_cast<WizCategoryBaseView*>(treeWidget());
    if (m_unReadCount > 0)
    {
        //
        QFont f;
        Utils::WizStyleHelper::fontNormal(f);
        QFontMetrics fm(f);
        //
        QSize szText = fm.size(0, unreadString());
        int textWidth = szText.width();
//        int textHeight = szText.height();
        //
        //int nMargin = textHeight / 4;
        //
        int nWidth = textWidth  + nNumberButtonHorizontalMargin * 2;
        int nHeight = nNumberButtonHeight;// textHeight + 2;
        if (nWidth < nHeight)
            nWidth = nHeight;
        //
        Q_ASSERT(view);

        // use parent height, group root could be unvisible
        QRect rcIemBorder = view->visualItemRect(this);
        QRect rcExtButton = getExtraButtonRect(rcIemBorder, true);
        //
        int nTop = rcIemBorder.y() + (rcIemBorder.height() - nHeight) / 2;
        int nLeft = rcExtButton.right() - nWidth;
        QRect rcb(nLeft, nTop, nWidth, nHeight);

        m_szUnreadSize = rcb.size();
    }

    view->updateItem(this);
}

QString WizCategoryViewBizGroupRootItem::unreadString() const
{
    return unreadNumToString(m_unReadCount);
}

bool WizCategoryViewBizGroupRootItem::hitTestUnread()
{
    WizCategoryBaseView* view = dynamic_cast<WizCategoryBaseView*>(treeWidget());
    Q_ASSERT(view);

    QRect rcItem = view->visualItemRect(this);
    QPoint pt = view->hitPoint();
    //
    int nMargin = 4;
    QRect rcRect = getExtraButtonRect(rcItem, true);
    QRect rcb = QRect(rcRect.right() - m_szUnreadSize.width() + 1, rcRect.y() + (rcRect.height() - m_szUnreadSize.height())/2,
                      m_szUnreadSize.width(), m_szUnreadSize.height());
    rcb.adjust(-nMargin, -nMargin, nMargin, nMargin);

    return rcb.contains(pt);
}

bool WizCategoryViewBizGroupRootItem::isOwner()
{
    return m_biz.bizUserRole == WIZ_BIZROLE_OWNER;
}
bool WizCategoryViewBizGroupRootItem::isAdmin()
{
    return m_biz.bizUserRole == WIZ_BIZROLE_ADMIN;
}

bool WizCategoryViewBizGroupRootItem::isHr()
{
    return m_biz.bizUserRole <= WIZ_BIZROLE_HR;
}

WizCategoryViewOwnGroupRootItem::WizCategoryViewOwnGroupRootItem(WizExplorerApp& app)
    : WizCategoryViewGroupsRootItem(app, CATEGORY_OWN_GROUPS)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_group");
    setIcon(0, icon);
}

void WizCategoryViewOwnGroupRootItem::showContextMenu(WizCategoryBaseView *pCtrl, QPoint pos)
{
    Q_UNUSED(pCtrl)
    Q_UNUSED(pos)
//    if (CWizCategoryView* view = dynamic_cast<CWizCategoryView *>(pCtrl)) {
//        view->showOwnGroupRootContextMenu(pos);
//    }
}


WizCategoryViewJionedGroupRootItem::WizCategoryViewJionedGroupRootItem(WizExplorerApp& app)
    : WizCategoryViewGroupsRootItem(app, CATEGORY_OTHER_GROUPS)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_group");
    setIcon(0, icon);
}


/* ------------------------------ CWizCategoryViewGroupRootItem ------------------------------ */

WizCategoryViewGroupRootItem::WizCategoryViewGroupRootItem(WizExplorerApp& app,
                                                             const WIZGROUPDATA& group)
    : WizCategoryViewItemBase(app, group.strGroupName, group.strGroupGUID, Category_GroupRootItem)
    , m_group(group)
    , m_nUnread(0)
{
    QIcon icon;
    if (group.bEncryptData)
    {
        icon = WizLoadSkinIcon(app.userSettings().skin(), "category_group_enc");
    } else {
        icon = WizLoadSkinIcon(app.userSettings().skin(), "category_group");
    }
    setIcon(0, icon);
    setText(0, m_strName);
}

void WizCategoryViewGroupRootItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        if(isOwner(m_app.databaseManager().db(m_strKbGUID)))
        {
            view->showOwnerGroupRootContextMenu(pos);
        }
        else if(isAdmin(m_app.databaseManager().db(m_strKbGUID)))
        {
            view->showAdminGroupRootContextMenu(pos);
        }
        else
        {
            view->showNormalGroupRootContextMenu(pos);
        }
    }
}

void WizCategoryViewGroupRootItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    if (hitTestUnread() && m_nUnread)
    {
        db.getGroupUnreadDocuments(arrayDocument);
    }
    else
    {
        db.getLastestDocuments(arrayDocument, 1000);
    }
}

bool WizCategoryViewGroupRootItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (db.isInDeletedItems(data.strLocation))
        return false;

    if (data.strKbGUID == kbGUID())
        return true;

    return false;
}

bool WizCategoryViewGroupRootItem::acceptDrop(const WIZDOCUMENTDATA &data) const
{
    Q_UNUSED(data);

    WizDatabase& db = WizDatabaseManager::instance()->db(kbGUID());
    if (WIZ_USERGROUP_AUTHOR >= db.permission())
        return true;

    return false;
}

bool WizCategoryViewGroupRootItem::acceptDrop(const WizCategoryViewItemBase* pItem) const
{
//    return pItem->type() == Category_FolderItem | pItem->type() == Category_GroupItem;

    const WizCategoryViewGroupItem* item = dynamic_cast<const WizCategoryViewGroupItem*>(pItem);
    return item && item->kbGUID() == kbGUID();
}

bool WizCategoryViewGroupRootItem::acceptDrop(const QString& urls) const
{
    Q_UNUSED(urls);
    WizDatabase& db = m_app.databaseManager().db(kbGUID());

    return WIZ_USERGROUP_AUTHOR >= db.permission();
}

void WizCategoryViewGroupRootItem::drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy)
{
    CWizDocumentDataArray arrayOp;
    bool needCopy = false;
    for (WIZDOCUMENTDATA doc : arrayDocument)
    {
        if (!acceptDrop(doc))
            continue;

        if (forceCopy || doc.strKbGUID != m_strKbGUID)
            needCopy = true;

        arrayOp.push_back(doc);
    }

    if (arrayOp.empty())
        return;

    WizDocumentOperator documentOperator(m_app.databaseManager());

    if (needCopy)
    {
        WIZTAGDATA tag;
        tag.strKbGUID = m_strKbGUID;
        documentOperator.copyDocumentsToGroupFolder(arrayOp, tag, true);
    }
    else
    {
        WIZTAGDATA tag;
        tag.strKbGUID = m_strKbGUID;
        documentOperator.moveDocumentsToGroupFolder(arrayOp, tag, true);
    }
}

void WizCategoryViewGroupRootItem::drawExtraBadge(QPainter* p, const QStyleOptionViewItem* vopt) const
{
    //
    if (m_nUnread > 0)
    {
        QString text = unreadString();
        p->save();
        //
        QRect rcb = getExtraButtonRect(vopt->rect, true);
        p->setRenderHint(QPainter::Antialiasing);
        drawClickableUnreadButton(p, rcb, text, m_extraButtonIconPressed);
        //
        p->restore();
    }
    else
    {
        WizCategoryViewItemBase::drawExtraBadge(p, vopt);
    }
}

void WizCategoryViewGroupRootItem::reload(WizDatabase& db)
{
    m_strName = db.name();
    setText(0, db.name());
}

void WizCategoryViewGroupRootItem::mousePressed(const QPoint& pos)
{
    QRect rcBorder = treeWidget()->visualItemRect(this);
    QRect rcIcon = getExtraButtonRect(rcBorder, true);
    if (rcIcon.contains(pos))
    {
        m_extraButtonIconPressed = true;
    }
}

void WizCategoryViewGroupRootItem::mouseReleased(const QPoint& pos)
{
    m_extraButtonIconPressed = false;
}

bool WizCategoryViewGroupRootItem::operator<(const QTreeWidgetItem& other) const
{
    if (other.type() != Category_GroupRootItem)
        return QTreeWidgetItem::operator <(other);

    const WizCategoryViewGroupRootItem* pItem = dynamic_cast<const WizCategoryViewGroupRootItem*>(&other);
    //
    return WizToolsSmartCompare(m_group.strGroupName, pItem->m_group.strGroupName) < 0;
}

bool WizCategoryViewGroupRootItem::isAdmin(WizDatabase& db)
{
    if (isBizGroup())
    {
        if (WizCategoryViewBizGroupRootItem* pBiz = dynamic_cast<WizCategoryViewBizGroupRootItem *>(parent()))
        {
            if (pBiz->isAdmin())
                return true;
        }
    }
    //
    return db.isGroupAdmin();
}

bool WizCategoryViewGroupRootItem::isOwner(WizDatabase& db)
{
    return db.isGroupOwner();
}

bool WizCategoryViewGroupRootItem::isBizGroup() const
{
    return m_group.isBiz();
}

QString WizCategoryViewGroupRootItem::bizGUID() const
{
    return m_group.bizGUID;
}

void WizCategoryViewGroupRootItem::setUnreadCount(int nCount)
{
    m_nUnread = nCount;
    WizCategoryBaseView* view = dynamic_cast<WizCategoryBaseView*>(treeWidget());
    //
    if (m_nUnread > 0)
    {
        QFont f;
        Utils::WizStyleHelper::fontNormal(f);
        QFontMetrics fm(f);
        //
        QSize szText = fm.size(0, unreadString());
        int textWidth = szText.width();
//        int textHeight = szText.height();
        //
        //int nMargin = textHeight / 4;
        //
        int nWidth = textWidth + nNumberButtonHorizontalMargin * 2;
        int nHeight = nNumberButtonHeight;//  textHeight + 2;
        if (nWidth < nHeight)
            nWidth = nHeight;
        //

        Q_ASSERT(view);

        // use parent height, group root could be unvisible
        QRect rcIemBorder = view->visualItemRect(this->parent());
        QRect rcExtButton = getExtraButtonRect(rcIemBorder, true);
        //
        int nTop = rcIemBorder.y() + (rcIemBorder.height() - nHeight) / 2;
        int nLeft = rcExtButton.right() - nWidth;
        QRect rcb(nLeft, nTop, nWidth, nHeight);

        m_szUnreadSize = rcb.size();
    }

    view->updateItem(this);
}

int WizCategoryViewGroupRootItem::getUnreadCount()
{
    return m_nUnread;
}

QString WizCategoryViewGroupRootItem::unreadString() const
{
    return unreadNumToString(m_nUnread);
}

bool WizCategoryViewGroupRootItem::hitTestUnread()
{
    WizCategoryBaseView* view = dynamic_cast<WizCategoryBaseView*>(treeWidget());
    Q_ASSERT(view);

    QRect rcItem = view->visualItemRect(this);
    QPoint pt = view->hitPoint();
    //
    int nMargin = 4;
    QRect rcRect = getExtraButtonRect(rcItem, true);
    QRect rcb = QRect(rcRect.right() - m_szUnreadSize.width() + 1, rcRect.y() + (rcRect.height() - m_szUnreadSize.height())/2,
                      m_szUnreadSize.width(), m_szUnreadSize.height());
    rcb.adjust(-nMargin, -nMargin, nMargin, nMargin);

    return rcb.contains(pt);
}

QString WizCategoryViewGroupRootItem::getExtraButtonToolTip() const
{
    if (m_nUnread > 0)
        return QObject::tr("You have %1 unread notes").arg(m_nUnread);

    if (m_extraButtonIcon.isNull())
        return "";

    return QObject::tr("Your group is in the abnormal state");
}

QRect WizCategoryViewGroupRootItem::getExtraButtonRect(const QRect& itemBorder, bool ignoreIconExist) const
{
    if (m_nUnread == 0)
        return WizCategoryViewItemBase::getExtraButtonRect(itemBorder, ignoreIconExist);

    if (!m_nUnread && !ignoreIconExist)
        return QRect();

    int nButtonWidth = 26;
    int nButtonHeight = 14;
    QRect rc(itemBorder.right() - EXTRABUTTONRIGHTMARGIN - nButtonWidth, itemBorder.y() + (itemBorder.height() - nButtonHeight) / 2,
             nButtonWidth, nButtonHeight);
    return rc;
}

/* --------------------- CWizCategoryViewGroupNoTagItem --------------------- */
WizCategoryViewGroupNoTagItem::WizCategoryViewGroupNoTagItem(WizExplorerApp& app,
                                                               const QString& strKbGUID)
    : WizCategoryViewItemBase(app, PREDEFINED_UNCLASSIFIED, strKbGUID, Category_GroupNoTagItem)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_unclassified");
    setIcon(0, icon);
    setText(0, PREDEFINED_UNCLASSIFIED);
}

void WizCategoryViewGroupNoTagItem::getDocuments(WizDatabase& db,
                                                  CWizDocumentDataArray& arrayDocument)
{
    db.getDocumentsNoTag(arrayDocument);
}

bool WizCategoryViewGroupNoTagItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (kbGUID() != data.strKbGUID)
        return false;

    if (db.isInDeletedItems(data.strLocation))
        return false;

    QString strTagGUIDs = db.getDocumentTagGuidsString(data.strGUID);
    if (strTagGUIDs.isEmpty())
        return true;

    return false;
}