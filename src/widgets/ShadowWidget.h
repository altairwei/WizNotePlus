#ifndef SHADOWWIDGET_H
#define SHADOWWIDGET_H

#include <QWidget>
#include <QPoint>

class ShadowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ShadowWidget(QWidget *parent = nullptr);

    QWidget *widget() { return m_widget; }
    void setTimeOut(unsigned int timeout) { m_timeout = timeout; }
    void resetTimer();
    void setMovable(bool movable) { m_movable = movable; }

protected:
    void showEvent(QShowEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QWidget *m_widget;
    int m_timerId;
    unsigned int m_timeout;
    bool m_movable;
    QPoint startPos;
};

#endif // SHADOWWIDGET_H
