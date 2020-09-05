#include "WizCategoryViewMessageItem.h"

#include "WizDef.h"
#include "share/WizMisc.h"
#include "share/WizSettings.h"
#include "utils/WizStyleHelper.h"
#include "gui/categoryviewer/WizCategoryView.h"


/* -------------------- CWizCategoryViewMessageRootItem -------------------- */
WizCategoryViewMessageItem::WizCategoryViewMessageItem(WizExplorerApp& app,
                                                                 const QString& strName, int nFilterType)
    : WizCategoryViewItemBase(app, strName, "", Category_MessageItem)
    , m_nUnread(0)    
{
    QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), "category_messages");
    setIcon(0, icon);
    setText(0, strName);

    m_nFilter = nFilterType;
}

void WizCategoryViewMessageItem::getMessages(WizDatabase& db, const QString& userGUID, CWizMessageDataArray& arrayMsg)
{
    if (hitTestUnread() && m_nUnread) {
        if (userGUID.isEmpty()) {
            db.getUnreadMessages(arrayMsg);
        } else {
            db.unreadMessageFromUserGUID(userGUID, arrayMsg);
        }
    } else {
        if (userGUID.isEmpty()) {
            db.getLastestMessages(arrayMsg);
        } else {
            db.messageFromUserGUID(userGUID, arrayMsg);
        }
    }
}

void WizCategoryViewMessageItem::setUnreadCount(int nCount)
{
   m_nUnread = nCount;

#ifdef Q_OS_MAC
    Utils::WizNotify::setDockBadge(nCount);
#endif

   m_nUnread = nCount;
   WizCategoryBaseView* view = dynamic_cast<WizCategoryBaseView*>(treeWidget());
   Q_ASSERT(view);
   //
   if (m_nUnread > 0)
   {
       QFont f;
       Utils::WizStyleHelper::fontNormal(f);
       QFontMetrics fm(f);
       //
       QSize szText = fm.size(0, unreadString());
       int textWidth = szText.width();
//       int textHeight = szText.height();
       //
       //int nMargin = textHeight / 4;
       //
       int nWidth = textWidth + nNumberButtonHorizontalMargin * 2;
       int nHeight = nNumberButtonHeight;
//       int nHeight = textHeight + 2;
       if (nWidth < nHeight)
           nWidth = nHeight;
       //
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

QString WizCategoryViewMessageItem::unreadString() const
{
    return unreadNumToString(m_nUnread);
}

bool WizCategoryViewMessageItem::hitTestUnread()
{
    if (m_nUnread == 0)
        return false;

    WizCategoryBaseView* view = dynamic_cast<WizCategoryBaseView*>(treeWidget());
    Q_ASSERT(view);

    QRect rcItem = view->visualItemRect(this);
    QPoint pt = view->hitPoint();
    //
    QRect rcRect = getExtraButtonRect(rcItem, true);
    return rcRect.contains(pt);
}

QString WizCategoryViewMessageItem::getSectionName()
{
    return WIZ_CATEGORY_SECTION_GENERAL;
}

QRect WizCategoryViewMessageItem::getExtraButtonRect(const QRect& itemBorder, bool ignoreIconExist) const
{
    if (!m_nUnread && !ignoreIconExist)
        return QRect();

    int nButtonWidth = 26;
    int nButtonHeight = 14;
    QRect rc(itemBorder.right() - EXTRABUTTONRIGHTMARGIN - nButtonWidth, itemBorder.y() + (itemBorder.height() - nButtonHeight) / 2,
             nButtonWidth, nButtonHeight);
    return rc;
}

//#define CATEGORYMESSAGEITEMTIPSCHECKED "CategoryMessageItemTipsChecked"

//void CWizCategoryViewMessageItem::showCoachingTips()
//{
//    bool showTips = false;
//    if (MainWindow* mainWindow = MainWindow::instance())
//    {
//        showTips = mainWindow->userSettings().get(CATEGORYMESSAGEITEMTIPSCHECKED).toInt() == 0;
//    }

//    if (showTips)
//    {
//        CWizTipListManager* manager = CWizTipListManager::instance();
//        if (manager->tipsWidgetExists(CATEGORYMESSAGEITEMTIPSCHECKED))
//            return;

//        CWizTipsWidget* tipWidget = new CWizTipsWidget(CATEGORYMESSAGEITEMTIPSCHECKED, this);
//        tipWidget->setAttribute(Qt::WA_DeleteOnClose, true);
//        tipWidget->setText(tr("More tool items"), tr("Use to show or hide extra tool items."));
//        tipWidget->setSizeHint(QSize(280, 60));
//        tipWidget->setButtonVisible(false);
//        tipWidget->bindFunction([](){
//            if (MainWindow* mainWindow = MainWindow::instance())
//            {
//                mainWindow->userSettings().set(CATEGORYMESSAGEITEMTIPSCHECKED, "1");
//            }
//        });
//        //
//        tipWidget->addToTipListManager(m_btnShowExtra, 0, -6);
//    }
//}

void WizCategoryViewMessageItem::drawExtraBadge(QPainter* p, const QStyleOptionViewItem *vopt) const
{
    if (!m_nUnread)
        return;
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

void WizCategoryViewMessageItem::mousePressed(const QPoint& pos)
{
    QRect rcBorder = treeWidget()->visualItemRect(this);
    QRect rcIcon = getExtraButtonRect(rcBorder, true);
    if (rcIcon.contains(pos))
    {
        m_extraButtonIconPressed = true;
    }
}

void WizCategoryViewMessageItem::mouseReleased(const QPoint& pos)
{
    m_extraButtonIconPressed = false;
}
