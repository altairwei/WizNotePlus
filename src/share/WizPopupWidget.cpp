#include "WizPopupWidget.h"

#include <QtGui>
#include <QStyleOption>
#include <QStylePainter>

#include "WizMisc.h"
#include "utils/WizStyleHelper.h"

#ifdef Q_OS_WIN
//#include "WizWindowsHelper.h"
#endif


WizPopupWidget::WizPopupWidget(QWidget* parent)
    : QWidget(parent, Qt::Popup | Qt::NoDropShadowWindowHint | Qt::FramelessWindowHint)
    , m_leftAlign(false)
    , m_triangleMargin(10)
    , m_triangleWidth(16)
    , m_triangleHeight(8)
{
    setAttribute(Qt::WA_TranslucentBackground);

    setContentsMargins(8, 20, 8, 8);

    QPalette pal;
    pal.setBrush(backgroundRole(), QBrush("#FFFFFF"));
    setPalette(pal);
}

/*
QSize WizPopupWidget::sizeHint() const
{
    return QSize(320, 400);
}
*/

QRect WizPopupWidget::getClientRect() const
{
    QMargins margins = contentsMargins();
    return QRect(margins.left(), margins.top(),
                 width() - margins.left() - margins.right(),
                 height() - margins.top() - margins.bottom());
}

void WizPopupWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    //painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen(QColor(150, 150, 150), 1, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    painter.setBrush(palette().color(QPalette::Window));
    painter.drawPath(m_path);
}

void WizPopupWidget::showAtPoint(const QPoint& pos)
{
    bool showArrow = true;
    QScreen *screen = QGuiApplication::screenAt(pos);
    if (!screen) return;
    QRect scr = screen->geometry();
    QSize sh = sizeHint();
    const int border = 1;
    const int arrowHeight = 8, arrowOffset = 18, arrowWidth = 16, borderRadius = 0;

    // Determine arrow position
    bool arrowAtTop = (pos.y() + sh.height() + arrowHeight < scr.height());
    bool arrowAtLeft = (pos.x() + sh.width() - arrowOffset < scr.width());

    // Resize content margin
    setContentsMargins(
        border + 1,
        border + (arrowAtTop ? arrowHeight : 0) + 1,
        border + 1,
        border + (arrowAtTop ? 0 : arrowHeight) + 1);
    updateGeometry();
    sh  = sizeHint();

    // Determine position of widget rect
    int marginLeft, marginRight, marginTop, marginBottom;
    QSize sz = sizeHint();
    if (!arrowAtTop) {
        marginLeft = 1;
        marginRight = sz.width() - 1;
        marginTop = 1;
        marginBottom = sz.height() - arrowHeight - 1;
    } else {
        marginLeft = 1;
        marginRight = sz.width() - 1;
        marginTop = arrowHeight + 1;
        marginBottom = sz.height() - 1;
    }

    // Draw border of whole widget including arrow
    QPainterPath path;
    path.moveTo(marginLeft + borderRadius, marginTop);
    if (arrowAtTop && arrowAtLeft) {
        if (showArrow) {
            path.lineTo(marginLeft + arrowOffset, marginTop);
            path.lineTo(marginLeft + arrowOffset + arrowWidth/2, marginTop - arrowHeight);
            path.lineTo(marginLeft + arrowOffset + arrowWidth, marginTop);
        }
        move(qMax(pos.x() - arrowOffset - arrowWidth/2, scr.left() + 2), pos.y());
    } else if (arrowAtTop && !arrowAtLeft) {
        if (showArrow) {
            path.lineTo(marginRight - arrowOffset - arrowWidth, marginTop);
            path.lineTo(marginRight - arrowOffset - arrowWidth/2, marginTop - arrowHeight);
            path.lineTo(marginRight - arrowOffset, marginTop);
        }
        move(qMin(pos.x() - sh.width() + arrowOffset + arrowWidth/2, scr.right() - sh.width() - 2), pos.y());
    }
    path.lineTo(marginRight - borderRadius, marginTop);
    path.arcTo(QRect(marginRight - borderRadius*2, marginTop, borderRadius*2, borderRadius*2), 90, -90);
    path.lineTo(marginRight, marginBottom - borderRadius);
    path.arcTo(QRect(marginRight - borderRadius*2, marginBottom - borderRadius*2, borderRadius*2, borderRadius*2), 0, -90);
    if (!arrowAtTop && !arrowAtLeft) {
        if (showArrow) {
            path.lineTo(marginRight - arrowOffset, marginBottom);
            path.lineTo(marginRight - arrowOffset - arrowWidth/2, marginBottom + arrowHeight);
            path.lineTo(marginRight - arrowOffset - arrowWidth, marginBottom);
        }
        move(qMin(pos.x() - sh.width() + arrowOffset + arrowWidth/2, scr.right() - sh.width() - 2),
             pos.y() - sh.height());
    } else if (!arrowAtTop && arrowAtLeft) {
        if (showArrow) {
            path.lineTo(arrowOffset + arrowWidth, marginBottom);
            path.lineTo(arrowOffset + arrowWidth/2, marginBottom + arrowHeight);
            path.lineTo(arrowOffset, marginBottom);
        }
        move(qMax(pos.x() - arrowOffset - arrowWidth/2, scr.x() + 2), pos.y() - sh.height());
    }
    path.lineTo(marginLeft + borderRadius, marginBottom);
    path.arcTo(QRect(marginLeft, marginBottom - borderRadius*2, borderRadius*2, borderRadius*2), -90, -90);
    path.lineTo(marginLeft, marginTop + borderRadius);
    path.arcTo(QRect(marginLeft, marginTop, borderRadius*2, borderRadius*2), 180, -90);

    m_path = path;

    // Set the mask
    QBitmap bitmap = QBitmap(sizeHint());
    bitmap.fill(Qt::color0);
    QPainter painter1(&bitmap);
    painter1.setPen(QPen(Qt::color1, border));
    painter1.setBrush(QBrush(Qt::color1));
    painter1.drawPath(path);
    //setMask(bitmap);

    // Draw the border
    m_pixmap = QPixmap(sz);
    QPainter painter2(&m_pixmap);
    painter2.setPen(QPen(palette().color(QPalette::Window).darker(160), border));
    painter2.setBrush(palette().color(QPalette::Window));
    painter2.drawPath(path);

    show();
}

void WizPopupWidget::setTriangleStyle(int triangleMargin, int triangleWidth, int triangleHeight)
{
    m_triangleMargin = triangleMargin;
    m_triangleWidth = triangleWidth;
    m_triangleHeight = triangleHeight;
}

QRegion WizPopupWidget::maskRegion()
{

    return Utils::WizStyleHelper::borderRadiusRegionWithTriangle(QRect(0, 0, size().width(), size().height()), m_leftAlign,
                                                       m_triangleMargin, m_triangleWidth, m_triangleHeight);
}