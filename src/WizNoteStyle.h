#ifndef WIZNOTESTYLE_H
#define WIZNOTESTYLE_H

#include <QString>
#include <QStyle>
#include <QProxyStyle>
#include <QListWidget>
#include <QListWidgetItem>

QStyle* WizGetStyle(const QString& skinName);

QStyle* WizGetImageButtonStyle(const QString& normalBackgroundFileName, const QString& hotBackgroundFileName,
                               const QString& downBackgroundFileName, const QString& disabledBackgroundFileName,
                               const QColor& normalTextColor, const QColor& activeTextColor, const QColor& disableTextColor);



template <typename T>
class WizListItemStyle : public QProxyStyle
{
public:
    WizListItemStyle(QStyle *style = nullptr) : QProxyStyle(style) {}
    virtual void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
    {
        switch (element)
        {
        case CE_ItemViewItem:
            {
                const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(option);
                Q_ASSERT(vopt);

                if (const QListWidget* view = dynamic_cast<const QListWidget*>(widget))
                {
                    QListWidgetItem *item = view->item(vopt->index.row());
                    if (item)
                    {
                        if (T* templateItem = dynamic_cast<T*>(item))
                        {
                            templateItem->draw(painter, vopt);
                        }
                    }
                } else {
                    QProxyStyle::drawControl(element, option, painter, widget);
                }

                break;
            }
        default:
            QProxyStyle::drawControl(element, option, painter, widget);
            break;
        }
    }
};

enum Direction {
    TopDown,
    FromLeft,
    BottomUp,
    FromRight
};

class WizNotePlusStyle : public QProxyStyle
{
    Q_OBJECT

public:
    explicit WizNotePlusStyle(QString styleName);
    static void drawArrow(const QStyle *style, const QStyleOptionToolButton *toolbutton,
                          const QRect &rect, QPainter *painter, const QWidget *widget = nullptr);
    static QWindow *qt_getWindow(const QWidget *widget);
    static QColor mergedColors(const QColor &colorA, const QColor &colorB, int factor = 50);
    static QLinearGradient qt_fusion_gradient(const QRect &rect, const QBrush &baseColor, Direction direction = TopDown);

    QColor outline(const QPalette &pal) const;

    QColor buttonColor(const QPalette &pal) const;
    QColor tabFrameColor(const QPalette &pal) const;
    QColor innerContrastLine() const;

    void drawControl(ControlElement element, const QStyleOption *opt, QPainter *p,
                                                const QWidget *w = nullptr) const override;
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                          QPainter *p, const QWidget *widget) const override;

};

#endif // WIZNOTESTYLE_H
