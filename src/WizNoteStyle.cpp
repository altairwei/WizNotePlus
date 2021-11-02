#include "WizNoteStyle.h"

#include <QProxyStyle>
#include <QCommonStyle>
#include <QPainter>
#include <QScrollBar>
#include <QApplication>
#include <QRect>

#include "gui/categoryviewer/WizCategoryView.h"
#include "gui/doclistviewer/WizDocumentListView.h"
#include "gui/doclistviewer/WizDocumentListViewItem.h"
#include "WizAttachmentListWidget.h"
#include "share/WizDrawTextHelper.h"
#include "share/WizQtHelper.h"
#include "share/WizSettings.h"
#include "share/WizUIHelper.h"
#include "share/WizUI.h"
#include "share/WizMultiLineListWidget.h"
#include "share/WizMisc.h"
//#include "share/wizimagepushbutton.h"

#include "WizMessageListView.h"

#include "utils/WizStyleHelper.h"
#include "sync/WizAvatarHost.h"

#ifdef Q_OS_MAC
#include "mac/WizMacHelper.h"
#endif


class WizCategoryBaseView;
class WizDocumentListView;


typedef QProxyStyle CWizNoteBaseStyle;

class CWizNoteStyle : public CWizNoteBaseStyle
{
public:
    CWizNoteStyle(const QString& skinName);

private:
    QImage m_expandedImage;
    QImage m_collapsedImage;
    QImage m_expandedImageSelected;
    QImage m_collapsedImageSelected;
    QImage m_imgDefaultAvatar;

//    CWizSkin9GridImage m_multiLineListSelectedItemBackground;
//    CWizSkin9GridImage m_multiLineListSelectedItemBackgroundHot;
    WizSkin9GridImage m_imagePushButton;
    WizSkin9GridImage m_imagePushButtonHot;
    WizSkin9GridImage m_imagePushButtonPressed;
    WizSkin9GridImage m_imagePushButtonDisabled;
    WizSkin9GridImage m_imagePushButtonLabel;
    WizSkin9GridImage m_imagePushButtonLabelRed;

    QFont m_fontImagePushButtonLabel;
    QFont m_fontLink;

protected:
    virtual void drawCategoryViewItem(const QStyleOptionViewItem *option, QPainter *painter, const WizCategoryBaseView *widget) const;
    virtual void drawMultiLineListWidgetItem(const QStyleOptionViewItem *option, QPainter *painter, const WizMultiLineListWidget *widget) const;
    virtual void drawMultiLineItemBackground(const QStyleOptionViewItem* vopt, QPainter* pt, const WizMultiLineListWidget* view) const;
    void drawcenterImage(QPainter* p, const QImage& image, const QRect& rc) const;

public:
    virtual void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const;
    virtual void drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p, const QWidget *w = 0) const;
    virtual int	pixelMetric(PixelMetric metric, const QStyleOption* option = 0, const QWidget* widget = 0 ) const;

public:
    static CWizNoteStyle* noteStyle(const QString& skinName);
};


const int IMAGE_WIDTH = 80;

CWizNoteStyle::CWizNoteStyle(const QString& strSkinName)
{
    if (!strSkinName.isEmpty())
    {
        QString strSkinPath = ::WizGetSkinResourcePath(strSkinName);
        bool bHightPixel = WizIsHighPixel();
        QString strIconName = bHightPixel ? "branch_expanded@2x.png" : "branch_expanded.png";
        m_expandedImage.load(strSkinPath + strIconName);
        strIconName = bHightPixel ? "branch_collapsed@2x.png" : "branch_collapsed.png";
        m_collapsedImage.load(strSkinPath + strIconName);
        strIconName = bHightPixel ? "branch_expandedSelected@2x.png" : "branch_expandedSelected.png";
        m_expandedImageSelected.load(strSkinPath + strIconName);
        strIconName = bHightPixel ? "branch_collapsedSelected@2x.png" : "branch_collapsedSelected.png";
        m_collapsedImageSelected.load(strSkinPath + strIconName);
        m_imgDefaultAvatar.load(strSkinPath + "avatar_default.png");

        //m_iconDocumentsBadge = ::WizLoadSkinIcon(strSkinName, "document_badge");
        //m_iconDocumentsBadgeEncrypted = ::WizLoadSkinIcon(strSkinName, "document_badge_encrypted");

//        m_multiLineListSelectedItemBackground.SetImage(strSkinPath + "multilinelist_selected_background.png", QPoint(4, 4));
//        m_multiLineListSelectedItemBackgroundHot.SetImage(strSkinPath + "multilinelist_selected_background_hot.png", QPoint(4, 4));
        m_imagePushButton.setImage(strSkinPath + "imagepushbutton.png", QPoint(4, 4));
        m_imagePushButtonHot.setImage(strSkinPath + "imagepushbutton_hot.png", QPoint(4, 4));
        m_imagePushButtonPressed.setImage(strSkinPath + "imagepushbutton_pressed.png", QPoint(4, 4));
        m_imagePushButtonDisabled.setImage(strSkinPath + "imagepushbutton_disabled.png", QPoint(4, 4));
        m_imagePushButtonLabel.setImage(strSkinPath + "imagepushbutton_label.png", QPoint(8, 8));
        m_imagePushButtonLabelRed.setImage(strSkinPath + "imagepushbutton_label_red.png", QPoint(8, 8));
    }

#ifdef Q_OS_MAC
    m_fontImagePushButtonLabel = QFont("Arial Black", 9);
#else
    m_fontImagePushButtonLabel = QFont("Arial Black", 8);
#endif
    m_fontImagePushButtonLabel.setBold(true);
    //
    m_fontLink.setItalic(true);
    //m_fontLink.setUnderline(true);
    m_fontLink.setPixelSize(m_fontLink.pixelSize() - 4);
}



void CWizNoteStyle::drawCategoryViewItem(const QStyleOptionViewItem *vopt,
                                         QPainter *p, const WizCategoryBaseView *view) const
{
    if (view->isDragHovered() && view->validateDropDestination(view->dragHoveredPos())) {
        QRect rect = view->visualItemRect(view->itemAt(view->dragHoveredPos()));
        p->setRenderHint(QPainter::Antialiasing, true);
        QPen pen;
        pen.setStyle(Qt::SolidLine);
//        pen.setCapStyle(Qt::RoundCap);
        pen.setColor(QColor("#3498DB"));
        pen.setWidth(1);
        p->setPen(pen);
        p->setBrush(Qt::NoBrush);
        rect.setWidth(rect.width() - 2);
        p->drawRect(rect);
    }

    p->save();
    view->categoryItemFromIndex(vopt->index)->drawItemBody(p, vopt);
    view->categoryItemFromIndex(vopt->index)->drawExtraBadge(p, vopt);
    p->restore();

}

void CWizNoteStyle::drawMultiLineListWidgetItem(const QStyleOptionViewItem *vopt, QPainter *p, const WizMultiLineListWidget *view) const
{
    bool imageAlignLeft = view->imageAlignLeft();
    int imageWidth = view->imageWidth();
    int lineCount = view->lineCount();
    int wrapTextLineText = view->wrapTextLineIndex();
    const QPixmap img = view->itemImage(vopt->index);

    p->save();
    p->setClipRect(vopt->rect);

    QRect textLine = vopt->rect;
    textLine.adjust(14, 0, 0, 0);
    p->setPen(Utils::WizStyleHelper::listViewItemSeperator());
    p->drawLine(textLine.bottomLeft(), textLine.bottomRight());

    QRect textRect = vopt->rect;
    //QRect textRect = subElementRect(SE_ItemViewItemText, vopt, view);

    // draw the background
    drawMultiLineItemBackground(vopt, p, view);

    if (!img.isNull() && img.width() > 0 && img.height() > 0)
    {
        QRect imageRect = textRect;
        if (imageAlignLeft)
        {
            imageRect.setRight(imageRect.left() + imageWidth + 14);
            textRect.setLeft(imageRect.right() + 12);
            imageRect.setRight(imageRect.right() + 14);
        }
        else
        {
            imageRect.setLeft(imageRect.right() - imageWidth - 14);
            textRect.setRight(imageRect.left() - 12);
            imageRect.setLeft(imageRect.left() - 12);
        }

//        int imgWidth = WizIsHighPixel() ? img.width() / 2 : img.width();
//        int imgHeight = WizIsHighPixel() ? img.height() / 2 : img.height();
//        if (imgWidth > imageRect.width() || imgHeight > imageRect.height())
//        {
//            double fRate = std::min<double>(double(imageRect.width()) / imgWidth, double(imageRect.height()) / imgHeight);
//            int newWidth = int(imgWidth * fRate);
//            int newHeight = int(imgHeight * fRate);
//            //
//            int adjustX = (imageRect.width() - newWidth) / 2;
//            int adjustY = (imageRect.height() - newHeight) / 2;
//            imageRect.adjust(adjustX, adjustY, -adjustX, -adjustY);
//        }
//        else
//        {
            int adjustX = (imageRect.width() - imageWidth) / 2;
            int adjustY = (imageRect.height() - imageWidth) / 2;
            imageRect.adjust(adjustX, adjustY, -adjustX, -adjustY);
//        }
        p->drawPixmap(imageRect, img);
    }

    // draw the text
    QPalette::ColorGroup cg = vopt->state & QStyle::State_Enabled
                              ? QPalette::Normal : QPalette::Disabled;
    if (cg == QPalette::Normal && !(vopt->state & QStyle::State_Active))
        cg = QPalette::Inactive;

    if (vopt->state & QStyle::State_Selected) {
        p->setPen(vopt->palette.color(cg, QPalette::HighlightedText));
    } else {
        p->setPen(vopt->palette.color(cg, QPalette::Text));
    }
    if (vopt->state & QStyle::State_Editing) {
        p->setPen(vopt->palette.color(cg, QPalette::Text));
        p->drawRect(textRect.adjusted(0, 0, -1, -1));
    }

    QFont font = p->font();
    font.setPixelSize(12);
    p->setFont(font);
    QFontMetrics fm(font);

    textRect.adjust(0, 3, -8, -8);
    bool selected = vopt->state.testFlag(State_Selected);
    int lineHeight = fm.height() + 2;

    QColor color("#535353");
    for (int line = 0; line < wrapTextLineText && line < lineCount; line++)
    {        
        CString strText = view->itemText(vopt->index, line);
        color = view->itemTextColor(vopt->index, line, selected, color);
        QRect rc = textRect;
        rc.setTop(rc.top() + line * lineHeight);
        rc.setHeight(lineHeight);
        rc.setWidth(190);
        ::WizDrawTextSingleLine(p, rc, strText,  Qt::TextSingleLine | Qt::AlignVCenter, color, true);
    }

    int line = wrapTextLineText;
    if (line < lineCount)
    {
        CString strText = view->itemText(vopt->index, line);
        for (; line < lineCount; line++)
        {            
            QRect rc = textRect;
            rc.setTop(rc.top() - 1 + line * lineHeight);
            rc.setHeight(lineHeight);
            bool elidedText = (line == lineCount - 1);
            ::WizDrawTextSingleLine(p, rc, strText,  Qt::TextSingleLine | Qt::AlignVCenter, color, elidedText);
        }
    }

//    // draw the focus rect
//    if (vopt->state & QStyle::State_HasFocus) {
//        QStyleOptionFocusRect o;
//        o.QStyleOption::operator=(*vopt);
//        o.rect = subElementRect(SE_ItemViewItemFocusRect, vopt, view);
//        o.state |= QStyle::State_KeyboardFocusChange;
//        o.state |= QStyle::State_Item;
//        QPalette::ColorGroup cg = (vopt->state & QStyle::State_Enabled)
//                                  ? QPalette::Normal : QPalette::Disabled;
//        o.backgroundColor = vopt->palette.color(cg, (vopt->state & QStyle::State_Selected)
//                                                ? QPalette::Highlight : QPalette::Window);
//        proxy()->drawPrimitive(QStyle::PE_FrameFocusRect, &o, p, view);
//    }

    //draw extra image
    QRect rcExtra;
    QPixmap pixExtra;
    if (view->itemExtraImage(vopt->index, vopt->rect.adjusted(0, 0, 0, -1), rcExtra, pixExtra))
    {
//        QScrollBar* scrollBar = view->verticalScrollBar();
//        if (scrollBar && scrollBar->isVisible())
//        {
//            int nMargin = -1;
//            rcExtra.adjust(nMargin, 0, nMargin, 0);
//        }

        p->drawPixmap(rcExtra, pixExtra);
    }

    p->restore();
}

void CWizNoteStyle::drawMultiLineItemBackground(const QStyleOptionViewItem* vopt, QPainter* pt, const WizMultiLineListWidget* view) const
{
    if (const WizAttachmentListView *attachView = dynamic_cast<const WizAttachmentListView *>(view))
    {
        const WizAttachmentListViewItem* item = attachView->attachmentItemFromIndex(vopt->index);
        if (item && (item->isDownloading() || item->isUploading()))
        {
            pt->save();
            pt->setPen(Qt::NoPen);
            pt->setBrush(QColor("#5990EF"));
            QRect rect = vopt->rect;
            rect.setWidth(rect.width() * item->loadProgress() / 100);
            pt->drawRect(rect);
            pt->restore();

            return;
        }
    }

    if (vopt->state.testFlag(State_Selected))
    {
//        m_multiLineListSelectedItemBackground.Draw(pt, vopt->rect, 0);
        pt->save();

        QPen pen(QColor("#3177EE"));
//        pen.setWidth(2);
        pt->setPen(pen);
        pt->setBrush(Qt::NoBrush);
        pt->drawRect(vopt->rect.adjusted(1, 1, -1, -2));

        pt->restore();
    }
}

void CWizNoteStyle::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    switch (element)
    {
    case CE_ItemViewItem:
        {
            const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(option);
            Q_ASSERT(vopt);

            if (const WizMessageListView* view = dynamic_cast<const WizMessageListView*>(widget))
            {
                view->drawItem(painter, vopt);
                //drawMessageListViewItem(vopt, painter, view);
            }
            else if (const WizDocumentListView *view = dynamic_cast<const WizDocumentListView *>(widget))
            {
//                qDebug() << "view left top : " << view->mapToGlobal(view->rect().topLeft());
                view->drawItem(painter, vopt);
                //drawDocumentListViewItem(vopt, painter, view);
            }
            else if (const WizMultiLineListWidget *view = dynamic_cast<const WizMultiLineListWidget *>(widget))
            {
                drawMultiLineListWidgetItem(vopt, painter, view);
            }
            else if (const WizCategoryBaseView *view = dynamic_cast<const WizCategoryBaseView *>(widget))
            {
                drawCategoryViewItem(vopt, painter, view);
            }
            else
            {
                CWizNoteBaseStyle::drawControl(element, option, painter, widget);
            }
            break;
        }
    //case CE_PushButton:
    //    {
    //        const QStyleOptionButton* vopt = qstyleoption_cast<const QStyleOptionButton *>(option);
    //        ATLASSERT(vopt);
    //        //
    //        if (const CWizImagePushButton *button = dynamic_cast<const CWizImagePushButton *>(widget))
    //        {
    //            drawImagePushButton(vopt, painter, button);
    //        }
    //        else
    //        {
    //            CWizNoteBaseStyle::drawControl(element, option, painter, widget);
    //        }
    //    }
    //    break;
    //case CE_Splitter:
    //{
    //    if (const QSplitter* splitter = dynamic_cast<const QSplitter *>(widget))
    //    {
    //        drawSplitter(option, painter, splitter);
    //    } else {
    //        CWizNoteBaseStyle::drawControl(element, option, painter, widget);
    //    }
    //}

    default:
        CWizNoteBaseStyle::drawControl(element, option, painter, widget);
        break;
    }
}

void CWizNoteStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                                  const QWidget *w) const
{
    switch (pe)
    {
    case PE_IndicatorBranch:
        // 目录树指示箭头绘制
        {
            if (const WizCategoryBaseView *view = dynamic_cast<const WizCategoryBaseView *>(w))
            {
                if (!view->isCursorEntered())
                    return;

                if (opt->state & QStyle::State_Children) {
                    bool bExpanded = (opt->state & QStyle::State_Open) ? true : false;
                    // 改变图标尺寸和位置
                    QRect rectIcon = opt->rect;
                    // 调整 brach 图标偏移量
                    rectIcon.adjust(0, 0, 0, 0);
                    //
                    if ((opt->state & QStyle::State_Selected)) {        //(opt->state & State_HasFocus)
                        //drawcenterImage(p, bExpanded ? m_expandedImageSelected : m_collapsedImageSelected, opt->rect.adjusted(8, 0, 0, 0));
                        drawcenterImage(p, bExpanded ? m_expandedImageSelected : m_collapsedImageSelected, rectIcon);
                    } else {
                        //drawcenterImage(p, bExpanded ? m_expandedImage : m_collapsedImage, opt->rect.adjusted(8, 0, 0, 0));
                        drawcenterImage(p, bExpanded ? m_expandedImage : m_collapsedImage, rectIcon);
                    }
                }
                return;
            }
        }
        break;
    case PE_IndicatorItemViewItemDrop:
    {
        if (const WizCategoryBaseView *view = dynamic_cast<const WizCategoryBaseView *>(w))
        {
            if (!(view->dragItemFlags() & Qt::ItemIsDropEnabled))
                return;

            p->setRenderHint(QPainter::Antialiasing, true);

            QPen pen;
            pen.setStyle(Qt::SolidLine);
//            pen.setCapStyle(Qt::RoundCap);
            pen.setColor(QColor("#3498DB"));
            pen.setWidth(1);
            p->setPen(pen);
            p->setBrush(Qt::NoBrush);
            if(opt->rect.height() == 0)
            {
                p->drawEllipse(opt->rect.topLeft(), 3, 3);
                p->drawLine(QPoint(opt->rect.topLeft().x()+3, opt->rect.topLeft().y()), opt->rect.topRight());
            } else {
                p->drawRect(opt->rect);
            }
            return;
        }
    }
        break;
    case PE_PanelItemViewRow:
        {
            if (const WizCategoryBaseView *view = dynamic_cast<const WizCategoryBaseView *>(w))
            {
                const QStyleOptionViewItem *vopt = qstyleoption_cast<const QStyleOptionViewItem *>(opt);
                Q_ASSERT(vopt);

                const QTreeWidgetItem* pItemBase = view->itemAt(vopt->rect.center());

                if (pItemBase == nullptr)
                    return;

                const WizCategoryViewSectionItem *secItem = dynamic_cast<const WizCategoryViewSectionItem *>(pItemBase);
                if (NULL != secItem) {                   
                    return;
                }

                if (pItemBase->isSelected()) {
                    // 选区背景
                    QRect rc(vopt->rect);
                    rc.moveLeft(0);
                    rc.setWidth(p->window().width());
                    int nMargin = (opt->rect.height() - WizSmartScaleUI(20)) / 2;
                    rc.adjust(0, nMargin, 0, -nMargin);
                    Utils::WizStyleHelper::drawTreeViewItemBackground(p, rc, opt->state & State_HasFocus);
                }                
            }

            return;
        }
        break;
    default:
        break;
    }
    CWizNoteBaseStyle::drawPrimitive(pe, opt, p, w);
}

int	CWizNoteStyle::pixelMetric(PixelMetric metric, const QStyleOption* option, const QWidget* widget) const
{
    switch (metric)
    {
        case PM_SplitterWidth:
            return 20;
//        case PM_ScrollBarExtent:
//            return 4;
        default:
            return CWizNoteBaseStyle::pixelMetric(metric, option, widget);
    }
}

void CWizNoteStyle::drawcenterImage(QPainter* p, const QImage& image, const QRect& rc) const
{
    bool bHighPixel = WizIsHighPixel();
    int width = bHighPixel ? image.width() / 2 : image.width();
    int height = bHighPixel ? image.height() / 2 : image.height();

    int x = rc.left() + (rc.width() - width) / 2;
    int y = rc.top() + (rc.height() - height) / 2;

    p->drawImage(x, y, image);
}

CWizNoteStyle* CWizNoteStyle::noteStyle(const QString& skinName)
{
    static CWizNoteStyle style(skinName);
    return &style;
}

QStyle* WizGetStyle(const QString& skinName)
{
    return CWizNoteStyle::noteStyle(skinName);
}

class CWizImageButtonStyle : public CWizNoteBaseStyle
{
public:
    CWizImageButtonStyle(const QString& normalBackgroundFileName, const QString& hotBackgroundFileName,
                         const QString& downBackgroundFileName, const QString& disabledBackgroundFileName,
                         const QColor& normalTextColor, const QColor& activeTextColor, const QColor& disableTextColor)
    {
        m_imagePushButton.setImage(normalBackgroundFileName, QPoint(4, 4));
        m_imagePushButtonHot.setImage(hotBackgroundFileName, QPoint(4, 4));
        m_imagePushButtonPressed.setImage(downBackgroundFileName, QPoint(4, 4));
        m_imagePushButtonDisabled.setImage(disabledBackgroundFileName, QPoint(4, 4));
        m_colorTextNormal = normalTextColor;
        m_colorTextActive = activeTextColor;
        m_colorTextDisable = disableTextColor;
    }
private:
    WizSkin9GridImage m_imagePushButton;
    WizSkin9GridImage m_imagePushButtonHot;
    WizSkin9GridImage m_imagePushButtonPressed;
    WizSkin9GridImage m_imagePushButtonDisabled;
    QColor m_colorTextNormal;
    QColor m_colorTextActive;
    QColor m_colorTextDisable;
protected:
    virtual void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
    {
        switch (element)
        {
        case CE_PushButton:
            {
                const QStyleOptionButton* vopt = qstyleoption_cast<const QStyleOptionButton *>(option);
                ATLASSERT(vopt);
                //
                if (!vopt->state.testFlag(QStyle::State_Enabled))
                {
                    m_imagePushButtonDisabled.draw(painter, vopt->rect, 0);
                    painter->setPen(m_colorTextDisable);
                }
                else if (!vopt->state.testFlag(QStyle::State_Raised))
                {
                    m_imagePushButtonPressed.draw(painter, vopt->rect, 0);
                    painter->setPen(m_colorTextActive);
                }
                else if (vopt->state.testFlag(QStyle::State_MouseOver))
                {
                    m_imagePushButtonHot.draw(painter, vopt->rect, 0);
                    painter->setPen(m_colorTextActive);
                }
                else
                {
                    m_imagePushButton.draw(painter, vopt->rect, 0);
                    painter->setPen(m_colorTextNormal);
                }
                //
                painter->drawText(vopt->rect,Qt::AlignCenter, vopt->text);
            }
            break;
        default:
            break;
        }
    }

};

QStyle* WizGetImageButtonStyle(const QString& normalBackgroundFileName, const QString& hotBackgroundFileName,
                               const QString& downBackgroundFileName, const QString& disabledBackgroundFileName,
                               const QColor& normalTextColor, const QColor& activeTextColor, const QColor& disableTextColor)
{
    CWizImageButtonStyle* style = new CWizImageButtonStyle(normalBackgroundFileName,
                                                           hotBackgroundFileName, downBackgroundFileName,
                                                           disabledBackgroundFileName, normalTextColor,
                                                           activeTextColor, disableTextColor);
    return style;
}


void WizNotePlusStyle::drawArrow(const QStyle *style, const QStyleOptionToolButton *toolbutton,
                      const QRect &rect, QPainter *painter, const QWidget *widget)
{
    QStyle::PrimitiveElement pe;
    switch (toolbutton->arrowType) {
    case Qt::LeftArrow:
        pe = QStyle::PE_IndicatorArrowLeft;
        break;
    case Qt::RightArrow:
        pe = QStyle::PE_IndicatorArrowRight;
        break;
    case Qt::UpArrow:
        pe = QStyle::PE_IndicatorArrowUp;
        break;
    case Qt::DownArrow:
        pe = QStyle::PE_IndicatorArrowDown;
        break;
    default:
        return;
    }
    QStyleOption arrowOpt = *toolbutton;
    arrowOpt.rect = rect;
    style->drawPrimitive(pe, &arrowOpt, painter, widget);
}


WizNotePlusStyle::WizNotePlusStyle(QString styleName)
    : QProxyStyle(styleName)
{

}

QWindow* WizNotePlusStyle::qt_getWindow(const QWidget *widget)
{
    return widget ? widget->window()->windowHandle() : nullptr;
}

// The default button and handle gradient
QLinearGradient WizNotePlusStyle::qt_fusion_gradient(const QRect &rect, const QBrush &baseColor, Direction direction)
{
    int x = rect.center().x();
    int y = rect.center().y();
    QLinearGradient gradient;
    switch (direction) {
    case FromLeft:
        gradient = QLinearGradient(rect.left(), y, rect.right(), y);
        break;
    case FromRight:
        gradient = QLinearGradient(rect.right(), y, rect.left(), y);
        break;
    case BottomUp:
        gradient = QLinearGradient(x, rect.bottom(), x, rect.top());
        break;
    case TopDown:
    default:
        gradient = QLinearGradient(x, rect.top(), x, rect.bottom());
        break;
    }
    if (baseColor.gradient())
        gradient.setStops(baseColor.gradient()->stops());
    else {
        QColor gradientStartColor = baseColor.color().lighter(124);
        QColor gradientStopColor = baseColor.color().lighter(102);
        gradient.setColorAt(0, gradientStartColor);
        gradient.setColorAt(1, gradientStopColor);
        //          Uncomment for adding shiny shading
        //            QColor midColor1 = mergedColors(gradientStartColor, gradientStopColor, 55);
        //            QColor midColor2 = mergedColors(gradientStartColor, gradientStopColor, 45);
        //            gradient.setColorAt(0.5, midColor1);
        //            gradient.setColorAt(0.501, midColor2);
    }
    return gradient;
}

QColor WizNotePlusStyle::mergedColors(const QColor &colorA, const QColor &colorB, int factor)
{
    const int maxFactor = 100;
    QColor tmp = colorA;
    tmp.setRed((tmp.red() * factor) / maxFactor + (colorB.red() * (maxFactor - factor)) / maxFactor);
    tmp.setGreen((tmp.green() * factor) / maxFactor + (colorB.green() * (maxFactor - factor)) / maxFactor);
    tmp.setBlue((tmp.blue() * factor) / maxFactor + (colorB.blue() * (maxFactor - factor)) / maxFactor);
    return tmp;
}

QColor WizNotePlusStyle::outline(const QPalette &pal) const {
    if (pal.window().style() == Qt::TexturePattern)
        return QColor(0, 0, 0, 160);
    return pal.background().color().darker(140);
}

QColor WizNotePlusStyle::buttonColor(const QPalette &pal) const {
    QColor buttonColor = pal.button().color();
    int val = qGray(buttonColor.rgb());
    buttonColor = buttonColor.lighter(100 + qMax(1, (180 - val)/6));
    buttonColor.setHsv(buttonColor.hue(), buttonColor.saturation() * 0.75, buttonColor.value());
    return buttonColor;
}

QColor WizNotePlusStyle::tabFrameColor(const QPalette &pal) const {
    if (pal.window().style() == Qt::TexturePattern)
        return QColor(255, 255, 255, 8);
    return buttonColor(pal).lighter(104);
}

QColor WizNotePlusStyle::innerContrastLine() const {
    return QColor(255, 255, 255, 30);
}

void WizNotePlusStyle::drawControl(ControlElement element, const QStyleOption *opt,
                                   QPainter *p, const QWidget *widget) const
{
    switch (element)
    {

    case CE_ToolButtonLabel:
        if (const QStyleOptionToolButton *toolbutton
                = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
            // get shift cor.
            QRect rect = toolbutton->rect;
            int shiftX = 0;
            int shiftY = 0;
            if (toolbutton->state & (State_Sunken | State_On)) {
                shiftX = proxy()->pixelMetric(PM_ButtonShiftHorizontal, toolbutton, widget);
                shiftY = proxy()->pixelMetric(PM_ButtonShiftVertical, toolbutton, widget);
            }
            //
            // Arrow type always overrules and is always shown
            bool hasArrow = toolbutton->features & QStyleOptionToolButton::Arrow;
            if (((!hasArrow && toolbutton->icon.isNull()) && !toolbutton->text.isEmpty())
                || toolbutton->toolButtonStyle == Qt::ToolButtonTextOnly) {
                // no arrow, no icon, only text
                int alignment = Qt::AlignCenter | Qt::TextShowMnemonic; // 水平居中，展示助记符号
                if (!proxy()->styleHint(SH_UnderlineShortcut, opt, widget))
                    alignment |= Qt::TextHideMnemonic;
                rect.translate(shiftX, shiftY);
                p->setFont(toolbutton->font);
                proxy()->drawItemText(p, rect, alignment, toolbutton->palette,
                             opt->state & State_Enabled, toolbutton->text,
                             QPalette::ButtonText);
            } else {
                // icon or text
                QPixmap pm;
                QSize pmSize = toolbutton->iconSize;
                if (!toolbutton->icon.isNull()) {
                    // get pixmap and state
                    QIcon::State state = toolbutton->state & State_On ? QIcon::On : QIcon::Off;
                    QIcon::Mode mode;
                    if (!(toolbutton->state & State_Enabled))
                        mode = QIcon::Disabled;
                    else if ((opt->state & State_MouseOver) && (opt->state & State_AutoRaise))
                        mode = QIcon::Active;
                    else
                        mode = QIcon::Normal;
                    pm = toolbutton->icon.pixmap(qt_getWindow(widget), toolbutton->rect.size().boundedTo(toolbutton->iconSize),
                                                 mode, state);
                    pmSize = pm.size() / pm.devicePixelRatio();
                }

                if (toolbutton->toolButtonStyle != Qt::ToolButtonIconOnly) {
                    // icon+text
                    p->setFont(toolbutton->font);
                    QRect pr = rect, // 图标的边界
                    tr = rect; // 文本的边界
                    int alignment = Qt::TextShowMnemonic; // 可在这里添加文本垂直对齐
                    if (!proxy()->styleHint(SH_UnderlineShortcut, opt, widget))
                        alignment |= Qt::TextHideMnemonic;
                    if (toolbutton->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                        // 文本居于图标下
                        pr.setHeight(pmSize.height() + 6);
                        tr.adjust(0, pr.height() - 1, 0, -1);
                        pr.translate(shiftX, shiftY);
                        if (!hasArrow) {
                            // 绘制图标
                            proxy()->drawItemPixmap(p, pr, Qt::AlignCenter, pm);
                        } else {
                            // 绘制箭头
                            drawArrow(proxy(), toolbutton, pr, p, widget);
                        }
                        alignment |= Qt::AlignCenter; // 文本水平居中
                    } else {
                        // 绘制其他情形
                        pr.setWidth(pmSize.width() + 8);
                        tr.adjust(pr.width(), 0, 0, 0);
                        pr.translate(shiftX, shiftY);
                        if (!hasArrow) {
                            // 绘制图标
                            proxy()->drawItemPixmap(p, QStyle::visualRect(opt->direction, rect, pr), Qt::AlignCenter, pm);
                        } else {
                            // 绘制箭头
                            drawArrow(proxy(), toolbutton, pr, p, widget);
                        }
                        alignment |= Qt::AlignLeft | Qt::AlignVCenter; // 文本左对齐，垂直居中
                        // 草！是图片文件中的图案没有居中！！！！！
                    }
                    tr.translate(shiftX, shiftY);
                    // 绘制文本
                    proxy()->drawItemText(p, QStyle::visualRect(opt->direction, rect, tr), alignment, toolbutton->palette,
                                 toolbutton->state & State_Enabled, toolbutton->text,
                                 QPalette::ButtonText);
                } else {
                    // only icon
                    rect.translate(shiftX, shiftY);
                    if (hasArrow) {
                        // 绘制箭头
                        drawArrow(proxy(), toolbutton, rect, p, widget);
                    } else {
                        // 绘制图标
                        proxy()->drawItemPixmap(p, rect, Qt::AlignCenter, pm);
                    }
                }
            }
        }
        break;
    default:
        QProxyStyle::drawControl(element, opt, p, widget);
        break;

    }
}

void WizNotePlusStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                      QPainter *p, const QWidget *widget) const
{
    switch (cc) {

    case CC_ToolButton:
    {

        if (const QStyleOptionToolButton *toolbutton
            = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {

            // 分别计算子控件按钮和菜单指示器的边界
            QRect button, menuarea;
            button = proxy()->subControlRect(cc, toolbutton, SC_ToolButton, widget);
            menuarea = proxy()->subControlRect(cc, toolbutton, SC_ToolButtonMenu, widget);

            //-------------------------------------------------------------------
            // 按钮状态判断
            //-------------------------------------------------------------------

            State bflags = toolbutton->state & ~State_Sunken; // 当前状态与非按下状态

            if (bflags & State_AutoRaise) { // 如果非按下状态且设置了自动回弹
                if (!(bflags & State_MouseOver) || !(bflags & State_Enabled)) {
                    bflags &= ~State_Raised;
                }
            }
            State mflags = bflags; // 下拉指示箭头状态
            if (toolbutton->state & State_Sunken) {
                if (toolbutton->activeSubControls & SC_ToolButton)
                    bflags |= State_Sunken;
                mflags |= State_Sunken;
            }

            //-------------------------------------------------------------------
            // 绘制样式元素
            //-------------------------------------------------------------------
            // 绘制按钮悬浮
            QStyleOption pannel = *toolbutton;
            if ( ( bflags | mflags ) & (State_Sunken | State_On | State_Raised)) {
                pannel.state = bflags | mflags;
                proxy()->drawPrimitive(PE_PanelButtonCommand, &pannel, p, widget);
            }

            // 绘制按钮子控件
            QStyleOption tool = *toolbutton;
            if (toolbutton->subControls & SC_ToolButton) { // 如果存在按钮子控件
                if (bflags & (State_Sunken | State_On | State_Raised)) {
                    tool.state = bflags;
                    tool.rect = button;
                    //proxy()->drawPrimitive(PE_PanelButtonTool, &tool, p, widget);
                }
            }

            // 绘制聚焦框
            if (toolbutton->state & State_HasFocus) {
                QStyleOptionFocusRect fr;
                fr.QStyleOption::operator=(*toolbutton);
                fr.rect.adjust(3, 3, -3, -3); // 边界内缩
                if (toolbutton->features & QStyleOptionToolButton::MenuButtonPopup)
                 // 如果设置了下拉指示箭头，则聚焦框左收缩
                    fr.rect.adjust(0, 0, -proxy()->pixelMetric(QStyle::PM_MenuButtonIndicator,
                                                      toolbutton, widget), 0);
                proxy()->drawPrimitive(PE_FrameFocusRect, &fr, p, widget);
            }

            // 绘制标签区域
            QStyleOptionToolButton label = *toolbutton;
            label.state = bflags;
            int fw = proxy()->pixelMetric(PM_DefaultFrameWidth, opt, widget);
            label.rect = button.adjusted(fw, fw, -fw, -fw); // 向内收缩默认框架厚度的距离
            proxy()->drawControl(CE_ToolButtonLabel, &label, p, widget);

            // 绘制菜单子控件
            if (toolbutton->subControls & SC_ToolButtonMenu) {
                tool.rect = menuarea;
                tool.state = mflags;
                if (mflags & (State_Sunken | State_On | State_Raised))
                {
                    // 为下拉箭头绘制按钮效果
                    //proxy()->drawPrimitive(PE_IndicatorButtonDropDown, &tool, p, widget);
                }
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &tool, p, widget);
            } else if (toolbutton->features & QStyleOptionToolButton::HasMenu) {
              // 如果没有设置菜单子控件区域，但是还设置了菜单的话，手动绘制按钮右下角
              // 菜单指示器占按钮高度的比例
                int mbi = proxy()->pixelMetric(PM_MenuButtonIndicator, toolbutton, widget);
                QRect ir = toolbutton->rect;
                QStyleOptionToolButton newBtn = *toolbutton;
                // 绘制按钮指示器
                newBtn.rect = QRect(
                     ir.right() + 5 - mbi, // 左上角X坐标
                     ir.y() + ir.height() - mbi + 4, // 左上角Y坐标
                     mbi - 6,
                     mbi - 6);
                newBtn.rect = visualRect(toolbutton->direction, button, newBtn.rect);
                proxy()->drawPrimitive(PE_IndicatorArrowDown, &newBtn, p, widget);
            }
        }
        break;
    default:
        QProxyStyle::drawComplexControl(cc, opt, p, widget);
        break;
    }

    }
}
