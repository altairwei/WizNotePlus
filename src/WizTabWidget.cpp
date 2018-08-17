#include "WizTabWidget.h"
#include "share/WizWebEngineView.h"
#include "WizDocumentView.h"
#include <QLabel>
#include <QMenu>
#include <QTabBar>

WizTabWidget::WizTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    // 标签栏设置
    QTabBar *tabBar = this->tabBar();
    tabBar->setTabsClosable(true);
    tabBar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    tabBar->setMovable(true);
    tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
    //
    //connect(tabBar, &QTabBar::customContextMenuRequested,
                    //this, &TabWidget::handleContextMenuRequested); // 右键菜单栏设置
    connect(tabBar, &QTabBar::tabCloseRequested, this, &TabWidget::closeTab);
    //
    setDocumentMode(true); // 不渲染tab widget frame
    setElideMode(Qt::ElideRight);
    // 处理标签切换信号
    //connect(this, &QTabWidget::currentChanged, this, &TabWidget::handleCurrentChanged);
}

void WizTabWidget::handleCurrentChanged(int index)
{
    // index 是新的当前标签
}

/** 浏览笔记文档
 *
 *  @param docView 已经构建好的文档视图
 */
void WizTabWidget::createTab(WizDocumentView *docView)
{
    // 创建标签页
    addTab(docView, docView->note().strTitle);
    webView->resize(currentWidget()->size()); // Workaround for QTBUG-61770
    // 设置成当前部件
    setCurrentWidget(docView);
}

/** 通过地址来浏览页面
 *
 *  @param url 要浏览的地址，可以使本地文件地址;
 */
void WizTabWidget::createTab(const QUrl &url)
{
    // 创建web页面
    WizWebEngineView* webView = new WizWebEngineView(nullptr); // 注意是否有内存泄漏
    WizWebEnginePage* webPage = new WizWebEnginePage(webView);
    webView->setPage(webPage);
    setupView(webView);
    // 创建标签页
    addTab(webView, tr("Untitled"));
    webView->resize(currentWidget()->size()); // Workaround for QTBUG-61770
    // 设置地址并浏览
    webView->setUrl(url);
    webView->setFocus();
    // 设置成当前部件
    webView->show();
    setCurrentWidget(webView);
}

/** 初始化页面视图，绑定或激发信号
 *
 *  @param webView 要初始化的页面视图;
 */
void WizTabWidget::setupView(WizWebEngineView *webView)
{
    WizWebEnginePage *webPage = webView->getPage();

    connect(webView, &WizWebEnginePage::titleChanged, [this, webView](const QString &title) {
        int index = indexOf(webView);
        if (index != -1) {
            setTabText(index, title);
            setTabToolTip(index, title);
        }
        if (currentIndex() == index)
            emit titleChanged(title);
    });
    connect(webView, &WizWebEnginePage::urlChanged, [this, webView](const QUrl &url) {
        int index = indexOf(webView);
        if (index != -1)
            tabBar()->setTabData(index, url);
        if (currentIndex() == index)
            emit urlChanged(url);
    });
    connect(webView, &WizWebEnginePage::loadProgress, [this, webView](int progress) {
        if (currentIndex() == indexOf(webView))
            emit loadProgress(progress);
    });
    connect(webPage, &WizWebEnginePage::linkHovered, [this, webView](const QString &url) {
        if (currentIndex() == indexOf(webView))
            emit linkHovered(url);
    });
    /*
    connect(webView, &WizWebEngineView::favIconChanged, [this, webView](const QIcon &icon) {
        int index = indexOf(webView);
        if (index != -1)
            setTabIcon(index, icon);
        if (currentIndex() == index)
            emit favIconChanged(icon);
    });

    connect(webView, &WizWebEngineView::webActionEnabledChanged, [this, webView](QWebEnginePage::WebAction action, bool enabled) {
        if (currentIndex() ==  indexOf(webView))
            emit webActionEnabledChanged(action,enabled);
    });
    */
    connect(webPage, &WizWebEnginePage::windowCloseRequested, [this, webView]() {
        int index = indexOf(webView);
        if (index >= 0)
            closeTab(index);
    });
    //connect(webView, &WizWebEngineView::devToolsRequested, this, &TabWidget::devToolsRequested);
}
