#include "WizCategoryViewItemBase.h"

#include <QDebug>
#include <QFile>

#include "WizCategoryView.h"

#include "share/WizMisc.h"
#include "utils/WizStyleHelper.h"
#include "utils/WizPinyin.h"

/* ------------------------------ CWizCategoryViewItemBase ------------------------------ */

WizCategoryViewItemBase::WizCategoryViewItemBase(WizExplorerApp& app,
                                                   const QString& strName,
                                                   const QString& strKbGUID, int type)
    : QTreeWidgetItem(type)
    , m_app(app)
    , m_strName(strName)
    , m_strKbGUID(strKbGUID)
    , m_extraButtonIconPressed(false)
{
}

void WizCategoryViewItemBase::drawItemBody(QPainter *p, const QStyleOptionViewItem *vopt) const
{
    bool bSelected = vopt->state.testFlag(QStyle::State_Selected);
    // 获得Area for a view item's decoration (icon).
    QRect rcIcon = treeWidget()->style()->subElementRect(QStyle::SE_ItemViewItemDecoration, vopt, treeWidget());
    if (!vopt->icon.isNull()) {
        const int iconSize = 16;
        //const int iconSize = WizSmartScaleUI(14);
        rcIcon.adjust(-6, 0, 0, 0);
        rcIcon.setTop(vopt->rect.top() + (vopt->rect.height() - iconSize) / 2);
        rcIcon.setWidth(iconSize);
        rcIcon.setHeight(iconSize);
        Utils::WizStyleHelper::drawTreeViewItemIcon(p, rcIcon, vopt->icon, bSelected);
    }

    QFont f;
    Utils::WizStyleHelper::fontCategoryItem(f);

    QFont fontCount;
    Utils::WizStyleHelper::fontExtend(fontCount);

    //获得Area for a view item's text.
    //通过subElementRect获取范围会产生不同的结果。此处通过icon进行计算
    //QRect rcText = subElementRect(SE_ItemViewItemText, vopt, view);
    QRect rcText(rcIcon.right() + 8, vopt->rect.top(), vopt->rect.right() - rcIcon.right() - 20,
                 vopt->rect.height());
    QString strCount = countString();

    QString strText = vopt->text;
    if (!strText.isEmpty()) {
        bool secondLevelFolder = (parent() && (parent()->type() == Category_GroupItem
                                                     || parent()->type() == Category_FolderItem));
        QColor colorText = Utils::WizStyleHelper::treeViewItemText(bSelected, secondLevelFolder);
        colorText.setAlpha(240);
        p->setPen(colorText);
        f.setStyleStrategy(QFont::PreferBitmap);
        QFontMetrics fm(f);
        strText = fm.elidedText(strText, Qt::ElideRight, rcText.width());
        int right = Utils::WizStyleHelper::drawSingleLineText(p, rcText, strText, Qt::AlignVCenter, colorText, f);
        //
        if (right != -1) {
            rcText.setLeft(right + 4);
        }
    }

    if (!strCount.isEmpty() && (rcText.width() > 10)) {
        QRect rcCount = rcText;
        rcCount.setTop(rcCount.top() + 1);  //add extra 1 pixel for vcenter / 2
        QColor colorCount = Utils::WizStyleHelper::treeViewItemTextExtend(bSelected);
        Utils::WizStyleHelper::drawSingleLineText(p, rcCount, strCount, Qt::AlignVCenter, colorCount, fontCount);
    }
}

bool WizCategoryViewItemBase::operator < (const QTreeWidgetItem &other) const
{
    const WizCategoryViewItemBase* pOther = dynamic_cast<const WizCategoryViewItemBase*>(&other);
    if (!pOther)
        return false;
    //
    int nThis = getSortOrder();
    int nOther = pOther->getSortOrder();
    //
    if (nThis != nOther)
    {
        return nThis < nOther;
    }
    //
    QString strThis = text(0).toLower();
    QString strOther = pOther->text(0).toLower();
    //
    bool ret = WizToolsSmartCompare(strThis, strOther) < 0;
    //
    if (strThis == "drds_ks" && strOther[0] == 'h') {
        qDebug() << ret;
    }
    return ret;
}

QVariant WizCategoryViewItemBase::data(int column, int role) const
{
    if (role == Qt::SizeHintRole) {
        int fontHeight = treeWidget()->fontMetrics().height();
        int defHeight = fontHeight + 8;
        int height = getItemHeight(defHeight);
        QSize sz(-1, height);
        return QVariant(sz);
    } else {
        return QTreeWidgetItem::data(column, role);
    }
}

int WizCategoryViewItemBase::getItemHeight(int /*hintHeight*/) const
{
    return Utils::WizStyleHelper::treeViewItemHeight();
}


QString WizCategoryViewItemBase::id() const
{
    return ::WizMd5StringNoSpaceJava(QString(text(0) + m_strKbGUID).toUtf8());
}

void WizCategoryViewItemBase::setDocumentsCount(int nCurrent, int nTotal)
{
    Q_ASSERT(nTotal != -1);

    if (nCurrent == -1)
    {
        if (nTotal == 0)
        {
            m_countString = "";
        }
        else
        {
            m_countString = QString("(%1)").arg(nTotal);
        }
    }
    else
    {
        m_countString = QString("(%1/%2)").arg(nCurrent).arg(nTotal);
    }
}

void WizCategoryViewItemBase::setExtraButtonIcon(const QString& file)
{
    if (WizIsHighPixel())
    {
        int nIndex = file.lastIndexOf('.');
        QString strFile = file.left(nIndex) + "@2x" + file.right(file.length() - nIndex);
        if (!strFile.isEmpty() && QFile::exists(strFile))
        {
            m_extraButtonIcon = QPixmap(strFile);
            return;
        }
    }

    m_extraButtonIcon = QPixmap(file);
}

bool WizCategoryViewItemBase::getExtraButtonIcon(QPixmap &ret) const
{
    ret = m_extraButtonIcon;
    return !m_extraButtonIcon.isNull();
}

QRect WizCategoryViewItemBase::getExtraButtonRect(const QRect &rcItemBorder, bool ignoreIconExist) const
{
    QSize szBtn(16, 16);
    if (!m_extraButtonIcon.isNull())
    {
        szBtn = m_extraButtonIcon.size();
        WizScaleIconSizeForRetina(szBtn);
    }
    else if (!ignoreIconExist)
    {
        return QRect(0, 0, 0, 0);
    }
    int nWidth = szBtn.width();
    int nHeight = szBtn.height();
    //
    int nTop = rcItemBorder.y() + (rcItemBorder.height() - nHeight) / 2;
    QRect rcb(rcItemBorder.right() - nWidth - EXTRABUTTONRIGHTMARGIN, nTop, nWidth, nHeight);
    return rcb;
}

bool WizCategoryViewItemBase::extraButtonClickTest()
{
    QPixmap pixmap;
    if(!getExtraButtonIcon(pixmap) || pixmap.isNull())
        return false;

    WizCategoryBaseView* view = dynamic_cast<WizCategoryBaseView*>(treeWidget());
    Q_ASSERT(view);

    QRect rcIemBorder = view->visualItemRect(this);
    QRect btnRect = getExtraButtonRect(rcIemBorder);
    int nClickDist = 2;
    btnRect.adjust(-nClickDist, -nClickDist, nClickDist, nClickDist);

    return btnRect.contains(view->hitPoint());
}

QString WizCategoryViewItemBase::getExtraButtonToolTip() const
{
    return "";
}

void WizCategoryViewItemBase::drawExtraBadge(QPainter* p, const QStyleOptionViewItem* vopt) const
{


    QPixmap pixmap;
    if(getExtraButtonIcon(pixmap) && !pixmap.isNull())
    {
        p->save();

        QRect rcb = getExtraButtonRect(vopt->rect);
        p->setRenderHint(QPainter::Antialiasing);
        p->setClipRect(rcb);
        p->drawPixmap(rcb, pixmap);

        p->restore();
    }



#if 0
    if (!vopt->icon.isNull()) {
        QRect iconRect = subElementRect(SE_ItemViewItemDecoration, vopt, view);

        if (vopt->state.testFlag(State_Selected)) {
            vopt->icon.paint(p, iconRect, Qt::AlignCenter, QIcon::Selected);
        } else {
            vopt->icon.paint(p, iconRect, Qt::AlignCenter, QIcon::Normal);
        }
    }

    // text should not empty
    if (vopt->text.isEmpty()) {
        Q_ASSERT(0);
        return;
    }

    // draw text little far from icon than the default
    QRect textRect = subElementRect(SE_ItemViewItemText, vopt, view);
    //textRect.adjust(8, 0, 0, 0);

    // draw the text
    QPalette::ColorGroup cg = vopt->state & QStyle::State_Enabled
            ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
        cg = QPalette::Inactive;

    if (vopt->state & QStyle::State_Selected) {
        p->setPen(vopt->palette.color(cg, QPalette::HighlightedText));
    } else {
        p->setPen(vopt->palette.color(cg, QPalette::Text));
    }

    if (vopt->state & QStyle::State_Editing) {
        p->setPen(vopt->palette.color(cg, QPalette::Text));
        p->drawRect(textRect.adjusted(0, 0, -1, -1));
    }

    // compute document count string length and leave enough space for drawing
    QString strCount = pItem->m_countString;
    int nCountWidthMax;
    int nMargin = 3;
    QFont fontCount = p->font();
    fontCount.setPointSize(10);

    if (!strCount.isEmpty()) {
        QFont fontOld = p->font();
        p->setFont(fontCount);
        nCountWidthMax = p->fontMetrics().width(strCount) + nMargin;
        textRect.adjust(0, 0, -nCountWidthMax, 0);
        p->setFont(fontOld);
    }

    QFont f = p->font();
    f.setPixelSize(12);
    p->setFont(f);

    QColor colorText = vopt->state.testFlag(State_Selected) ?
                m_colorCategoryTextSelected : m_colorCategoryTextNormal;

    CString strText = vopt->text;
    int nWidth = ::WizDrawTextSingleLine(p, textRect, strText,
                                         Qt::TextSingleLine | Qt::AlignVCenter,
                                         colorText, true);

    // only draw document count if count string is not empty
    if (!strCount.isEmpty()) {
        p->setFont(fontCount);
        textRect.adjust(nWidth + nMargin, 0, nCountWidthMax, 0);
        QColor colorCount = vopt->state.testFlag(State_Selected) ? QColor(230, 230, 230) : QColor(150, 150, 150);
        CString strCount2(strCount);
        ::WizDrawTextSingleLine(p, textRect, strCount2,  Qt::TextSingleLine | Qt::AlignVCenter, colorCount, false);
    }

    p->restore();

#endif
}

void drawClickableUnreadButton(QPainter* p, const QRect& rcd, const QString& text, bool isPressed)
{
    QFont f;
    f.setPixelSize(::WizSmartScaleUI(10));
    p->setFont(f);
    p->setPen("999999");
    //
    QRect rcb = rcd;
    if (isPressed)
    {
        rcb.adjust(0, 0, 0, 2);
        QPixmap pixBg(Utils::WizStyleHelper::skinResourceFileName("category_unreadButton_selected", true));
        p->drawPixmap(rcb, pixBg);
        rcb.adjust(0, 0, 0, -2);
        p->drawText(rcb, Qt::AlignCenter, text);
    }
    else
    {
        rcb.adjust(0, 0, 0, 2);
        QPixmap pixBg(Utils::WizStyleHelper::skinResourceFileName("category_unreadButton", true));
        p->drawPixmap(rcb, pixBg);
        rcb.adjust(0, 0, 0, -2);
        p->drawText(rcb, Qt::AlignCenter, text);
    }
}

QString unreadNumToString(int unread)
{
    if (unread <= 0)
        return "";
    else if (unread > 99)
        return "99+";
    else
        return QString::number(unread);
}