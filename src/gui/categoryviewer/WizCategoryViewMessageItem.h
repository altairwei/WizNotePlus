#ifndef GUI_CATEGORYVIEWER_WIZCATEGORYVIEWMESSAGEITEM_H
#define GUI_CATEGORYVIEWER_WIZCATEGORYVIEWMESSAGEITEM_H

#include "gui/categoryviewer/WizCategoryViewItemBase.h"


class WizCategoryViewMessageItem : public WizCategoryViewItemBase
{
public:
    enum FilterType {
        All,
        SendToMe,
        ModifyNote,
        comment,
        SendFromMe
    };

    WizCategoryViewMessageItem(WizExplorerApp& app, const QString& strName, int nFilter);
    virtual void drawExtraBadge(QPainter* p, const QStyleOptionViewItem *vopt) const;

    virtual void showContextMenu(WizCategoryBaseView* pCtrl, QPoint pos)
    { Q_UNUSED(pCtrl); Q_UNUSED(pos); }

    virtual void getDocuments(WizDatabase& db, CWizDocumentDataArray& arrayDocument)
    {  Q_UNUSED(arrayDocument); }


    virtual bool acceptMousePressedInfo() { return true; }
    virtual void mousePressed(const QPoint& pos);
    virtual void mouseReleased(const QPoint& pos);

    void getMessages(WizDatabase& db, const QString& userGUID, CWizMessageDataArray& arrayMsg);
    void setUnreadCount(int nCount);
    QString unreadString() const;
    bool hitTestUnread();

    virtual QString getSectionName();
    virtual int getSortOrder() const { return 10; }

    virtual QRect getExtraButtonRect(const QRect &itemBorder, bool ignoreIconExist = false) const;

//    void showCoachingTips();
private:
    int m_nFilter;
    int m_nUnread;
    QSize m_szUnreadSize;   
};

#endif // GUI_CATEGORYVIEWER_WIZCATEGORYVIEWMESSAGEITEM_H