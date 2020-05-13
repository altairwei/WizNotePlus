#include "FullScreenNotification.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

FullScreenNotification::FullScreenNotification(QWidget *parent)
    : QLabel(parent)
    , m_previouslyVisible(false)
{
    setText(tr("Press ESC to quit full screen"));
    setStyleSheet(
        "font-size: 18px;"
        "color: white;"
        "background-color: black;"
        "padding: 10px");
    setAttribute(Qt::WA_TransparentForMouseEvents);

    auto effect = new QGraphicsOpacityEffect;
    effect->setOpacity(1);
    setGraphicsEffect(effect);

    auto animations = new QSequentialAnimationGroup(this);
    animations->addPause(3000);
    auto opacityAnimation = new QPropertyAnimation(effect, "opacity", animations);
    opacityAnimation->setDuration(2000);
    opacityAnimation->setStartValue(1.0);
    opacityAnimation->setEndValue(0.0);
    opacityAnimation->setEasingCurve(QEasingCurve::OutQuad);
    animations->addAnimation(opacityAnimation);

    connect(this, &FullScreenNotification::shown,
            [animations](){ animations->start(); });

    connect(animations, &QAbstractAnimation::finished,
            [this](){ this->hide(); });
}

void FullScreenNotification::showEvent(QShowEvent *event)
{
    QLabel::showEvent(event);
    if (!m_previouslyVisible && isVisible())
        emit shown();
    m_previouslyVisible = isVisible();
}
