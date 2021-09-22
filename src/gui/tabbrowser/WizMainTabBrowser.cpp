#include "WizMainTabBrowser.h"

#include <stdexcept>

#include <QWidget>
#include <QMessageBox>
#include <QLabel>
#include <QMenu>
#include <QTabBar>
#include <QLayout>
#include <QShortcut>
#include <QKeySequence>
#include <QKeyEvent>

#include "share/WizWebEngineView.h"
#include "share/WizGlobal.h"
#include "WizDef.h"
#include "share/WizMisc.h"
#include "utils/WizStyleHelper.h"
#include "utils/WizPathResolve.h"
#include "database/WizDatabaseManager.h"
#include "gui/documentviewer/WizTitleBar.h"
#include "gui/documentviewer/WizDocumentView.h"
#include "gui/tabbrowser/WizWebsiteView.h"
#include "gui/tabbrowser/WebEngineWindow.h"
#include "gui/tabbrowser/TabButton.h"


WizMainTabBrowser::WizMainTabBrowser(WizExplorerApp& app, QWidget *parent)
    : QTabWidget(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_strTheme(Utils::WizStyleHelper::themeName())
{
    QTabBar *tabBar = this->tabBar();
    tabBar->setTabsClosable(false);
    tabBar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    tabBar->setMovable(true);
    tabBar->setContextMenuPolicy(Qt::CustomContextMenu);

    setElideMode(Qt::ElideRight);

    // This signal is emitted whenever the current page index changes.
    connect(this, &QTabWidget::currentChanged, this, &WizMainTabBrowser::handleCurrentChanged);

    connect(tabBar, &QTabBar::customContextMenuRequested, this, &WizMainTabBrowser::handleContextMenuRequested);
    connect(tabBar, &QTabBar::tabCloseRequested, this, &WizMainTabBrowser::handleTabCloseRequested);

    tabBar->installEventFilter(this);

    new QShortcut(QKeySequence("Ctrl+W"), this, SLOT(closeCurrentTab()));
}

void WizMainTabBrowser::handleCurrentChanged(int index)
{
    if (index != -1) {
        emit titleChanged(tabPage(index)->Title());
    } else {
        emit titleChanged(QString());
    }
}

/** Show menu when right clicked */
void WizMainTabBrowser::handleContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    int index = tabBar()->tabAt(pos);
    bool isLocked = isTabLocked(index);
    // ensure click pos is in tab not tabbar
    if (index != -1) {
        // close actions
        QAction *action = menu.addAction(tr("Close Tab"));
        connect(action, &QAction::triggered, this, [this, index]() {
            closeTab(index);
        });
        action = menu.addAction(tr("Close Other Tabs"));
        connect(action, &QAction::triggered, this, [this, index]() {
            closeOtherTabs(index);
        });
        action = menu.addAction(tr("Close Left Tabs"));
        connect(action, &QAction::triggered, this, [this, index]() {
            closeLeftTabs(index);
        });
        action = menu.addAction(tr("Close Right Tabs"));
        connect(action, &QAction::triggered, this, [this, index]() {
            closeRightTabs(index);
        });
        action = menu.addAction(tr("Close All Tabs"));
        connect(action, &QAction::triggered, this, [this]() {
            closeAllTabs();
        });
        action = menu.addAction(tr("Force to Close Tab"));
        connect(action, &QAction::triggered, this, [this, index]() {
            closeTab(index, true);
        });
        menu.addSeparator();
        // lock action
        if (isLocked) {
            QAction* action = menu.addAction(tr("Unlock The Tab"));
            connect(action, &QAction::triggered, this, [this, index](){
               this->unlockTab(index);
            });
        } else {
            QAction *action = menu.addAction(tr("Lock The Tab"));
            connect(action, &QAction::triggered, this, [this, index](){
               this->lockTab(index);
            });
        }

        auto page = tabPage(index);
        auto page_actions = page->TabContextMenuActions();
        if (!page_actions.isEmpty()) {
            menu.addSeparator();
            menu.addActions(page_actions);
        }

        menu.exec(QCursor::pos());
    }
}

void WizMainTabBrowser::triggeredFullScreen()
{
    WizWebEngineView *webView = currentWebView();
    if (!webView)
        // Avoid responsing to non-webpage widget
        return;
    FullScreenWindow *fWindow = new FullScreenWindow(webView);
    // Only full screen action (F11) comes from outside of web page needed to 
    // handle ExitFullScreen requests separately. Because there is no way to 
    // emit QWebEngineFullScreenRequest by C++ side.
    connect(fWindow, &FullScreenWindow::ExitFullScreen,
                    this, &WizMainTabBrowser::handleExitFullScreen);
    m_fullScreenWindow.reset(fWindow);
}

void WizMainTabBrowser::handleExitFullScreen()
{
    if (!m_fullScreenWindow.isNull())
        m_fullScreenWindow.reset();
}

void WizMainTabBrowser::fullScreenRequested(QWebEngineFullScreenRequest request)
{
    if (request.toggleOn()) {
        if (m_fullScreenWindow)
            return;
        request.accept();
        // Request always comes from current web page.
        m_fullScreenWindow.reset(new FullScreenWindow(currentWebView()));
    } else {
        if (!m_fullScreenWindow)
            return;
        request.accept();
        m_fullScreenWindow.reset();
    }
}

/**
 * @brief Create an empty web page with WizWebsiteView, but do not focus on new tab.
 * 
 * @return WizWebEngineView* 
 */
WizWebEngineView *WizMainTabBrowser::createBackgroundTab()
{
    // create default website view
    WizWebsiteView* websiteView = new WizWebsiteView(m_app);
    int index = addTab(websiteView, tr("Untitled"));
    setupTab(index);
    setupTabPage(websiteView);
    //
    return websiteView->webView();
}

WizWebEngineView *WizMainTabBrowser::createWindow()
{
    WebEngineWindow *webWindow = new WebEngineWindow(this);
    webWindow->setAttribute(Qt::WA_DeleteOnClose);
    webWindow->show();
    webWindow->raise();
    return webWindow->webView();
}

/**
 * @brief Create an empty web page, then focus on new tab page.
 * 
 * @return WizWebEngineView* 
 */
WizWebEngineView *WizMainTabBrowser::createTab()
{
    // create default website view
    WizWebsiteView* websiteView = new WizWebsiteView(m_app);
    createTab(websiteView);
    //
    return websiteView->webView();
}

/**
 * @brief create a tab with url
 * @param url The url can be local filename.
 */
int WizMainTabBrowser::createTab(const QUrl &url)
{
    // create default website view
    WizWebsiteView* websiteView = new WizWebsiteView(m_app);
    websiteView->viewHtml(url);
    int index = createTab(websiteView);
    setTabText(index, url.url());
    return index;
}

int WizMainTabBrowser::createTab(AbstractTabPage *tabPage)
{
    int index = addTab(tabPage, tabPage->Title());
    setupTab(index);
    setupTabPage(tabPage);
    // Workaround for QTBUG-61770
    tabPage->resize(currentWidget()->size());
    setCurrentWidget(tabPage);

    return index;
}

void WizMainTabBrowser::setupTab(int index)
{
    // Close Button
    if (index != -1) {
        TabButton* closeBtn = new TabButton(tabBar());
        closeBtn->setIcon(WizLoadSkinIcon(m_strTheme, "tab_close", QSize(16, 16)));
        connect(closeBtn, &QAbstractButton::clicked, this, &WizMainTabBrowser::handleCloseButtonClicked);
        tabBar()->setTabButton(index, QTabBar::RightSide, closeBtn);
        closeBtn->hide();

        QWidget* placeHolder = new QWidget(tabBar());
        placeHolder->setFixedSize(QSize(16, 16));
        tabBar()->setTabButton(index, QTabBar::LeftSide, placeHolder);

        QMap<QString, QVariant> status;
        status["Locked"] = QVariant(false);
        tabBar()->setTabData(index, status);
    }
}

void WizMainTabBrowser::handleTabCloseRequested(int index)
{
    closeTab(index);
}

void WizMainTabBrowser::handleCloseButtonClicked()
{
    QObject *b = sender();
    // Find which close button was clicked
    for (int i = count() - 1; i >= 0; --i) {
        if (b == tabBar()->tabButton(i, QTabBar::RightSide)) {
            closeTab(i);
            return;
        }
    }
}

void WizMainTabBrowser::destroyTab(int index)
{
    auto p = tabPage(index);
    removeTab(index);
    p->deleteLater();
}

void WizMainTabBrowser::closeTab(int index, bool force /*= false*/)
{
    if (index != -1) {
        // Only one page needed to be closed.
        if (!isTabLocked(index))
            tabPage(index)->RequestClose(force);
    }
}

void WizMainTabBrowser::closeCurrentTab()
{
    closeTab(currentIndex());
}

void WizMainTabBrowser::closeOtherTabs(int index)
{
    // Locked tabs will disturb the order of tabs,
    // To avoid that, we collect all the pages first.
    for (int i = count() - 1; i > index; --i) {
        if (!isTabLocked(i)) {
            m_scheduleForClose.append(tabPage(i));
        }
    }

    for (int i = index - 1; i >= 0; --i) {
        if (!isTabLocked(i)) {
            m_scheduleForClose.append(tabPage(i));
        }
    }

    doCloseSchedule();
}

void WizMainTabBrowser::closeAllTabs()
{
    for (int i = count() - 1; i >= 0; --i) {
        if (!isTabLocked(i)) {
             m_scheduleForClose.append(tabPage(i));
        }
    }

    doCloseSchedule();
}

void WizMainTabBrowser::closeLeftTabs(int index)
{
    for (int i = index - 1; i >= 0; --i) {
        if (!isTabLocked(i)) {
             m_scheduleForClose.append(tabPage(i));
        }
    }

    doCloseSchedule();
}

void WizMainTabBrowser::closeRightTabs(int index)
{
    for (int i = count() - 1; i > index; --i) {
        if (!isTabLocked(i)) {
             m_scheduleForClose.append(tabPage(i));
        }
    }

    doCloseSchedule();
}

void WizMainTabBrowser::doCloseSchedule()
{
    for (auto page : m_scheduleForClose) {
        page->RequestClose();
    }

    m_scheduleForClose.clear();
}

QMap<QString, QVariant> WizMainTabBrowser::tabStatus(int index) const
{
    return tabBar()->tabData(index).toMap();
}

void WizMainTabBrowser::switchTabStatus(int index, bool lock)
{
    if (index != -1) {
        auto status = tabStatus(index);
        QWidget* tb = tabBar()->tabButton(index, QTabBar::RightSide);
        TabButton* tabBtn = qobject_cast<TabButton*>(tb);
        if (tabBtn) {
            tabBtn->setIcon(WizLoadSkinIcon(m_strTheme, lock ? "tab_lock" : "tab_close", QSize(16, 16)));
            status["Locked"] = QVariant(lock);
            tabBar()->setTabData(index, status);
        }
    }
}

void WizMainTabBrowser::lockTab(int index)
{
    switchTabStatus(index, true);
}

void WizMainTabBrowser::unlockTab(int index)
{
    switchTabStatus(index, false);

}

bool WizMainTabBrowser::isTabLocked(int index) const
{
    if (index != -1) {
        QMap<QString, QVariant> status = tabStatus(index);
        return status["Locked"].toBool();
    }

    return false;
}

/**
 * @brief 从标签页中获得页面视图
 * @param index 标签序号
 * @return
 */
WizWebEngineView* WizMainTabBrowser::getWebView(int index) const
{
    // 判断是文档视图还是页面视图
    WizDocumentView* docView = qobject_cast<WizDocumentView*>(widget(index));
    WizWebsiteView* websiteView = qobject_cast<WizWebsiteView*>(widget(index));
    if ( websiteView != nullptr && docView == nullptr )
    {
        return websiteView->getWebView();

    } else if (websiteView == nullptr && docView != nullptr )
    {
        return docView->web();

    } else {
        return nullptr;
    }
}

/**
 * @brief 获取当前页面视图
 * @return
 */
WizWebEngineView* WizMainTabBrowser::currentWebView() const
{
    return getWebView(currentIndex());
}

AbstractTabPage *WizMainTabBrowser::tabPage(int index) const noexcept(false)
{
    auto tabPage = qobject_cast<AbstractTabPage *>(widget(index));
    if (tabPage) {
        return tabPage;
    } else {
        throw std::runtime_error("Can not cast to AbstractTabPage.");
    }
}

/**
 * @brief 初始化WizWebEngineView
 * @param view
 */
void WizMainTabBrowser::setupView(WizWebEngineView* view) {
    QWebEnginePage *webPage = view->page();
    connect(webPage, &QWebEnginePage::fullScreenRequested, this, &WizMainTabBrowser::fullScreenRequested);
}

void WizMainTabBrowser::setupTabPage(AbstractTabPage *tabPage)
{
    connect(tabPage, &AbstractTabPage::titleChanged, [this, tabPage](const QString &title) {
        int index = indexOf(tabPage);
        if (index != -1) {
            setTabText(index, title);
            setTabToolTip(index, title);
        }
        if (currentIndex() == index)
            emit titleChanged(title);
    });

    connect(tabPage, &AbstractTabPage::pageCloseRequested, [this, tabPage]() {
        int index = indexOf(tabPage);
        if (index != -1) {
            destroyTab(index);
        }
    });
}

void WizMainTabBrowser::keyPressEvent(QKeyEvent* ev)
{
    if (ev->modifiers() && ev->key()) {
        if (ev->modifiers() & Qt::AltModifier
                && ev->key() >= Qt::Key_1  && ev->key() <= Qt::Key_9) {
            int index = ev->key() - Qt::Key_0;
            if (index <= count() && index >= 0)
                setCurrentIndex(index-1);

            return;
        }
    }

    QTabWidget::keyPressEvent(ev);
}

bool WizMainTabBrowser::eventFilter(QObject* watched, QEvent* event)
{
    if(watched == tabBar())
    {
        // Show close button when tab is hovered.
        switch(event->type())
        {
            case QEvent::HoverEnter:
            case QEvent::HoverLeave:
            {
                bool isEnter = event->type() == QEvent::HoverEnter;
                auto he = static_cast<QHoverEvent*>(event);
                QPoint pos = isEnter ? he->pos() : he->oldPos();
                int index = tabBar()->tabAt(pos);
                if (index != -1) {
                    auto closeBtn = tabBar()->tabButton(index, QTabBar::RightSide);
                    isEnter ? closeBtn->show() : closeBtn->hide();
                }
            }
            break;
            case QEvent::HoverMove:
            {
                auto he = static_cast<QHoverEvent*>(event);
                int indexNew = tabBar()->tabAt(he->pos());
                int indexOld = tabBar()->tabAt(he->oldPos());
                if (indexNew != indexOld) {
                    if (indexOld != -1)
                        tabBar()->tabButton(indexOld, QTabBar::RightSide)->hide();
                    if (indexNew != -1)
                        tabBar()->tabButton(indexNew, QTabBar::RightSide)->show();
                }
            }
            break;
        }
    }

    return QTabWidget::eventFilter(watched, event);
}

void WizMainTabBrowser::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton)
    {
        event->accept();
        int tab_index = tabBar()->tabAt(event->pos());
        closeTab(tab_index);
    }
    else
        QTabWidget::mousePressEvent(event);
}