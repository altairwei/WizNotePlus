#ifndef WIZFRAMELESSWINDOW_H
#define WIZFRAMELESSWINDOW_H

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

#include "libs/3rdparty/framelesshelper/src/core/framelesshelper.h"

template <class Base>
class WizFramelessWindow : public Base
{
public:
    explicit WizFramelessWindow(QWidget *parent, bool canResize)
        : Base(parent)
        , m_clientWidget(NULL)
        , m_clientLayout(NULL)
        , m_canResize(canResize)
        , m_helper(new __flh_ns::FramelessHelper)
    {
        Base* pT = this;

        pT->setAttribute(Qt::WA_DontCreateNativeAncestors);

        m_titleBar = new WizWindowTitleBar(this, this, canResize);

        m_clientWidget = new QWidget(this);
        m_clientLayout = new QBoxLayout(QBoxLayout::TopToBottom);
        m_clientWidget->setLayout(m_clientLayout);
        m_clientLayout->setSpacing(0);
        m_clientLayout->setContentsMargins(0, 0, 0, 0);
    
        m_clientLayout->addWidget(m_titleBar);
    }

public:
    QWidget* rootWidget() const { return m_clientWidget; }
    QWidget *clientWidget() const { return m_clientWidget; }
    QLayout* clientLayout() const { return m_clientLayout; }
    WizWindowTitleBar* titleBar() const { return m_titleBar; }
    void setTitleText(QString title) { m_titleBar->setText(title); }

    void setHitTestVisible(QObject *obj)
    {
        m_helper->setHitTestVisible(obj);
    }

private:
    QWidget* m_clientWidget;
    QLayout* m_clientLayout;
    WizWindowTitleBar* m_titleBar;
    __flh_ns::FramelessHelper *m_helper;
    bool m_canResize;

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

    void showEvent(QShowEvent *event) override
    {
        Base* pT = this;
        Base::showEvent(event);
        static bool inited = false;
        if (!inited) {
            const auto win = pT->windowHandle();
            if (win) {
                m_helper->setWindow(win);
                m_helper->setTitleBarHeight(m_titleBar->height());
                m_helper->setResizable(m_canResize);
                m_helper->setResizeBorderThickness(2);
                m_helper->setHitTestVisible(m_titleBar->closeButton());
                m_helper->setHitTestVisible(m_titleBar->maxButton());
                m_helper->setHitTestVisible(m_titleBar->minButton());
                m_helper->install();
                pT->setContentsMargins(1, 1, 1, 1);
                inited = true;
            }
        }
    }

#ifdef Q_OS_WIN
	bool nativeEvent(const QByteArray &eventType, void *message, long *result) override
    {
        if (!m_helper)
            return Base::nativeEvent(eventType, message, result);

        if (m_helper->handleNativeEvent(this->windowHandle(), eventType, message, result))
            return true;
        else
            return Base::nativeEvent(eventType, message, result);
    }
#endif // Q_OS_WIN
};

#endif // WIZFRAMELESSWINDOW_H
