#include "WizCellButton.h"

#include <QString>
#include <QPainter>
#include <QStyleOptionToolButton>
#include <QStyleFactory>
#include <QStyle>
#include <QColor>
#include <QPalette>
#include <QSettings>
#include <QSize>
#include <QDebug>
#include <QFontMetrics>
#include <QPropertyAnimation>
#include <QStylePainter>

#include "utils/WizStyleHelper.h"
#include "share/WizMisc.h"
#include "share/WizQtHelper.h"
#include "WizNoteStyle.h"


WizCellButton::WizCellButton(ButtonType type, QWidget *parent)
    : QToolButton(parent)
    , m_state(0)
    , m_count(0)
    , m_buttonType(type)
    , m_iconSize(WizSmartScaleUI(14), WizSmartScaleUI(14))
{    
    setAutoRaise(true);
    //setStyle(new WizNotePlusStyle("fusion"));
}

void WizCellButton::setNormalIcon(const QIcon& icon, const QString& strTips)
{
    m_iconNomal = icon;
    m_strTipsNormal = strTips;

    setToolTip(strTips);
}

void WizCellButton::setCheckedIcon(const QIcon& icon, const QString& strTips)
{
    m_iconChecked = icon;
    m_strTipsChecked = strTips;

    setToolTip(strTips);
}

void WizCellButton::setBadgeIcon(const QIcon& icon, const QString& strTips)
{
    m_iconBadge = icon;
    m_strTipsBagde = strTips;

    setToolTip(strTips);
}

void WizCellButton::setState(int state)
{
    switch (state) {
    case Normal:
        setIcon(m_iconNomal);
        setToolTip(m_strTipsNormal);
        m_state = 0;
        break;
    case Checked:
        setIcon(m_iconChecked);
        setToolTip(m_strTipsChecked);
        m_state = 1;
        break;
    case Badge:
        setIcon(m_iconBadge);
        setToolTip(m_strTipsBagde);
        m_state = 2;
        break;
    default:
        Q_ASSERT(0);
    }
}

void WizCellButton::setCount(int count)
{
    m_count = count;
    update();
}

const int nTextWidth = 14;
void WizCellButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    // Create StyleOption
    QStyleOptionToolButton opt;
    initStyleOption(&opt); // autofill QToolButton information
    QPainter p(this);

    // icon State and Mode
    QIcon::Mode mode = opt.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
    if (mode == QIcon::Normal && (opt.state & QStyle::State_HasFocus || opt.state & QStyle::State_Sunken))
        // set icon active mode if focus or pushdown
        mode = QIcon::Active;
    QIcon::State state = QIcon::Off;
    if (opt.state & QStyle::State_On)
        // set icon state checkon if widget checkon
        state = QIcon::On;    

    // icon Rect
    QSize size = m_iconSize;// opt.icon.actualSize(iconSize());
    int nLeft = (opt.rect.width() - size.width()) / 2;
    if (WithCountInfo == m_buttonType)
    {
        nLeft = (opt.rect.width() - WizSmartScaleUI(nTextWidth) - size.width()) / 2;
    }
    QRect rcIcon(nLeft, (opt.rect.height() - size.height()) / 2, size.width(), size.height());

    // paint icon
    if (opt.icon.isNull())
    {
        m_iconNomal.paint(&p, rcIcon, Qt::AlignCenter, mode, state);
    }
    else
    {
        opt.icon.paint(&p, rcIcon, Qt::AlignCenter, mode, state);
    }


    // calculate text rect
    if (WithCountInfo == m_buttonType)
    {
        QRect rcText(rcIcon.right() + 5, opt.rect.y(), opt.rect.width() - rcIcon.width(), opt.rect.height());
        p.setPen(m_count == 0 ? QColor("#A7A7A7") : QColor("#5990EF"));
        p.drawText(rcText,Qt::AlignVCenter | Qt::AlignLeft, countInfo());
    }
}

QSize WizCellButton::sizeHint() const
{
    switch (m_buttonType)
    {
    case ImageOnly:
        return QSize(WizSmartScaleUI(28), WizSmartScaleUI(26));
    case WithCountInfo:
        return QSize(WizSmartScaleUI(28) + WizSmartScaleUI(nTextWidth), WizSmartScaleUI(26));
#ifdef Q_OS_WIN
    default:
        return QSize(WizSmartScaleUI(28), WizSmartScaleUI(26));
#endif
    }
}

QString WizCellButton::countInfo() const
{
    if (m_count > 99)
        return "99+";
    return QString::number(m_count);
}


namespace RoundCellButtonConst {
    const int margin = 8;
    const int spacing = 8;
    const int iconHeight = 14;
    const int fontSize = 12;
    const int buttonHeight = 18;
}


WizRoundCellButton::WizRoundCellButton(QWidget* parent)
    : WizCellButton(ImageOnly, parent)
{
    m_iconSize = QSize(iconWidth(), WizSmartScaleUI(RoundCellButtonConst::iconHeight));

    setMaximumWidth(0);
    m_animation = new QPropertyAnimation(this, "maximumWidth", this);
}

void WizRoundCellButton::setNormalIcon(const QIcon& icon, const QString& text, const QString& strTips)
{
    WizCellButton::setNormalIcon(icon, strTips);
    m_textNormal = text;
}

void WizRoundCellButton::setCheckedIcon(const QIcon& icon, const QString& text, const QString& strTips)
{
    WizCellButton::setCheckedIcon(icon, strTips);
    m_textChecked = text;
}

void WizRoundCellButton::setBadgeIcon(const QIcon& icon, const QString& text, const QString& strTips)
{
    WizCellButton::setBadgeIcon(icon, strTips);
    m_textBadge = text;
}

QString WizRoundCellButton::text() const
{
    switch (m_state) {
    case Normal:
        return m_textNormal;
        break;
    case Checked:
        return m_textChecked;
        break;
    case Badge:
        return m_textBadge;
        break;
    default:
        Q_ASSERT(0);
        break;
    }
    return "";
}

int WizRoundCellButton::iconWidth() const
{
    return WizSmartScaleUI(RoundCellButtonConst::iconHeight);

}

int WizRoundCellButton::buttonWidth() const
{
    QFont f;
    f.setPixelSize(WizSmartScaleUI(RoundCellButtonConst::fontSize));
    QFontMetrics fm(f);
    int width = static_cast<int>(RoundCellButtonConst::margin * 2.5 + RoundCellButtonConst::spacing
            + iconWidth() + fm.width(text()));
    return width;
}

void WizRoundCellButton::setState(int state)
{
    WizCellButton::setState(state);

    applyAnimation();
}

void WizRoundCellButton::paintEvent(QPaintEvent* /*event*/)
{
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    QPainter p(this);

    QIcon::Mode mode = opt.state & QStyle::State_Enabled ? QIcon::Normal : QIcon::Disabled;
    if (mode == QIcon::Normal && (opt.state & QStyle::State_HasFocus || opt.state & QStyle::State_Sunken))
        mode = QIcon::Active;
    QIcon::State state = QIcon::Off;
    if (opt.state & QStyle::State_On)
        state = QIcon::On;

    p.setPen(Qt::NoPen);
    p.setBrush(QColor((mode & QIcon::Active) ? "#D3D3D3" : "#E6E6E6"));
    p.setRenderHint(QPainter::Antialiasing, true);
    p.drawRoundedRect(opt.rect, 8, 10);

    QSize size = m_iconSize;
    int nLeft = RoundCellButtonConst::margin;

    QRect rcIcon(nLeft, (opt.rect.height() - size.height()) / 2, iconWidth(), size.height());
    if (opt.icon.isNull())
    {
        m_iconNomal.paint(&p, rcIcon, Qt::AlignCenter, mode, state);
    }
    else
    {
        opt.icon.paint(&p, rcIcon, Qt::AlignCenter, mode, state);
    }

    QFont f = p.font();
    f.setPixelSize(WizSmartScaleUI(RoundCellButtonConst::fontSize));
    QFontMetrics fm(f);
    QRect rcText(rcIcon.right() + RoundCellButtonConst::spacing, (opt.rect.height() - fm.height()) / 2,
                 opt.rect.right() - rcIcon.right() - RoundCellButtonConst::spacing, fm.height());
    p.setPen(QColor("#535353"));
    p.setFont(f);
    p.drawText(rcText,Qt::AlignVCenter | Qt::AlignLeft, text());
}

QSize WizRoundCellButton::sizeHint() const
{
    //NTOE: 设置一个最大宽度，实际宽度由animation通过maxWidth进行控制
    int maxWidth = 200;
    return QSize(maxWidth, WizSmartScaleUI(RoundCellButtonConst::buttonHeight));
}

void WizRoundCellButton::applyAnimation()
{
    m_animation->stop();
    m_animation->setDuration(150);
    m_animation->setStartValue(maximumWidth());
    m_animation->setEndValue(buttonWidth());
    m_animation->setEasingCurve(QEasingCurve::InCubic);

    m_animation->start();
}


//-------------------------------------------------------------------
// Class WizToolButton implementation block.
//-------------------------------------------------------------------


WizToolButton::WizToolButton(QWidget* parent, int type)
    : QToolButton(parent)
    , m_buttonType(type)
    , m_state(Normal)
    , m_count(0)
    , m_iconSize(WizSmartScaleUI(14), WizSmartScaleUI(14)) //FIXME: cannot really scale iconsize.
{
    //setStyle(new WizNotePlusStyle("fusion"));
    setAutoRaise(true);
    setIcon(m_icon);

    switch (type) {

    case WithCountInfo:
    case WithTextLabel:
        setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        break;
    case ImageOnly:
    case ImageOnly | WithMenu:
        setToolButtonStyle(Qt::ToolButtonIconOnly);
        break;
    }
}

/**
 * @brief Set QIcon::off state icon pixmap.
 * @param icon
 * @param strTips
 */
void WizToolButton::setNormalIcon(const QIcon& icon, const QString& strTips)
{
    m_icon = icon;
    m_iconNomal = icon;
    m_strTipsNormal = strTips;

    setToolTip(strTips);
}

/**
 * @brief Set QIcon::on state icon pixmap.
 * @param icon
 * @param strTips
 */
void WizToolButton::setCheckedIcon(const QIcon& icon, const QString& strTips)
{
    QPixmap pm = icon.pixmap(m_iconSize, QIcon::Normal, QIcon::On);
    m_icon.addPixmap(pm, QIcon::Normal, QIcon::On);
    m_iconChecked = icon;
    m_strTipsChecked = strTips;
    
    setToolTip(strTips);
}

void WizToolButton::setBadgeIcon(const QIcon& icon, const QString& strTips)
{
    m_iconBadge = icon;
    m_strTipsBadge = strTips;

    setToolTip(strTips);
}

void WizToolButton::setState(int state)
{
    m_state = state;
}

void WizToolButton::setCount(int count)
{
    m_count = count;
    update();
}


QSize WizToolButton::sizeHint() const
{

    switch (m_buttonType) {

    case WithTextLabel:
    case ImageOnly:
        return QSize(28, 26);
    case WithCountInfo:
        return QSize(28 + nTextWidth, 26);
    case ImageOnly | WithMenu:
        return QSize(28 + 8, 26);
#ifdef Q_OS_WIN
    default:
        return QSize(28, 26);
#endif
    }

}

QString WizToolButton::countInfo() const
{
    if (m_count > 99)
        return "99+";
    return QString::number(m_count);
}

void WizToolButton::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);
    // 模拟一下菜单按钮下拉时不弹出菜单，而弹出自定义部件时，按钮的下层状态。
    QStylePainter p(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    if (m_buttonType == WithCountInfo)
    {
        opt.toolButtonStyle = Qt::ToolButtonTextBesideIcon;
        opt.text = countInfo();
        QColor textColor = m_count == 0 ? QColor("#A7A7A7") : QColor("#5990EF");
        opt.palette.setColor(QPalette::ButtonText, textColor);
    }
    //
    if (opt.icon.isNull())
        opt.icon = m_icon;
    //
    p.drawComplexControl(QStyle::CC_ToolButton, opt);
}

//TODO: 工具按钮除了能弹出菜单外，还要能弹出自定义窗口部件。
// QMenu 拥有aboutToHide()信号，参考QMenu来实现弹出部件的自动隐藏
// 然后用connect(actualMenu, SIGNAL(aboutToHide()), q, SLOT(_q_updateButtonDown()));来更新按钮状态

//-------------------------------------------------------------------
// Class WizToolButton implementation block.
//-------------------------------------------------------------------

//TODO: <保存/编辑> 可切换状态按钮
// 1. 按钮得checkable的。
// 2. 默认动作是打开内置内置编辑器的阅读和编辑模式
// 3. 能弹出外置编辑器选择菜单
// 4. 有一个较暗的背景可以标识此按钮
// 5. 状态转换之间的动画，比如群组笔记状态转换
// 6. 高度缩小，形状为药丸形

WizEditButton::WizEditButton(QWidget* parent)
    : WizToolButton(parent, WizToolButton::WithMenu)
{
    m_iconSize = QSize(14, 12);
    setMaximumWidth(0);
    setCheckable(true);
    //
    m_animation = new QPropertyAnimation(this, "maximumWidth", this);
}

QString WizEditButton::text() const
{
    switch (m_state) {

    case Normal:
        return m_textNormal;
    case Checked:
        return m_textChecked;
    case Badge:
        return m_textBadge;
    default:
        Q_ASSERT(0);
        break;
    }
    return "";
}

QString WizEditButton::tips() const
{
    switch (m_state) {

    case Normal:
        return m_strTipsNormal;
    case Checked:
        return m_strTipsChecked;
    case Badge:
        return m_strTipsBadge;
    default:
        Q_ASSERT(0);
        break;
    }
    return "";
}

void WizEditButton::setStatefulIcon(const QIcon& ico, WizToolButton::State state)
{

    switch (state) {

    case Normal:
    {
        QPixmap pm = ico.pixmap(m_iconSize, QIcon::Normal, QIcon::Off);
        QIcon i = icon();
        i.addPixmap(pm, QIcon::Normal, QIcon::Off);
        setIcon(i);
     }
        break;
    case Checked:
    {
        QPixmap pm = ico.pixmap(m_iconSize, QIcon::Normal, QIcon::On);
        QIcon i = icon();
        i.addPixmap(pm, QIcon::Normal, QIcon::On);
        setIcon(i);
    }
        break;
    case Badge:

        break;
    }
}

void WizEditButton::setStatefulText(const QString& text, const QString& strTips, WizToolButton::State state)
{
    switch (state) {

    case Normal:
        m_textNormal = text;
        m_strTipsNormal = strTips;
        break;
    case Checked:
        m_textChecked = text;
        m_strTipsChecked = strTips;
        break;
    case Badge:
        m_textBadge = text;
        m_strTipsBadge = strTips;
        break;
    }
}

/**
 * @brief Set button state programmatically.
 * @param state
 */
void WizEditButton::setState(WizToolButton::State state)
{
    switch (state) {
    case Normal:
        setChecked(false);
        setText(m_textNormal);
        setToolTip(m_strTipsNormal);
        m_state = Normal;
        break;
    case Checked:
        setChecked(true);
        setText(m_textChecked);
        setToolTip(m_strTipsChecked);
        m_state = Checked;
        break;
    case Badge:
        setText(m_textBadge);
        setToolTip(m_strTipsBadge);
        m_state = Badge;
        break;
    }

    applyAnimation();
}

int WizEditButton::iconWidth() const
{
    return 14;
}

int WizEditButton::buttonWidth() const
{
    QFont f;
    f.setPixelSize(WizSmartScaleUI(RoundCellButtonConst::fontSize));
    QFontMetrics fm(f);
    int width = static_cast<int>(RoundCellButtonConst::margin * 2.5 + RoundCellButtonConst::spacing
            + iconWidth() + fm.width(text()));
    return width;
}

void WizEditButton::paintEvent(QPaintEvent*)
{
    m_state = isChecked() ? Checked : Normal;
    setText(text());
    setToolTip(tips());
    QStylePainter p(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    opt.iconSize = m_iconSize;
    opt.toolButtonStyle = Qt::ToolButtonTextBesideIcon;
    //
    p.drawComplexControl(QStyle::CC_ToolButton, opt);
}

QSize WizEditButton::sizeHint() const
{
    //NTOE: 设置一个最大宽度，实际宽度由animation通过maxWidth进行控制
    int maxWidth = 200;
    return QSize(maxWidth, WizSmartScaleUI(RoundCellButtonConst::buttonHeight));
}

void WizEditButton::applyAnimation()
{
    m_animation->stop();
    m_animation->setDuration(150);
    m_animation->setStartValue(maximumWidth());
    m_animation->setEndValue(buttonWidth());
    m_animation->setEasingCurve(QEasingCurve::InCubic);

    m_animation->start();
}
