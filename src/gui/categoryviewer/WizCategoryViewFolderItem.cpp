#include "WizCategoryViewFolderItem.h"

#include "WizDef.h"
#include "share/WizObjectOperator.h"
#include "share/WizGlobal.h"
#include "share/WizSettings.h"
#include "share/WizMisc.h"

#include "gui/categoryviewer/WizCategoryView.h"


/* ------------------------------ CWizCategoryViewAllFoldersItem ------------------------------ */

WizCategoryViewAllFoldersItem::WizCategoryViewAllFoldersItem(
    WizExplorerApp& app, const QString& strName, const QString& strKbGUID)
    : WizCategoryViewItemBase(app, strName, strKbGUID, Category_AllFoldersItem)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_folders");
    setIcon(0, icon);
    setText(0, strName);
}

void WizCategoryViewAllFoldersItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.getDocumentsBySQLWhere("DOCUMENT_LOCATION not like '/Deleted Items/%' order by DT_DATA_MODIFIED desc limit 1000", arrayDocument);
}

bool WizCategoryViewAllFoldersItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (db.isInDeletedItems(data.strLocation)) {
        return false;
    }

    return !db.isGroup();
}

bool WizCategoryViewAllFoldersItem::acceptDrop(const WizCategoryViewItemBase* pItem) const
{
//    return pItem->type() == Category_FolderItem | pItem->type() == Category_GroupItem;

    const WizCategoryViewFolderItem* item = dynamic_cast<const WizCategoryViewFolderItem*>(pItem);
    return NULL != item;
}

QString WizCategoryViewAllFoldersItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_PERSONAL;
}


void WizCategoryViewAllFoldersItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showFolderRootContextMenu(pos);
    }
}


/* ------------------------------ CWizCategoryViewFolderItem ------------------------------ */

WizCategoryViewFolderItem::WizCategoryViewFolderItem(WizExplorerApp& app,
                                                       const QString& strLocation,
                                                       const QString& strKbGUID)
    : WizCategoryViewItemBase(app, strLocation, strKbGUID, Category_FolderItem)
{
    QIcon icon;
    if (::WizIsPredefinedLocation(strLocation) && strLocation == "/My Journals/") {
        icon = WizLoadSkinIcon(app.userSettings().skin(), "category_folder_diary");
    } else {
        icon  = WizLoadSkinIcon(app.userSettings().skin(), "category_folder");
    }
    setIcon(0, icon);

    auto name = WizDatabase::getLocationDisplayName(strLocation);
    setText(0, name);
    setToolTip(0, name);
}

QTreeWidgetItem* WizCategoryViewFolderItem::clone() const
{
    return new WizCategoryViewFolderItem(m_app, m_strName, m_strKbGUID);
}

QString WizCategoryViewFolderItem::id() const
{
    return ::WizMd5StringNoSpaceJava(QString(m_strName + m_strKbGUID).toUtf8());
}

void WizCategoryViewFolderItem::setLocation(const QString& strLocation)
{
    m_strName = strLocation;
}

void WizCategoryViewFolderItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.getDocumentsByLocation(m_strName, arrayDocument, m_app.userSettings().showSubFolderDocuments());
}

bool WizCategoryViewFolderItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    Q_UNUSED(db);

    if (m_strName == data.strLocation && data.strKbGUID == kbGUID())
        return true;

    if (kbGUID().isEmpty() && !db.isGroup())
        return true;

    return false;
}

bool WizCategoryViewFolderItem::acceptDrop(const WIZDOCUMENTDATA& data) const
{
    Q_UNUSED(data);

    return true;
}

bool WizCategoryViewFolderItem::acceptDrop(const WizCategoryViewItemBase* pItem) const
{
//    return pItem->type() == Category_FolderItem | pItem->type() == Category_GroupItem;

    const WizCategoryViewFolderItem* item = dynamic_cast<const WizCategoryViewFolderItem*>(pItem);
    return NULL != item;
}

void WizCategoryViewFolderItem::drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy)
{
    CWizDocumentDataArray arrayOp;
    bool needCopy = false;
    for (WIZDOCUMENTDATAEX doc : arrayDocument)
    {
        if (!acceptDrop(doc))
            continue;

        arrayOp.push_back(doc);
        if (forceCopy || kbGUID() != doc.strKbGUID)
        {
            needCopy = true;
        }
    }

    if (arrayOp.empty())
        return;

    WizDocumentOperator documentOperator(m_app.databaseManager());
    if (needCopy)
    {
        documentOperator.copyDocumentsToPersonalFolder(arrayOp, location(), false, true, true);
    }
    else
    {
        documentOperator.moveDocumentsToPersonalFolder(arrayOp, location(), false);
    }    
}

void WizCategoryViewFolderItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showFolderContextMenu(pos);
    }
}

QString WizCategoryViewFolderItem::name() const
{
    return WizDatabase::getLocationName(m_strName);
}

bool WizCategoryViewFolderItem::operator < (const QTreeWidgetItem &other) const
{
    const WizCategoryViewFolderItem* pOther = dynamic_cast<const WizCategoryViewFolderItem*>(&other);
    if (!pOther) {
        return false;
    }

//    qDebug() << "compare, this : " << name() << " , other : " << pOther->name();

    if (getSortOrder() != pOther->getSortOrder())
    {
        bool result  = getSortOrder() < pOther->getSortOrder();
//        qDebug() << "sortoder different : " << result;
        return result;
    }

    // sort by folder pos
    if (m_app.userSettings().isManualSortingEnabled())
    {
        int nThis = 0, nOther = 0;
        if (!pOther->location().isEmpty()) {
            QSettings* setting = WizGlobal::settings();
//            qDebug() << "pother location : " << pOther->location() << "  this location : " << location();
            nOther = setting->value("FolderPosition/" + pOther->location()).toInt();
            nThis = setting->value("FolderPosition/" + location()).toInt();
        }

//        qDebug() << "manual sort enable, this folder pos : " << nThis << "  other sort pos : " << nOther;

        if (nThis != nOther)
        {
            if (nThis > 0 && nOther > 0)
            {
                bool result  =  nThis < nOther;
//                qDebug() << "folder position different : " << result;
                return result;
            }
        }
    }

    //
    return WizCategoryViewItemBase::operator <(other);
}
