#include "WizCategoryViewItem.h"

#include <QTextCodec>
#include <QPainter>
#include <cstring>
#include <QFile>
#include <QStyle>
#include <QDebug>

#include "utils/WizPinyin.h"
#include "utils/WizStyleHelper.h"
#include "utils/WizNotify.h"
#include "utils/WizLogger.h"
#include "utils/WizMisc.h"

#include "gui/categoryviewer/WizCategoryView.h"
#include "WizMainWindow.h"
#include "WizDocumentTransitionView.h"
#include "share/WizObjectDataDownloader.h"
#include "share/WizObjectOperator.h"
#include "WizProgressDialog.h"
#include "widgets/WizTipsWidget.h"

#include "WizDef.h"
#include "WizNoteStyle.h"

#include "share/WizSettings.h"
#include "share/WizGlobal.h"
#include "share/WizDatabaseManager.h"
#include "share/WizMisc.h"



/* --------------------- CWizCategoryViewStyleRootItem --------------------- */
WizCategoryViewStyleRootItem::WizCategoryViewStyleRootItem(WizExplorerApp& app,
                                                             const QString& strName)
    : WizCategoryViewItemBase(app, strName)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "style_normal");
    setIcon(0, icon);
    setText(0, strName);
}

QString WizCategoryViewStyleRootItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_PERSONAL;
}


QString WizCategoryViewCreateGroupLinkItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GROUPS;
}


/* ------------------------------ CWizCategoryViewTrashItem ------------------------------ */

WizCategoryViewTrashItem::WizCategoryViewTrashItem(WizExplorerApp& app,
                                                     const QString& strKbGUID)
    : WizCategoryViewFolderItem(app, "/Deleted Items/", strKbGUID)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_trash");
    setIcon(0, icon);
    setText(0, PREDEFINED_TRASH);
}

void WizCategoryViewTrashItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showTrashContextMenu(pos);
    }
}

void WizCategoryViewTrashItem::getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
{
    db.getDocumentsByLocation(db.getDeletedItemsLocation(), arrayDocument, true);
}

bool WizCategoryViewTrashItem::accept(WizDatabase& db, const WIZDOCUMENTDATA& data)
{
    if (kbGUID() != data.strKbGUID)
        return false;
    //
    return db.isInDeletedItems(data.strLocation);
}

bool WizCategoryViewTrashItem::acceptDrop(const WIZDOCUMENTDATA &data) const
{
    Q_UNUSED(data);

    WizCategoryViewGroupRootItem* parentItem = dynamic_cast<WizCategoryViewGroupRootItem*>(parent());
    if (parentItem)
        return false;

    return true;
}

bool WizCategoryViewTrashItem::acceptDrop(const WizCategoryViewItemBase* pItem) const
{
    return false;
}

void WizCategoryViewTrashItem::drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy)
{
    CWizDocumentDataArray arrayOp;
    for (WIZDOCUMENTDATA doc : arrayDocument)
    {
        if (!acceptDrop(doc))
            continue;

        arrayOp.push_back(doc);
    }

    if (arrayOp.empty())
        return;

    WizDocumentOperator documentOperator(m_app.databaseManager());
    documentOperator.deleteDocuments(arrayOp);
}


void WizCategoryViewLinkItem::drawItemBody(QPainter *p, const QStyleOptionViewItem *vopt) const
{
    QString str = vopt->text;
    QRect rc(vopt->rect);
    rc.setLeft(rc.left() + 16);
    QFont fontLink = p->font();
    //fontLink.setItalic(true);
    fontLink.setPixelSize(::WizSmartScaleUI(12));
    Utils::WizStyleHelper::drawSingleLineText(p, rc, str, Qt::AlignTop, Utils::WizStyleHelper::treeViewItemLinkText(), fontLink);
}
