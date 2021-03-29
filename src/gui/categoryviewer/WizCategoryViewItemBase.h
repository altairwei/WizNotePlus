#ifndef GUI_CATEGORYVIEWER_WIZCATEGORYVIEWITEMBASE_H
#define GUI_CATEGORYVIEWER_WIZCATEGORYVIEWITEMBASE_H

#include <QString>
#include <QPainter>
#include <QTreeWidgetItem>

#include "share/WizObject.h"

#define PREDEFINED_TRASH            QObject::tr("Trash")
#define PREDEFINED_UNCLASSIFIED     QObject::tr("Unclassified")

#define CATEGORY_OWN_GROUPS      QObject::tr("My Groups")
#define CATEGORY_OTHER_GROUPS      QObject::tr("Other Groups")


#define WIZ_CATEGORY_SECTION_GENERAL QObject::tr("General")
#define WIZ_CATEGORY_SECTION_PERSONAL QObject::tr("Personal Notes")
#define WIZ_CATEGORY_SECTION_GROUPS QObject::tr("Team Notes")

#define WIZ_CATEGORY_SHOTCUT_PLACEHOLD QObject::tr("Drag note form note list")

const int nNumberButtonHeight = 14;
const int nNumberButtonHorizontalMargin = 3;
const int EXTRABUTTONRIGHTMARGIN = 10;

enum ItemType
{
    Category_WizNoneItem = QTreeWidgetItem::UserType + 1,
    Category_MessageRootItem,
    Category_MessageItem,
    Category_ShortcutRootItem,
    Category_ShortcutPlaceHoldItem,
    Category_ShortcutItem,
    Category_QuickSearchRootItem,
    Category_QuickSearchItem,
    Category_QuickSearchCustomItem,
    Category_AllFoldersItem,
    Category_FolderItem,
    Category_AllTagsItem,
    Category_TagItem,
    Category_GroupsRootItem,
    Category_BizGroupRootItem,
    Category_OwnGroupRootItem,
    Category_JoinedGroupRootItem,
    Category_GroupRootItem,
    Category_GroupItem,
    Category_GroupNoTagItem,
    Category_SectionItem
};

enum DateInterval
{
    DateInterval_Today,
    DateInterval_Yestoday,
    DateInterval_TheDayBeforeYestoday,
    DateInterval_LastWeek,
    DateInterval_LastMonth,
    DateInterval_LastYear
};


class WizDatabase;
class WizExplorerApp;
class WizCategoryBaseView;

class WizCategoryViewItemBase : public QTreeWidgetItem
{

public:
    WizCategoryViewItemBase(WizExplorerApp& app, const QString& strName = "", const QString& strKbGUID = "", int type = Type);
    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos) = 0;
    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument) = 0;
    virtual bool accept(WizDatabase& db, const WIZDOCUMENTDATA& data) { Q_UNUSED(data); return false; }
    virtual bool acceptDrop(const WizCategoryViewItemBase* pItem) const { Q_UNUSED(pItem); return false;}
    virtual bool acceptDrop(const WIZDOCUMENTDATA& data) const { Q_UNUSED(data); return false; }
    virtual bool acceptDrop(const QString& urls) const { Q_UNUSED(urls); return false; }
    virtual bool dragAble() const { return false; }
    virtual void drop(const CWizDocumentDataArray& arrayDocument, bool forceCopy = false) { Q_UNUSED(arrayDocument); Q_UNUSED(forceCopy);}
    virtual void drop(const WizCategoryViewItemBase* pItem) { Q_UNUSED(pItem); }

    virtual bool acceptMousePressedInfo() { return false; }
    virtual void mousePressed(const QPoint& pos) { Q_UNUSED(pos); }
    virtual void mouseReleased(const QPoint& pos) { Q_UNUSED(pos); }

    virtual void drawItemBody(QPainter* p, const QStyleOptionViewItem* vopt) const;
    virtual void drawExtraBadge(QPainter* p, const QStyleOptionViewItem* vopt) const;

    virtual QVariant data(int column, int role) const;
    virtual int getItemHeight(int hintHeight) const;
    virtual bool operator<(const QTreeWidgetItem &other) const;

    const QString& kbGUID() const { return m_strKbGUID; }
    const QString& name() const { return m_strName; }

    virtual QString id() const;

    void setDocumentsCount(int nCurrent, int nTotal);

    //
    virtual int getSortOrder() const { return 0; }

    //
    virtual QString getSectionName() { return QString(); }

    //Extra Button
    virtual void setExtraButtonIcon(const QString &file);
    virtual bool getExtraButtonIcon(QPixmap &ret) const;
    virtual QRect getExtraButtonRect(const QRect &itemBorder, bool ignoreIconExist = false) const;
    virtual bool extraButtonClickTest();
    virtual QString getExtraButtonToolTip() const;

    //
    virtual QString countString() const { return m_countString; }

protected:
    WizExplorerApp& m_app;
    QString m_strName;
    QString m_strKbGUID;
    QPixmap m_extraButtonIcon;
    QString m_countString;
    bool m_extraButtonIconPressed;
};


void drawClickableUnreadButton(QPainter* p, const QRect& rcd, const QString& text, bool isPressed);
QString unreadNumToString(int unread);

#endif // GUI_CATEGORYVIEWER_WIZCATEGORYVIEWITEMBASE_H
