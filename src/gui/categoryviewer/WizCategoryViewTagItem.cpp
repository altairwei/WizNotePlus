#include "WizCategoryViewTagItem.h"

#include "WizDef.h"
#include "database/WizDatabaseManager.h"
#include "share/WizSettings.h"

#include "gui/categoryviewer/WizCategoryView.h"


/* ------------------------------ CWizCategoryViewAllTagsItem ------------------------------ */

WizCategoryViewAllTagsItem::WizCategoryViewAllTagsItem(WizExplorerApp& app,
                                                         const QString& strName,
                                                         const QString& strKbGUID)
    : WizCategoryViewItemBase(app, strName, strKbGUID, Category_AllTagsItem)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_tags");
    setIcon(0, icon);
    setText(0, strName);
}

void WizCategoryViewAllTagsItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showTagRootContextMenu(pos);
    }
}

void WizCategoryViewAllTagsItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    Q_UNUSED(db);
    Q_UNUSED(arrayDocument);
    // no deleted
    //db.getDocumentsNoTag(arrayDocument);
}

bool WizCategoryViewAllTagsItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    CString strTagGUIDs = db.getDocumentTagGuidsString(data.strGUID);
    if (strTagGUIDs.isEmpty() && data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
}

bool WizCategoryViewAllTagsItem::acceptDrop(const WizCategoryViewItemBase* pItem) const
{
    const WizCategoryViewTagItem* item = dynamic_cast<const WizCategoryViewTagItem*>(pItem);
    return NULL != item;
}

QString WizCategoryViewAllTagsItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_PERSONAL;
}

/* ------------------------------ CWizCategoryViewTagItem ------------------------------ */

WizCategoryViewTagItem::WizCategoryViewTagItem(WizExplorerApp& app,
                                                 const WIZTAGDATA& tag,
                                                 const QString& strKbGUID)
    : WizCategoryViewItemBase(app, tag.strName, strKbGUID, Category_TagItem)
    , m_tag(tag)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_tagItem");
    setIcon(0, icon);
    setText(0, WizDatabase::tagNameToDisplayName(tag.strName));
}

QTreeWidgetItem* WizCategoryViewTagItem::clone() const
{
    return new WizCategoryViewTagItem(m_app, m_tag, m_strKbGUID);
}

void WizCategoryViewTagItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.getDocumentsByTag(m_tag, arrayDocument);
}

bool WizCategoryViewTagItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (data.strKbGUID == kbGUID()) {
        QString strTagGUIDs = db.getDocumentTagGuidsString(data.strGUID);
        if (strTagGUIDs.indexOf(m_tag.strGUID) != -1)
            return true;
    }

    return false;
}

bool WizCategoryViewTagItem::acceptDrop(const WIZDOCUMENTDATA& data) const
{
    // only accept drop from user db
    if (data.strKbGUID == kbGUID()) {
        return true;
    }

    return false;
}

bool WizCategoryViewTagItem::acceptDrop(const WizCategoryViewItemBase* pItem) const
{
    return pItem && pItem->type() == Category_TagItem;
}

void WizCategoryViewTagItem::drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy)
{
    Q_UNUSED(forceCopy);

    WizDatabase& db = WizDatabaseManager::instance()->db(kbGUID());
    for (WIZDOCUMENTDATA document : arrayDocument)
    {
        if (!acceptDrop(document))
            continue;

        // skip
        QString strTagGUIDs = db.getDocumentTagGuidsString(document.strGUID);
        if (strTagGUIDs.indexOf(m_tag.strGUID) != -1)
            continue;

        WizDocument doc(db, document);
        doc.addTag(tag());
    }
}

void WizCategoryViewTagItem::drop(const WizCategoryViewItemBase* pItem)
{
    if (pItem && pItem->type() == Category_TagItem)
    {
        const WizCategoryViewTagItem* childItem = dynamic_cast<const WizCategoryViewTagItem*>(pItem);
        WIZTAGDATA childTag = childItem->tag();
        childTag.strParentGUID = tag().strGUID;
        m_app.databaseManager().db().modifyTag(childTag);
    }
}

void WizCategoryViewTagItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showTagContextMenu(pos);
    }
}

void WizCategoryViewTagItem::reload(WizDatabase& db)
{
    db.tagFromGuid(m_tag.strGUID, m_tag);
    setText(0, m_tag.strName);
    m_strName = m_tag.strName;
}

void WizCategoryViewTagItem::setTagPosition(int nPos)
{
    m_tag.nPosition = nPos;
}