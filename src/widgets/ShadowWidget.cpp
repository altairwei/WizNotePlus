#include "ShadowWidget.h"

#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QTimerEvent>
#include <QShowEvent>
#include <QDebug>
#include <QDrag>

ShadowWidget::ShadowWidget(QWidget *parent)
    : QWidget(parent)
    , m_widget(new QWidget)
    , m_shadowEffect(new QGraphicsDropShadowEffect)
    , m_timerId(-1)
    , m_timeout(0)
    , m_movable(false)
{
    auto layout = new QVBoxLayout;
    m_widget->setObjectName("innerWidget");
    layout->addWidget(m_widget);
    layout->setContentsMargins(14, 14, 14, 14);
    setLayout(layout);

    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);

    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_DeleteOnClose);

    // Add the corresponding shadow effect to the inner window
    m_shadowEffect->setOffset(0, 2);
    m_shadowEffect->setColor(QColor(150, 150, 150));
    m_shadowEffect->setBlurRadius(14);
    m_widget->setGraphicsEffect(m_shadowEffect);
}

void ShadowWidget::showEvent(QShowEvent *event)
{
    if (m_timeout > 0)
        m_timerId = startTimer(m_timeout);
    QWidget::showEvent(event);
}

void ShadowWidget::enterEvent(QEvent *event)
{
    clearTimer();
    QWidget::enterEvent(event);
}

void ShadowWidget::resetTimer()
{
    if (m_timeout > 0 && m_timerId != -1) {
        killTimer(m_timerId);
        m_timerId = startTimer(m_timeout);
    }
}

void ShadowWidget::clearTimer()
{
    if (m_timeout > 0 && m_timerId != -1) {
        killTimer(m_timerId);
        m_timerId = -1;
    }
}

void ShadowWidget::leaveEvent(QEvent *event)
{
    if (m_timeout > 0 && m_timerId == -1)
        m_timerId = startTimer(m_timeout);
    QWidget::leaveEvent(event);
}

void ShadowWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timerId) {
        killTimer(m_timerId);
        m_timerId = -1;
        if (!underMouse())
            close();
        return;
    }

    QWidget::timerEvent(event);
}

void ShadowWidget::mousePressEvent(QMouseEvent *event)
{
    if (m_movable)
        startPos = event->globalPos();
    QWidget::mousePressEvent(event);
}

void ShadowWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_movable) {
        const QPoint delta = event->globalPos() - startPos;
        move(x()+delta.x(), y()+delta.y());
        startPos = event->globalPos();
    }

    QWidget::mouseMoveEvent(event);
}

void ShadowWidget::setPopup(bool pop)
{
    if (pop)
        setWindowFlags(
            Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint
                    | Qt::Popup | Qt::WindowStaysOnTopHint);
    else
        setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
}
