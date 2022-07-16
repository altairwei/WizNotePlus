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
    , m_timerId(-1)
    , m_timeout(0)
    , m_movable(false)
{
    auto layout = new QVBoxLayout;
    m_widget->setObjectName("innerWidget");
    layout->addWidget(m_widget);
    layout->setContentsMargins(14, 14, 14, 14);
    setLayout(layout);

    // Set the window as borderless and displayed on the top layer
    // Don't show icons on the taskbar
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);

    // If you want to make it work like a popup, you should use these flags.
    //setWindowFlags(Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::Popup | Qt::WindowStaysOnTopHint)

    // The outer window appears transparent
    setAttribute(Qt::WA_TranslucentBackground, true);

    // This is not necessary for rounded shadow
    setAttribute(Qt::WA_DeleteOnClose);

    // Add the corresponding shadow effect to the inner window
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(0, 2);
    shadow_effect->setColor(QColor(150, 150, 150));
    shadow_effect->setBlurRadius(14);
    m_widget->setGraphicsEffect(shadow_effect);

    // We manage style sheet outside C++ code.
    //m_widget->setStyleSheet("border:1px solid #FFFFFF;border-radius:7px;background-color:#FFFFFF;");
}

void ShadowWidget::showEvent(QShowEvent *event)
{
    if (m_timeout > 0)
        m_timerId = startTimer(m_timeout);
    QWidget::showEvent(event);
}

void ShadowWidget::enterEvent(QEvent *event)
{
    if (m_timeout > 0 && m_timerId != -1) {
        killTimer(m_timerId);
        m_timerId = -1;
    }

    QWidget::enterEvent(event);
}

void ShadowWidget::resetTimer()
{
    if (m_timeout > 0 && m_timerId != -1) {
        killTimer(m_timerId);
        m_timerId = startTimer(m_timeout);
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
