#include "WizUserInfoWidgetBase.h"

//#ifndef Q_OS_MAC

#include <QMenu>
#include <QFileDialog>
#include <QMessageBox>
#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>
#include "utils/WizStyleHelper.h"
#ifdef Q_OS_MAC
#include "mac/WizMacHelper.h"
#endif

WizUserInfoWidgetBase::WizUserInfoWidgetBase(QWidget *parent)
    : QToolButton(parent)
{
    setPopupMode(QToolButton::MenuButtonPopup);
}


void WizUserInfoWidgetBase::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    int nAvatarWidth = 26;
    int nArrawWidth = 10;
    int nMargin = 4;

    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    QPainter p(this);
    p.setClipRect(opt.rect);

    // draw user avatar
    QRect rectIcon = opt.rect;
    rectIcon.setLeft(rectIcon.left());
    rectIcon.setRight(rectIcon.left() + nAvatarWidth);
    rectIcon.setTop(rectIcon.top() + (rectIcon.height() - nAvatarWidth) / 2);
    rectIcon.setHeight(nAvatarWidth);

#ifdef Q_OS_MAC
    float factor = qt_mac_get_scalefactor(0);
    nAvatarWidth *= factor;
#endif

    QPixmap pixmap = getAvatar(nAvatarWidth, nAvatarWidth);
    Utils::WizStyleHelper::drawPixmapWithScreenScaleFactor(&p, rectIcon, pixmap);    

    // draw display name
    QRect rectText = rectIcon;
    rectText.setLeft(rectText.right() + nMargin);
    rectText.setRight(rectText.left() + fontMetrics().width(opt.text));
//    rectText.setBottom(rectText.top() + rectText.height()/2);
    if (!opt.text.isEmpty()) {
        if (opt.state & QStyle::State_MouseOver) {
            QFont font = p.font();
            font.setUnderline(true);
            p.setFont(font);
        }

        p.setPen("#787878"); // FIXME
        p.drawText(rectText, Qt::AlignLeft|Qt::AlignVCenter, opt.text);
    }

    // draw vip indicator
    QRect rectVip = rectText;
    QIcon iconVip = getVipIcon();
    QSize iconSize(16, 16);
    if (iconVip.availableSizes().size() != 0)
    {
        iconSize = iconVip.availableSizes().first();
    }
    rectVip.setLeft(rectVip.right() + nMargin);
    rectVip.setRight(rectVip.left() + iconSize.width());
//    rectVip.setBottom(rectVip.top() + rectVip.height()/2);
//    rectVip.setTop(rectVip.top() + (rectVip.height() - iconSize.height()) / 2);
    if (!iconVip.isNull()) {
        QPixmap pm = iconVip.pixmap(iconSize);
        style()->drawItemPixmap(&p, rectVip, Qt::AlignLeft|Qt::AlignVCenter, pm);
    }

    // draw arraw
    QRect rectArrow = rectVip;
    QIcon arrow = getArrow();
    if (!arrow.isNull()) {
        rectArrow.setLeft(rectArrow.right() + nMargin);
        rectArrow.setRight(rectArrow.left() + nArrawWidth);
        QPixmap pm = arrow.pixmap(10, 6);
        style()->drawItemPixmap(&p, rectArrow, Qt::AlignVCenter, pm);
    }
}

void WizUserInfoWidgetBase::mousePressEvent(QMouseEvent* event)
{
    // show menu at proper position
    if (hitButton(event->pos())) {
        //QPoint pos(event->pos().x(), sizeHint().height());
        // FIXME
        QPoint pos(32 + 4, 32 - fontMetrics().height() / 2);
        menu()->popup(mapToGlobal(pos), defaultAction());
    }
}

bool WizUserInfoWidgetBase::hitButton(const QPoint& pos) const
{
    // FIXME
    QRect rectArrow(32 + 8, 32 - fontMetrics().height() - 4, sizeHint().width() - 32 - 4, fontMetrics().height());
    return rectArrow.contains(pos) ? true : false;
}

int WizUserInfoWidgetBase::textWidth() const
{
    return fontMetrics().horizontalAdvance(text());
}

void WizUserInfoWidgetBase::updateUI()
{
    update();
}

//#endif //Q_OS_MAC

