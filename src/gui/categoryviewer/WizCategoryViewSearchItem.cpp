#include "WizCategoryViewSearchItem.h"

#include "WizDef.h"
#include "share/WizSettings.h"
#include "share/WizMisc.h"

#include "gui/categoryviewer/WizCategoryView.h"

/* -------------------- CWizCategoryViewSearchRootItem -------------------- */
WizCategoryViewSearchRootItem::WizCategoryViewSearchRootItem(WizExplorerApp& app,
                                                               const QString& strName)
    : WizCategoryViewItemBase(app, strName, "", Category_QuickSearchRootItem)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_search");
    setIcon(0, icon);
    setText(0, strName);
}

void WizCategoryViewSearchRootItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showCustomSearchContextMenu(pos, false);
    }
}

QString WizCategoryViewSearchRootItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GENERAL;
}

/* ------------------------------ CWizCategoryViewSearchItem ------------------------------ */

//CWizCategoryViewSearchItem::CWizCategoryViewSearchItem(CWizExplorerApp& app, const QString& keywords)
//    : CWizCategoryViewItemBase(app, keywords)
//{
//    setKeywords(keywords);
//    setIcon(0, WizLoadSkinIcon(app.userSettings().skin(), QColor(0xff, 0xff, 0xff), "search"));
//}

//bool CWizCategoryViewSearchItem::accept(CWizDatabase& db, const WIZDOCUMENTDATA& data)
//{
//    Q_UNUSED(db);

//    if (m_strName.isEmpty())
//        return false;

//    return -1 != ::WizStrStrI_Pos(data.strTitle, m_strName);
//}

//void CWizCategoryViewSearchItem::setKeywords(const QString& keywords)
//{
//    m_strName = keywords;

//    QString strText = QObject::tr("Search for %1").arg(m_strName);

//    setText(0, strText);
//}


WizCategoryViewSearchItem::WizCategoryViewSearchItem(WizExplorerApp& app,
                                                       const QString& strName, int type)
    : WizCategoryViewItemBase(app, strName, "", type)
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_searchItem");
    setIcon(0, icon);
    setText(0, strName);    
}

void WizCategoryViewSearchItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showCustomSearchContextMenu(pos, false);
    }
}

bool WizCategoryViewTimeSearchItem::operator<(const QTreeWidgetItem& other) const
{
    const WizCategoryViewTimeSearchItem* pOther = dynamic_cast<const WizCategoryViewTimeSearchItem*>(&other);
    if (!pOther) {
        return false;
    }

    return m_dateInterval < pOther->m_dateInterval;
}


WizCategoryViewTimeSearchItem::WizCategoryViewTimeSearchItem(WizExplorerApp& app,
                                                               const QString& strName, const QString strSelectParam, DateInterval interval)
    : WizCategoryViewSearchItem(app, strName)
    , m_dateInterval(interval)
    , m_strSelectParam(strSelectParam)
{
}

QString WizCategoryViewTimeSearchItem::getSQLWhere()
{
    WizOleDateTime dt;
    switch (m_dateInterval) {
    case DateInterval_Today:
        dt = dt.addDays(-1);
        break;
    case DateInterval_Yestoday:
        dt = dt.addDays(-2);
        break;
    case DateInterval_TheDayBeforeYestoday:
        dt = dt.addDays(-3);
        break;
    case DateInterval_LastWeek:
        dt = dt.addDays(-8);
        break;
    case DateInterval_LastMonth:
        dt = dt.addMonths(-1);
        break;
    case DateInterval_LastYear:
        dt = dt.addYears(-1);
        break;
    default:
        break;
    }
    QString str = m_strSelectParam;
    str.replace("%1", TIME2SQL(dt));
    return str;
}


WizCategoryViewCustomSearchItem::WizCategoryViewCustomSearchItem(WizExplorerApp& app,
                                                                   const QString& strName, const QString strSelectParam,
                                                                   const QString strSqlWhere, const QString& strGuid,
                                                                   const QString& keyword, int searchScope)
    : WizCategoryViewSearchItem(app, strName, Category_QuickSearchCustomItem)
    , m_strSelectParam(strSelectParam)
    , m_strSQLWhere(strSqlWhere)
    , m_strKeywrod(keyword)
    , m_nSearchScope(searchScope)
{
    m_strKbGUID = strGuid;
}

void WizCategoryViewCustomSearchItem::showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
{
    if (WizCategoryView* view = dynamic_cast<WizCategoryView *>(pCtrl)) {
        view->showCustomSearchContextMenu(pos, true);
    }
}

QString WizCategoryViewCustomSearchItem::getSQLWhere()
{
    return m_strSQLWhere;
}

void WizCategoryViewCustomSearchItem::setSQLWhere(const QString& strSql)
{
    m_strSQLWhere = strSql;
}

QString WizCategoryViewCustomSearchItem::getSelectParam()
{
    return m_strSelectParam;
}

void WizCategoryViewCustomSearchItem::setSelectParam(const QString& strParam)
{
    m_strSelectParam = strParam;
}

void WizCategoryViewCustomSearchItem::setKeyword(const QString& strKeyword)
{
    m_strKeywrod = strKeyword;
}

QString WizCategoryViewCustomSearchItem::getKeyword()
{
    return m_strKeywrod;
}
int WizCategoryViewCustomSearchItem::searchScope() const
{
    return m_nSearchScope;
}

void WizCategoryViewCustomSearchItem::setSearchScope(int nSearchScope)
{
    m_nSearchScope = nSearchScope;
}