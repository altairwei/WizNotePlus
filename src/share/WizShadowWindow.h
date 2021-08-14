#ifndef WIZSHADOWWINDOW_H
#define WIZSHADOWWINDOW_H

#include <QWidget>
#include <QPainter>
#include <QPaintEngine>
#include <QMouseEvent>
#include <QDebug>
#include <QBoxLayout>
#include <QTimer>
#include <QToolButton>

#include "WizWindowTitleBar.h"
#include "WizShadowEffect.h"

#include "libs/3rdparty/framelesshelper/framelesshelper.h"
#include "libs/3rdparty/framelesshelper/framelesswindowsmanager.h"

//-------------------------------------------------------------------
// 给Window制造阴影的模板，Base应该为具有Qt::Window的QWidget
//-------------------------------------------------------------------

template <class Base>
class WizShadowWindow
        : public Base
{
public:
    explicit WizShadowWindow(QWidget *parent, bool canResize)
        : Base(parent)
        , m_shadowWidget(NULL)
        , m_clientWidget(NULL)
        , m_clientLayout(NULL)
    {
        Base* pT = this;

        pT->setAttribute(Qt::WA_DontCreateNativeAncestors);
        pT->createWinId();

        //pT->setAttribute(Qt::WA_TranslucentBackground); //enable MainWindow to be transparent
        //pT->setWindowFlags(Qt::FramelessWindowHint);
        //pT->setContentsMargins(0, 0, 0, 0);

        // 设置窗体布局
        QLayout* windowLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        pT->setLayout(windowLayout);
        windowLayout->setContentsMargins(0, 0, 0, 0);
        windowLayout->setSpacing(0);

        // 设置阴影
        int shadowSize = 20;
        //m_shadowWidget = new WizShadowWidget(this, shadowSize, canResize);
        m_shadowWidget = new QWidget(this);
        //m_shadowWidget->setContentsMargins(shadowSize, shadowSize, shadowSize, shadowSize);
        windowLayout->addWidget(m_shadowWidget);

        // 设置阴影布局
        QLayout* rootLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        m_shadowWidget->setLayout(rootLayout);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(0);

        // Shadow client is the region to place title bar and real client widget
        QWidget* shadowClientWidget = new QWidget(m_shadowWidget);
        rootLayout->addWidget(shadowClientWidget);

        QLayout* shadowClientLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        shadowClientLayout->setContentsMargins(0, 0, 0, 0);
        shadowClientLayout->setSpacing(0);
        shadowClientWidget->setLayout(shadowClientLayout);
        shadowClientWidget->setAutoFillBackground(true);
        shadowClientWidget->setCursor(QCursor(Qt::ArrowCursor));

        // Put title bar to client widget
        m_titleBar = new WizWindowTitleBar(shadowClientWidget, this, m_shadowWidget, canResize);
        shadowClientLayout->addWidget(m_titleBar);

        // Real client is the region to place main UI,
        // which is different from shadow client.
        m_clientWidget = new QWidget(shadowClientWidget);
        shadowClientLayout->addWidget(m_clientWidget);
        m_clientLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        m_clientWidget->setLayout(m_clientLayout);
        m_clientLayout->setSpacing(0);
        m_clientLayout->setContentsMargins(0, 0, 0, 0);
    }

public:
    QWidget* rootWidget() const { return m_shadowWidget; }
    QWidget *clientWidget() const { return m_clientWidget; }
    QLayout* clientLayout() const { return m_clientLayout; }
    WizWindowTitleBar* titleBar() const { return m_titleBar; }
    void setTitleText(QString title) { m_titleBar->setText(title); }

private:
    QWidget* m_shadowWidget;
    QWidget* m_clientWidget;
    QLayout* m_clientLayout;
    WizWindowTitleBar* m_titleBar;

protected:
    virtual void changeEvent (QEvent * event)
    {
        Base::changeEvent(event);

        Base* pT = this;
        bool shouldUpdate = false;
        if (event->type() == QEvent::WindowStateChange) {
            if (pT->isMaximized() || pT->isFullScreen()) {
                pT->setContentsMargins(0, 0, 0, 0);
            } else if (!pT->isMinimized()) {
                pT->setContentsMargins(1, 1, 1, 1);
            }
            m_titleBar->windowStateChanged();
            shouldUpdate = true;
        } else if (event->type() == QEvent::ActivationChange) {
            shouldUpdate = true;
        }
        if (shouldUpdate) {
            pT->update();
        }
    }

    virtual void layoutTitleBar()
    {
        m_titleBar->layoutTitleBar();
    }

    void showEvent(QShowEvent *event)
    {
        Base* pT = this;
        Base::showEvent(event);
        static bool inited = false;
        if (!inited) {
            const auto win = pT->windowHandle();
            if (win) {
                __flh_ns::FramelessWindowsManager::addWindow(win);
                __flh_ns::FramelessWindowsManager::setResizable(win, true);
                __flh_ns::FramelessWindowsManager::setTitleBarHeight(win, m_titleBar->height());
                pT->setContentsMargins(1, 1, 1, 1);
                inited = true;
            }
        }
    }
};

#endif // WIZSHADOWWINDOW_H
