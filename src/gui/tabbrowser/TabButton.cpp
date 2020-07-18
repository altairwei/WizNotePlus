#include "TabButton.h"

#include <QStyle>
#include <QStylePainter>
#include <QStyleOption>
#include <QStyleOptionTabBarBase>

TabButton::TabButton(QWidget *parent)
    : QAbstractButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::ArrowCursor);
    resize(sizeHint());
    setIconSize(QSize(16, 16));
}

QSize TabButton::sizeHint() const
{
    ensurePolished();
    int width = style()->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, 0, this);
    int height = style()->pixelMetric(QStyle::PM_TabCloseIndicatorHeight, 0, this);
    return QSize(width, height);
}

void TabButton::enterEvent(QEvent *event)
{
    if (isEnabled())
        update();
    QAbstractButton::enterEvent(event);
}

void TabButton::leaveEvent(QEvent *event)
{
    if (isEnabled())
        update();
    QAbstractButton::leaveEvent(event);
}

void TabButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOptionButton opt;
    opt.init(this);
    opt.state |= QStyle::State_AutoRaise;
    if (isEnabled() && underMouse() && !isChecked() && !isDown())
        opt.state |= QStyle::State_Raised;
    if (isChecked())
        opt.state |= QStyle::State_On;
    if (isDown())
        opt.state |= QStyle::State_Sunken;

    if (const QTabBar *tb = qobject_cast<const QTabBar *>(parent())) {
        int index = tb->currentIndex();
        QTabBar::ButtonPosition position = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, tb);
        if (tb->tabButton(index, position) == this)
            opt.state |= QStyle::State_Selected;
    }
    opt.icon = icon();
    opt.iconSize = QSize(16, 16);
    //style()->drawPrimitive(QStyle::PE_IndicatorTabClose, &opt, &p, this);
    drawTabBtn(&opt, &p, this);
}

void TabButton::drawTabBtn(const QStyleOptionButton *opt, QPainter *p, const QWidget *widget) const
{

    /* 应该添加下面几种状态
    if (d->tabBarcloseButtonIcon.isNull()) {
        d->tabBarcloseButtonIcon.addPixmap(QPixmap(
                    QLatin1String(":/qt-project.org/styles/commonstyle/images/standardbutton-closetab-16.png")),
                    QIcon::Normal, QIcon::Off);
        d->tabBarcloseButtonIcon.addPixmap(QPixmap(
                    QLatin1String(":/qt-project.org/styles/commonstyle/images/standardbutton-closetab-down-16.png")),
                    QIcon::Normal, QIcon::On);
        d->tabBarcloseButtonIcon.addPixmap(QPixmap(
                    QLatin1String(":/qt-project.org/styles/commonstyle/images/standardbutton-closetab-hover-16.png")),
                    QIcon::Active, QIcon::Off);
    }
    */
    //int size = style()->pixelMetric(QStyle::PM_SmallIconSize);
    QIcon::Mode mode = opt->state & QStyle::State_Enabled ?
                        (opt->state & QStyle::State_Raised ? QIcon::Active : QIcon::Normal)
                        : QIcon::Disabled;
    if (!(opt->state & QStyle::State_Raised)
        && !(opt->state & QStyle::State_Sunken)
        && !(opt->state & QStyle::State_Selected))
        mode = QIcon::Disabled;
    //

    QIcon::State state = opt->state & QStyle::State_Sunken ? QIcon::On : QIcon::Off;
    QPixmap pixmap = opt->icon.pixmap(opt->iconSize, mode, state);
    style()->drawItemPixmap(p, opt->rect, Qt::AlignCenter, pixmap);
}