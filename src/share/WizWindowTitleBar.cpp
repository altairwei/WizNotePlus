#include "WizWindowTitleBar.h"

#include <QPixmap>
#include <QStyle>
#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QDebug>

#include "libs/3rdparty/framelesshelper/framelesswindowsmanager.h"

#include "utils/WizStyleHelper.h"
#include "WizMisc.h"


WizWindowTitleBar::WizWindowTitleBar(QWidget *parent, QWidget* window, QWidget* shadowContainerWidget, bool canResize)
    : QWidget(parent)
    , m_window(window)
    , m_shadowContainerWidget(shadowContainerWidget)
    , m_oldContentsMargin(10, 10, 10, 10)
    , m_canResize(canResize)
{
    // 不继承父组件的背景色
    setAutoFillBackground(true);

    m_minimize = new QToolButton(this);
    m_maximize = new QToolButton(this);
    m_restore = new QToolButton(this);
    m_close = new QToolButton(this);

    m_close->setObjectName("window-close-btn");
    m_minimize->setObjectName("window-min-btn");
    m_maximize->setObjectName("window-max-btn");
    m_restore->setObjectName("window-restore-btn");
    m_restore->hide();

    m_titleLabel = new QLabel(this);
    m_titleLabel->setText("");
    m_window->setWindowTitle("");

    connect(m_close, SIGNAL(clicked()), m_window, SLOT(close()));
    connect(m_minimize, SIGNAL(clicked()), this, SLOT(showSmall()));
    connect(m_maximize, SIGNAL(clicked()), this, SLOT(showMaxRestore()));
    connect(m_restore, SIGNAL(clicked()), this, SLOT(showMaxRestore()));

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_maximize->setEnabled(m_canResize);
    m_minimize->setEnabled(m_canResize);

    setHitTestVisible(m_minimize);
    setHitTestVisible(m_maximize);
    setHitTestVisible(m_restore);
    setHitTestVisible(m_close);
}

void WizWindowTitleBar::layoutTitleBar()
{
    QHBoxLayout *hbox = new QHBoxLayout(this);

    hbox->addWidget(m_titleLabel);
    hbox->addWidget(m_minimize);
    hbox->addWidget(m_maximize);
    hbox->addWidget(m_restore);
    hbox->addWidget(m_close);

    hbox->insertStretch(1, 500);
    hbox->setSpacing(0);

    hbox->setContentsMargins(0, 0, 0, 0);
}

void WizWindowTitleBar::windowStateChanged()
{
    if (m_window->isMaximized() || m_window->isFullScreen()) {
        m_maximize->hide();
        m_restore->show();
    } else if (!m_window->isMinimized()) {
        m_restore->hide();
        m_maximize->show();
    }
}

void WizWindowTitleBar::showSmall()
{
    m_window->showMinimized();
}

void WizWindowTitleBar::showMaxRestore()
{
    if (!m_canResize)
        return;

    if (Qt::WindowMaximized == m_window->windowState()) {
        // Restore shadow effect when exit maximization
        //m_shadowContainerWidget->setContentsMargins(m_oldContentsMargin);
        m_window->showNormal();
    } else {
        // Hide shadow effect when maximize mainwindow.
        m_oldContentsMargin = m_shadowContainerWidget->contentsMargins();
        //m_shadowContainerWidget->setContentsMargins(0, 0, 0, 0);
        m_window->showMaximized();
    }
}

/*
void WizWindowTitleBar::mousePressEvent(QMouseEvent *me)
{
    m_startPos = me->globalPos();
    m_clickPos = mapTo(m_window, me->pos());
}

void WizWindowTitleBar::mouseMoveEvent(QMouseEvent *me)
{
    if (Qt::WindowMaximized == m_window->windowState())
        return;
    m_window->move(me->globalPos() - m_clickPos);
}

void WizWindowTitleBar::mouseDoubleClickEvent ( QMouseEvent * event )
{
    if (event->button() == Qt::LeftButton)
    {
        showMaxRestore();
    }
}
*/

void WizWindowTitleBar::setContentsMargins(QMargins margins)
{
    m_oldContentsMargin = margins;
    QWidget::setContentsMargins(margins);
    layout()->setContentsMargins(margins);
}

void WizWindowTitleBar::setText(QString title)
{
    m_titleLabel->setText(title);
}

QString WizWindowTitleBar::text() const
{
    return m_titleLabel->text();
}

void WizWindowTitleBar::setHitTestVisible(QObject *obj)
{
    __flh_ns::FramelessWindowsManager::setHitTestVisibleInChrome(m_window->windowHandle(), obj, true);
}