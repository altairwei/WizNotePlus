#ifndef WIZPOPUPWIDGET_H
#define WIZPOPUPWIDGET_H

#include <QWidget>

class QTimer;

class WizPopupWidget : public QWidget
{
    Q_OBJECT

public:
    WizPopupWidget(QWidget* parent);

    virtual QSize sizeHint() const;
    virtual QRect getClientRect() const;

private:
    QVector<QPoint> m_pointsRegion;
    bool m_leftAlign;
    QPoint m_pos;
    QPixmap m_pixmap;

    QRegion maskRegion();

protected:
    int m_triangleMargin;
    int m_triangleWidth;
    int m_triangleHeight;


protected:
    virtual void paintEvent(QPaintEvent* event);

public:
    void setLeftAlign(bool b) { m_leftAlign = b; }
    void showAtPoint(const QPoint& pt);
    void setTriangleStyle(int triangleMargin, int triangleWidth, int triangleHeight);
};

#endif // WIZPOPUPWIDGET_H
