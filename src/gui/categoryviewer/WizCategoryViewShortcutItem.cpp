#include "WizCategoryViewShortcutItem.h"

#include <QTimer>

#include "WizDef.h"
#include "utils/WizStyleHelper.h"
#include "share/WizSettings.h"
#include "share/WizDatabase.h"
#include "share/WizDatabaseManager.h"
#include "share/WizMisc.h"
#include "gui/categoryviewer/WizCategoryView.h"


/* -------------------- CWizCategoryViewShortcutRootItem -------------------- */
WizCategoryViewShortcutRootItem::WizCategoryViewShortcutRootItem(WizExplorerApp& app,
                                                                   const QString& strName)
    : WizCategoryViewItemBase(app, strName, "", Category_ShortcutRootItem)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_shortcut");
    setIcon(0, icon);
    setText(0, strName);
}

void WizCategoryViewShortcutRootItem::getDocuments(WizDatabase& /*db*/, CWizDocumentDataArray& arrayDocument)
{
    for (int i = 0; i < childCount(); i++)
    {
        WizCategoryViewShortcutItem *pItem = dynamic_cast<WizCategoryViewShortcutItem*>(child(i));
        if (pItem && !pItem->guid().isEmpty())
        {
            WizDatabase &db = m_app.databaseManager().db(pItem->kbGUID());
            WIZDOCUMENTDATA doc;
            if (db.documentFromGuid(pItem->guid(), doc))
            {
                arrayDocument.push_back(doc);
            }
        }
    }
}

bool WizCategoryViewShortcutRootItem::accept(WizDatabase& /*db*/, const WIZDOCUMENTDATA& data)
{
    for (int i = 0; i < childCount(); i++)
    {
        WizCategoryViewShortcutItem *pItem = dynamic_cast<WizCategoryViewShortcutItem*>(child(i));
        if (pItem)
        {
            if (pItem->guid() == data.strGUID)
                return true;
        }
    }
    return false;
}

void WizCategoryViewShortcutRootItem::drop(const CWizDocumentDataArray& arrayDocument, bool /*forceCopy*/)
{
    bool changed = false;
    for (WIZDOCUMENTDATA document : arrayDocument)
    {        
        WizCategoryViewShortcutItem *pItem = addDocumentToShortcuts(document);
        if (pItem)
        {
            changed = true;
        }
    }

    if (changed)
    {
        QTimer::singleShot(200, [this]() {
            WizCategoryView* categoryView = dynamic_cast<WizCategoryView*>(treeWidget());
            Q_ASSERT(categoryView);
            categoryView->saveShortcutState();
        });
    }
}

void WizCategoryViewShortcutRootItem::drop(const WizCategoryViewItemBase* pItem)
{
    WizCategoryViewShortcutItem* newItem = addItemToShortcuts(pItem);
    if (!newItem)
        return;
    //
    treeWidget()->blockSignals(true);
    treeWidget()->setCurrentItem(newItem);
    treeWidget()->blockSignals(false);
    sortChildren(0, Qt::AscendingOrder);

    WizCategoryView* categoryView = dynamic_cast<WizCategoryView*>(treeWidget());
    QTimer::singleShot(200, categoryView, SLOT(saveShortcutState()));
}

bool WizCategoryViewShortcutRootItem::acceptDrop(const WizCategoryViewItemBase* pItem) const
{
    if (!pItem)
        return false;

    if (pItem->type() == Category_FolderItem || pItem->type() == Category_TagItem || pItem->type() == Category_GroupItem)
        return true;

    return false;
}

WizCategoryViewShortcutItem* WizCategoryViewShortcutRootItem::addItemToShortcuts(const WizCategoryViewItemBase* pItem)
{
    WizCategoryViewShortcutItem* newItem = nullptr;
    if (pItem->type() == Category_FolderItem)
    {
        const WizCategoryViewFolderItem* folderItem = dynamic_cast<const WizCategoryViewFolderItem*>(pItem);
        newItem = new WizCategoryViewShortcutItem(m_app, WizDatabase::getLocationName(folderItem->location()),
                                                                                 WizCategoryViewShortcutItem::PersonalFolder, "", "", folderItem->location());
    }
    else if (pItem->type() == Category_TagItem)
    {
        const WizCategoryViewTagItem* tagItem = dynamic_cast<const WizCategoryViewTagItem*>(pItem);
        newItem = new WizCategoryViewShortcutItem(m_app, tagItem->tag().strName, WizCategoryViewShortcutItem::PersonalTag,
                                                    tagItem->tag().strKbGUID, tagItem->tag().strGUID, "");
    }
    else if (pItem->type() == Category_GroupItem)
    {
        const WizCategoryViewGroupItem* groupItem = dynamic_cast<const WizCategoryViewGroupItem*>(pItem);
        newItem = new WizCategoryViewShortcutItem(m_app, groupItem->tag().strName, WizCategoryViewShortcutItem::GroupTag,
                                                    groupItem->tag().strKbGUID, groupItem->tag().strGUID, "");
    }
    else
    {
        return nullptr;
    }
    //
    for (int i = 0; i < childCount(); i++)
    {
        WizCategoryViewShortcutItem *shortcutItem = dynamic_cast<WizCategoryViewShortcutItem*>(child(i));
        if (shortcutItem)
        {
            switch (shortcutItem->shortcutType()) {
            case WizCategoryViewShortcutItem::PersonalTag:
            case WizCategoryViewShortcutItem::GroupTag:
            {
                if (shortcutItem->guid() == newItem->guid())
                {
                    delete newItem;
                    return nullptr;
                }
            }
                break;
            case WizCategoryViewShortcutItem::PersonalFolder:
            {
                if (shortcutItem->location() == newItem->location())
                {
                    delete newItem;
                    return nullptr;
                }
            }
                break;
            default:
                continue;
            }
        }
    }

    //
    addChild(newItem);
    sortChildren(0, Qt::AscendingOrder);
    if (isContainsPlaceHoldItem())
        removePlaceHoldItem();

    return newItem;
}

WizCategoryViewShortcutItem*WizCategoryViewShortcutRootItem::addDocumentToShortcuts(const WIZDOCUMENTDATA& document)
{
    for (int i = 0; i < childCount(); i++)
    {
        WizCategoryViewShortcutItem *pItem = dynamic_cast<WizCategoryViewShortcutItem*>(child(i));
        if (pItem)
        {
            if (pItem->guid() == document.strGUID)
                return nullptr;
        }
    }

    if (isContainsPlaceHoldItem())
        removePlaceHoldItem();

    bool isEncrypted = document.nProtected == 1;
    WizCategoryViewShortcutItem *pItem = new WizCategoryViewShortcutItem(m_app,
                                                                           document.strTitle, WizCategoryViewShortcutItem::Document,
                                                                           document.strKbGUID, document.strGUID, document.strLocation, isEncrypted);

    addChild(pItem);
    sortChildren(0, Qt::AscendingOrder);
    return pItem;
}

QString WizCategoryViewShortcutRootItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GENERAL;
}

void WizCategoryViewShortcutRootItem::addPlaceHoldItem()
{
    WizCategoryViewShortcutPlaceHoldItem *item = new
            WizCategoryViewShortcutPlaceHoldItem(m_app, WIZ_CATEGORY_SHOTCUT_PLACEHOLD);
    item->setText(0, WIZ_CATEGORY_SHOTCUT_PLACEHOLD);
    addChild(item);
}

bool WizCategoryViewShortcutRootItem::isContainsPlaceHoldItem()
{
    if (childCount() < 1)
        return false;

    QTreeWidgetItem *item = child(0);
    return item->text(0) == WIZ_CATEGORY_SHOTCUT_PLACEHOLD;
}

void WizCategoryViewShortcutRootItem::removePlaceHoldItem()
{
    if (isContainsPlaceHoldItem())
    {
        removeChild(child(0));
    }
}



WizCategoryViewShortcutItem::WizCategoryViewShortcutItem(WizExplorerApp& app,
                                                           const QString& strName, ShortcutType type, const QString& strKbGuid,
                                                           const QString& strGuid, const QString& location, bool bEncrypted)
    : WizCategoryViewItemBase(app, strName, strKbGuid, Category_ShortcutItem)
    , m_strGuid(strGuid)
    , m_type(type)
    , m_location(location)
{
    QIcon icon;
    switch (type) {
    case Document:
    {
        if (bEncrypted)
            icon = WizLoadSkinIcon(app.userSettings().skin(), "document_badge_encrypted");
        else
            icon = WizLoadSkinIcon(app.userSettings().skin(), "document_badge");
    }
        break;
    case PersonalFolder:
    case GroupTag:
        icon = WizLoadSkinIcon(app.userSettings().skin(), "category_folder_normal");
        break;
    case PersonalTag:
        icon = WizLoadSkinIcon(app.userSettings().skin(), "category_tag");
        break;
    }

    //
    setIcon(0, icon);
    setText(0, strName);
}

void WizCategoryViewShortcutItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showShortcutContextMenu(pos);
    }
}

bool WizCategoryViewShortcutItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    switch (m_type) {
    case Document:
        //现在笔记列表会显示快捷方式中笔记所在文件夹的所有笔记，所以允许该文件夹下所有笔记
    {
        if(db.isGroup())
        {
            CWizStdStringArray arrayTag1;
            db.getDocumentTags(data.strGUID, arrayTag1);
            CWizStdStringArray arrayTag2;
            m_app.databaseManager().db(m_strKbGUID).getDocumentTags(m_strGuid, arrayTag2);

            return (arrayTag1.size() == 1 && arrayTag2.size() == 1 && arrayTag1.front() == arrayTag2.front());
        }
        else
        {
            WIZDOCUMENTDATA doc;
            m_app.databaseManager().db(m_strKbGUID).documentFromGuid(m_strGuid, doc);
            return doc.strLocation == data.strLocation;
        }
    }
        break;
    case PersonalFolder:
        return data.strLocation == m_location;
        break;
    case PersonalTag:
    {
        CWizStdStringArray arrayTag;
        m_app.databaseManager().db().getDocumentTags(data.strGUID, arrayTag);
        for (CString tag : arrayTag)
        {
            if (tag == m_strGuid)
                return true;
        }
    }
        break;
    case GroupTag:
    {
        if (data.strKbGUID != m_strKbGUID)
            return false;

        CWizStdStringArray arrayTag;
        m_app.databaseManager().db(data.strKbGUID).getDocumentTags(data.strGUID, arrayTag);
        for (CString tag : arrayTag)
        {
            if (tag == m_strGuid)
                return true;
        }
    }
        break;
    default:
        break;
    }
    return false;
}


WizCategoryViewShortcutPlaceHoldItem::WizCategoryViewShortcutPlaceHoldItem(
        WizExplorerApp& app, const QString& strName)
    : WizCategoryViewItemBase(app, strName, "", Category_ShortcutPlaceHoldItem)
{

}

int WizCategoryViewShortcutPlaceHoldItem::getItemHeight(int /*hintHeight*/) const
{
    return WizSmartScaleUI(20);
}

void WizCategoryViewShortcutPlaceHoldItem::drawItemBody(QPainter *p, const QStyleOptionViewItem *vopt) const
{
    QRect rcIcon = treeWidget()->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, vopt, treeWidget());
    QRect rcText(rcIcon.right() + 8, vopt->rect.top(), vopt->rect.right() - rcIcon.right() - 24,
                 vopt->rect.height());

    QString strText = vopt->text;
    if (!strText.isEmpty()) {
        bool isSelected = vopt->state & QStyle::State_Selected;
        QColor colorText(isSelected ? "#FFFFFF" : "#BEBEBE");
        colorText.setAlpha(240);
        p->setPen(colorText);
        QFont f;
        f.setPixelSize(::WizSmartScaleUI(10));
        f.setStyleStrategy(QFont::PreferBitmap);
        QFontMetrics fm(f);
        strText = fm.elidedText(strText, Qt::ElideRight, rcText.width());
        int right = Utils::WizStyleHelper::drawSingleLineText(p, rcText, strText, Qt::AlignVCenter, colorText, f);
        //
        if (right != -1) {
            rcText.setLeft(right + 4);
        }
    }
}