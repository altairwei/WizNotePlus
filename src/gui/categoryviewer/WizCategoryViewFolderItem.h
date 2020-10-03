#ifndef GUI_CATEGORYVIEWER_WIZCATEGORYVIEWFOLDERITEM_H
#define GUI_CATEGORYVIEWER_WIZCATEGORYVIEWFOLDERITEM_H


#include "gui/categoryviewer/WizCategoryViewItemBase.h"


/** The root of all folder items. */
class WizCategoryViewAllFoldersItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewAllFoldersItem(WizExplorerApp& app, const QString& strName, const QString& strKbGUID);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const;
    virtual bool acceptDrop(const QString& urls) const { Q_UNUSED(urls); return true; }
    virtual QString getSectionName();
    virtual int getSortOrder() const { return 20; }
};

class WizCategoryViewFolderItem : public WizCategoryViewItemBase
{
public:
    WizCategoryViewFolderItem(WizExplorerApp& app, const QString& strLocation, const QString& strKbGUID);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos);
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument);
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data);
    virtual bool acceptDrop(const WIZDOCUMENTDATA& data) const;
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const;
    virtual bool acceptDrop(const QString& urls) const { Q_UNUSED(urls); return true; }
    virtual bool dragAble() const { return true; }
    virtual void drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy = false);

    virtual bool operator < (const QTreeWidgetItem &other) const;

    virtual QTreeWidgetItem* clone() const;

    virtual QString id() const;

    QString location() const { return m_strName; }
    void setLocation(const QString& strLocation);
    QString name() const;

private:
    QRect m_rcUnread;
};


#endif // GUI_CATEGORYVIEWER_WIZCATEGORYVIEWFOLDERITEM_H