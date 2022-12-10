#include "WizWindowTitleBar.h"

#include <QPixmap>
#include <QStyle>
#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QDebug>
#include <QStyleOption>
#include <QPainter>

#include "framelesshelper/src/core/framelesshelper.h"

#include "utils/WizStyleHelper.h"
#include "WizMisc.h"


WizWindowTitleBar::WizWindowTitleBar(QWidget *parent, QWidget* window, bool canResize)
    : QWidget(parent)
    , m_window(window)
    , m_oldContentsMargin(10, 10, 10, 10)
    , m_canResize(canResize)
{
    // 不继承父组件的背景色
    setAutoFillBackground(true);

    m_minimize = new QToolButton(this);
    m_maximize = new QToolButton(this);
    m_maximize->setCheckable(true);
    m_close = new QToolButton(this);

    m_close->setObjectName("window-close-btn");
    m_minimize->setObjectName("window-min-btn");
    m_maximize->setObjectName("window-max-btn");

    m_titleLabel = new QLabel(this);
    m_titleLabel->setText("");
    m_window->setWindowTitle("");

    connect(m_close, SIGNAL(clicked()), m_window, SLOT(close()));
    connect(m_minimize, SIGNAL(clicked()), this, SLOT(showSmall()));
    connect(m_maximize, SIGNAL(clicked()), this, SLOT(showMaxRestore()));

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_maximize->setEnabled(m_canResize);
    m_minimize->setEnabled(m_canResize);
}

void WizWindowTitleBar::layoutTitleBar()
{
    QHBoxLayout *hbox = new QHBoxLayout(this);

    hbox->addWidget(m_titleLabel);
    hbox->addWidget(m_minimize);
    hbox->addWidget(m_maximize);
    hbox->addWidget(m_close);

    hbox->insertStretch(1, 500);
    hbox->setSpacing(0);

    hbox->setContentsMargins(0, 0, 0, 0);
}

void WizWindowTitleBar::windowStateChanged()
{
    if (m_window->isMaximized() || m_window->isFullScreen()) {
        m_maximize->setChecked(true);
    } else if (!m_window->isMinimized()) {
        m_maximize->setChecked(false);
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
        m_window->showNormal();
    } else {
        m_window->showMaximized();
    }
}

void WizWindowTitleBar::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

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