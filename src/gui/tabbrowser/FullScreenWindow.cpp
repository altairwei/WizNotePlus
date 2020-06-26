#include "FullScreenWindow.h"
#include "FullScreenNotification.h"

#include <QAction>
#include <QLabel>
#include <QWebEngineView>

FullScreenWindow::FullScreenWindow(QWebEngineView *oldView, QWidget *parent)
    : QWidget(parent)
    , m_view(new QWebEngineView(this))
    , m_notification(new FullScreenNotification(this))
    , m_oldView(oldView)
    , m_oldGeometry(oldView->window()->geometry())
{
    // Show tips on top
    m_view->stackUnder(m_notification);

    // Create 'Exit' action
    auto exitAction = new QAction(this);
    exitAction->setShortcut(Qt::Key_Escape);
    connect(exitAction, &QAction::triggered, [this]() {
        m_view->triggerPageAction(QWebEnginePage::ExitFullScreen);
        emit ExitFullScreen();
    });
    addAction(exitAction);

    // Move web page to new view
    m_view->setPage(m_oldView->page());
    setGeometry(m_oldGeometry);
    showFullScreen();
    m_oldView->window()->hide();
}

FullScreenWindow::~FullScreenWindow()
{
    m_oldView->setPage(m_view->page());
    m_oldView->window()->setGeometry(m_oldGeometry);
    m_oldView->window()->show();
    hide();
}

void FullScreenWindow::resizeEvent(QResizeEvent *event)
{
    QRect viewGeometry(QPoint(0, 0), size());
    m_view->setGeometry(viewGeometry);

    QRect notificationGeometry(QPoint(0, 0), m_notification->sizeHint());
    notificationGeometry.moveCenter(viewGeometry.center());
    m_notification->setGeometry(notificationGeometry);

    QWidget::resizeEvent(event);
}
