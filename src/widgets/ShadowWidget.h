#ifndef SHADOWWIDGET_H
#define SHADOWWIDGET_H

#include <QWidget>
#include <QPoint>

class QGraphicsDropShadowEffect;

class ShadowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ShadowWidget(QWidget *parent = nullptr);

    QWidget *widget() { return m_widget; }
    QGraphicsDropShadowEffect *shadowEffect() { return m_shadowEffect; }
    void setTimeOut(unsigned int timeout) { m_timeout = timeout; }
    void resetTimer();
    void clearTimer();
    void setMovable(bool movable) { m_movable = movable; }
    void setPopup(bool pop);

protected:
    void showEvent(QShowEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QWidget *m_widget;
    QGraphicsDropShadowEffect *m_shadowEffect;
    int m_timerId;
    unsigned int m_timeout;
    bool m_movable;
    QPoint startPos;
};

#endif // SHADOWWIDGET_H
