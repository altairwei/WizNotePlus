#include "WizMainTabBrowser.h"

#include <stdexcept>

#include <QWidget>
#include <QMessageBox>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOption>
#include <QStyleOptionTabBarBase>
#include <QLabel>
#include <QMenu>
#include <QTabBar>
#include <QLayout>

#include "share/WizWebEngineView.h"
#include "share/WizGlobal.h"
#include "WizDef.h"
#include "share/WizMisc.h"
#include "utils/WizStyleHelper.h"
#include "utils/WizPathResolve.h"
#include "share/WizDatabaseManager.h"
#include "gui/documentviewer/WizTitleBar.h"
#include "gui/documentviewer/WizDocumentView.h"
#include "gui/tabbrowser/WizWebsiteView.h"
#include "gui/tabbrowser/WebEngineWindow.h"

//-------------------------------------------------------------------
// class WizMainTabBrowser
//-------------------------------------------------------------------

TabButton::TabButton(QWidget *parent)
    : QAbstractButton(parent)
{
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::ArrowCursor);
    resize(sizeHint());
    setIconSize(QSize(16, 16));
}

QSize TabButton::sizeHint() const
{
    ensurePolished();
    int width = style()->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, 0, this);
    int height = style()->pixelMetric(QStyle::PM_TabCloseIndicatorHeight, 0, this);
    return QSize(width, height);
}

void TabButton::enterEvent(QEvent *event)
{
    if (isEnabled())
        update();
    QAbstractButton::enterEvent(event);
}

void TabButton::leaveEvent(QEvent *event)
{
    if (isEnabled())
        update();
    QAbstractButton::leaveEvent(event);
}

void TabButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOptionButton opt;
    opt.init(this);
    opt.state |= QStyle::State_AutoRaise;
    if (isEnabled() && underMouse() && !isChecked() && !isDown())
        opt.state |= QStyle::State_Raised;
    if (isChecked())
        opt.state |= QStyle::State_On;
    if (isDown())
        opt.state |= QStyle::State_Sunken;

    if (const QTabBar *tb = qobject_cast<const QTabBar *>(parent())) {
        int index = tb->currentIndex();
        QTabBar::ButtonPosition position = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, tb);
        if (tb->tabButton(index, position) == this)
            opt.state |= QStyle::State_Selected;
    }
    opt.icon = icon();
    opt.iconSize = QSize(16, 16);
    //style()->drawPrimitive(QStyle::PE_IndicatorTabClose, &opt, &p, this);
    drawTabBtn(&opt, &p, this);
}

void TabButton::drawTabBtn(const QStyleOptionButton *opt, QPainter *p, const QWidget *widget) const
{

    /* 应该添加下面几种状态
    if (d->tabBarcloseButtonIcon.isNull()) {
        d->tabBarcloseButtonIcon.addPixmap(QPixmap(
                    QLatin1String(":/qt-project.org/styles/commonstyle/images/standardbutton-closetab-16.png")),
                    QIcon::Normal, QIcon::Off);
        d->tabBarcloseButtonIcon.addPixmap(QPixmap(
                    QLatin1String(":/qt-project.org/styles/commonstyle/images/standardbutton-closetab-down-16.png")),
                    QIcon::Normal, QIcon::On);
        d->tabBarcloseButtonIcon.addPixmap(QPixmap(
                    QLatin1String(":/qt-project.org/styles/commonstyle/images/standardbutton-closetab-hover-16.png")),
                    QIcon::Active, QIcon::Off);
    }
    */
    //int size = style()->pixelMetric(QStyle::PM_SmallIconSize);
    QIcon::Mode mode = opt->state & QStyle::State_Enabled ?
                        (opt->state & QStyle::State_Raised ? QIcon::Active : QIcon::Normal)
                        : QIcon::Disabled;
    if (!(opt->state & QStyle::State_Raised)
        && !(opt->state & QStyle::State_Sunken)
        && !(opt->state & QStyle::State_Selected))
        mode = QIcon::Disabled;
    //

    QIcon::State state = opt->state & QStyle::State_Sunken ? QIcon::On : QIcon::Off;
    QPixmap pixmap = opt->icon.pixmap(opt->iconSize, mode, state);
    style()->drawItemPixmap(p, opt->rect, Qt::AlignCenter, pixmap);
}

//-------------------------------------------------------------------
// class WizMainTabBrowser
//-------------------------------------------------------------------

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

    /* 如果要让标签栏下移，得设置整个QTabWidget布局，比如加个 spacer；
       如果想要让documentMode状态下的标签栏右移，但底线会一同右移动；
       看来这个底线是QTabBar的而非documentMode下，这些底线是tab widget frame。*/
    setStyleSheet("QTabBar::tab { max-width: 300px; }");
    setDocumentMode(true); // 不渲染tab widget frame
    setElideMode(Qt::ElideRight);

    // This signal is emitted whenever the current page index changes.
    connect(this, &QTabWidget::currentChanged, this, &WizMainTabBrowser::handleCurrentChanged);

    connect(tabBar, &QTabBar::customContextMenuRequested, this, &WizMainTabBrowser::handleContextMenuRequested);
    connect(tabBar, &QTabBar::tabCloseRequested, this, &WizMainTabBrowser::handleTabCloseRequested);
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
    TabStatusData status = tabBar()->tabData(index).toMap();
    bool isLocked = status["Locked"].toBool();
    // ensure click pos is in tab not tabbar
    if (index != -1) {
        // close actions
        QAction *action = menu.addAction(tr("Close Tab"));
        connect(action, &QAction::triggered, this, [this, index]() {
            tabPage(index)->RequestClose();
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
    }
    menu.exec(QCursor::pos());
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
    addTab(websiteView, tr("Untitled"));
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
    setupTabPage(tabPage);
    // Workaround for QTBUG-61770
    tabPage->resize(currentWidget()->size());
    setCurrentWidget(tabPage);

    return index;
}

void WizMainTabBrowser::handleTabCloseRequested(int index)
{
    tabPage(index)->RequestClose();
}

/**
 * @brief 处理标签栏发出的关闭信号
 * @param index 标签编号
 */
void WizMainTabBrowser::closeTab(int index)
{
    auto p = tabPage(index);
    removeTab(index);
    p->deleteLater();
}

void WizMainTabBrowser::closeOtherTabs(int index)
{
    for (int i = count() - 1; i > index; --i)
        tabPage(i)->RequestClose();
    for (int i = index - 1; i >= 0; --i)
        tabPage(i)->RequestClose();
}

void WizMainTabBrowser::closeAllTabs()
{
    for (int i = count() - 1; i >= 0; --i)
        tabPage(i)->RequestClose();
}

void WizMainTabBrowser::closeLeftTabs(int index)
{
    for (int i = index - 1; i >= 0; --i)
        tabPage(i)->RequestClose();
}

void WizMainTabBrowser::closeRightTabs(int index)
{
    for (int i = count() - 1; i > index; --i)
        tabPage(i)->RequestClose();
}

void WizMainTabBrowser::lockTab(int index)
{
    if (index != -1) {
        TabStatusData status = tabBar()->tabData(index).toMap();
        QWidget* tb = tabBar()->tabButton(index, QTabBar::RightSide);
        TabButton* tabBtn = qobject_cast<TabButton*>(tb);
        if (tabBtn) {
            tabBtn->setIcon(WizLoadSkinIcon(m_strTheme, "tab_lock", QSize(16, 16)));
            tabBtn->disconnect();
            status["Locked"] = QVariant(true);
            tabBar()->setTabData(index, status);
        }
    }
}

void WizMainTabBrowser::unlockTab(int index)
{
    if (index != -1) {
        TabStatusData status = tabBar()->tabData(index).toMap();
        QWidget* tb = tabBar()->tabButton(index, QTabBar::RightSide);
        TabButton* tabBtn = qobject_cast<TabButton*>(tb);
        if (tabBtn) {
            tabBtn->setIcon(WizLoadSkinIcon(m_strTheme, "tab_close", QSize(16, 16)));
            connect(tabBtn, &QAbstractButton::clicked, this, [this, index](){
                emit this->tabBar()->tabCloseRequested(index);
            });
            status["Locked"] = QVariant(false);
            tabBar()->setTabData(index, status);
        }
    }

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
            closeTab(index);
        }
    });

    // Close Button
    int index = indexOf(tabPage);
    if (index != -1) {
        TabButton* closeBtn = new TabButton(tabBar());
        closeBtn->setIcon(WizLoadSkinIcon(m_strTheme, "tab_close", QSize(16, 16)));
        connect(closeBtn, &QAbstractButton::clicked, this, [this, tabPage](){
            tabPage->RequestClose();
        });
        tabBar()->setTabButton(index, QTabBar::RightSide, closeBtn);
        //
        TabStatusData status;
        status["Locked"] = QVariant(false);
        tabBar()->setTabData(index, status);
    }
}
