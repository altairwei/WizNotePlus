#ifndef GUI_TABBROWSER_TABBUTTON_H
#define GUI_TABBROWSER_TABBUTTON_H

#include <QAbstractButton>

class QStyleOptionButton;
class QStyleOptionTabBarBase;

class TabButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit TabButton(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override
        { return sizeHint(); }
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void drawTabBtn(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget = nullptr) const;
};

#endif // GUI_TABBROWSER_TABBUTTON_H