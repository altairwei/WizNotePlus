#ifndef GUI_CATEGORYVIEWER_WIZCATEGORYVIEWSEARCHITEM_H
#define GUI_CATEGORYVIEWER_WIZCATEGORYVIEWSEARCHITEM_H

#include "gui/categoryviewer/WizCategoryViewItemBase.h"

class WizCategoryViewSearchRootItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewSearchRootItem(WizExplorerApp& app, const QString& strName);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);

    virtual void getDocuments(WizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(arrayDocument); }

    virtual QString getSectionName();
    virtual int getSortOrder() const { return 12; }
};

class WizCategoryViewSearchItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewSearchItem(WizExplorerApp& app, const QString& strName,
                               int type = Category_QuickSearchItem);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);

    virtual void getDocuments(WizDatabase& db,
                              CWizDocumentDataArray& arrayDocument)
    { Q_UNUSED(arrayDocument); }

    virtual QString getSQLWhere() { return ""; }
    virtual QString getSelectParam() { return ""; }
};

class WizCategoryViewTimeSearchItem : public WizCategoryViewSearchItem
{
public:
    WizCategoryViewTimeSearchItem(WizExplorerApp& app, const QString& strName,
                               const QString strSelectParam, DateInterval interval);        

    virtual bool operator<(const QTreeWidgetItem &other) const;

    virtual QString getSQLWhere();

protected:
    QString m_strSelectParam;
    DateInterval m_dateInterval;
};

class WizCategoryViewCustomSearchItem : public WizCategoryViewSearchItem
{
public:
    WizCategoryViewCustomSearchItem(WizExplorerApp& app, const QString& strName,
                               const QString strSelectParam, const QString strSqlWhere,
                                     const QString& strGuid, const QString& keyword, int searchScope);

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);

    virtual QString getSQLWhere();
    virtual void setSQLWhere(const QString& strSql);
    virtual QString getSelectParam();
    virtual void setSelectParam(const QString& strParam);
    void setKeyword(const QString& strKeyword);
    QString getKeyword();
    int searchScope() const;
    void setSearchScope(int searchScope);

protected:
    QString m_strSelectParam;
    QString m_strSQLWhere;
    QString m_strKeywrod;
    int m_nSearchScope;
};

#endif // GUI_CATEGORYVIEWER_WIZCATEGORYVIEWSEARCHITEM_H