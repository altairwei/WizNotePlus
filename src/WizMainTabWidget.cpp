#include "WizMainTabWidget.h"

#include <QLabel>
#include <QMenu>
#include <QTabBar>

#include "share/WizWebEngineView.h"
#include "WizDocumentView.h"
#include "share/WizGlobal.h"

WizMainTabWidget::WizMainTabWidget(QWidget *parent)
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
    connect(tabBar, &QTabBar::tabCloseRequested, this, &WizMainTabWidget::closeTab);
    //
    setDocumentMode(true); // 不渲染tab widget frame
    setElideMode(Qt::ElideRight);
    // 处理标签切换信号
    //connect(this, &QTabWidget::currentChanged, this, &TabWidget::handleCurrentChanged);
}

void WizMainTabWidget::handleCurrentChanged(int index)
{
    // index 是新的当前标签
    // 发送各种信号
}

/**
 * @brief 处理浏览笔记信号
 * @param view 文档视图
 * @param doc 文档数据
 * @param forceEditing 是否强制编辑
 */
void WizMainTabWidget::onViewNoteRequested(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool forceEditing)
{

}

/**
 * @brief 浏览笔记文档
 * @param docView 已经构建好的文档视图
 */
void WizMainTabWidget::createTab(WizDocumentView *docView)
{
    // 创建标签页
    addTab(docView, docView->note().strTitle);
    docView->resize(currentWidget()->size()); // Workaround for QTBUG-61770
    // 设置标签标题，此处应该检测文档视图标题变化
    //setTabText(index, docView->note().strTitle); // 此时笔记还没加载
    connect(WizGlobal::instance(), &WizGlobal::viewNoteRequested, [this](WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool forceEditing) {
        int index = indexOf(view);
        if (index != -1) {
            setTabText(index, doc.strTitle);
        }
    });
    // 设置成当前部件
    setCurrentWidget(docView);
}

/**
 * @brief 通过地址来浏览页面
 * @param url 要浏览的地址，可以使本地文件地址;
 */
void WizMainTabWidget::createTab(const QUrl &url)
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

/**
 * @brief 处理标签栏发出的关闭信号
 * @param index 标签编号
 */
void WizMainTabWidget::closeTab(int index)
{
    removeTab(index);
    /*
    if (WizWebEngineView *view = getWebView(index))
    {
        bool hasFocus = view->hasFocus();

        if (hasFocus && count() > 0)
            currentWebView()->setFocus();
        if (count() == 0)
            createTab(QUrl("http://www.wiz.cn/"));
        view->deleteLater();
    }
    */
}

/**
 * @brief 从标签页中获得页面视图
 * @param index 标签序号
 * @return
 */
WizWebEngineView* WizMainTabWidget::getWebView(int index) const
{
    // 判断是文档视图还是页面视图
    WizDocumentView* docView = qobject_cast<WizDocumentView*>(widget(index));
    WizWebEngineView* webView = qobject_cast<WizWebEngineView*>(widget(index));
    if ( webView != nullptr )
        return webView;
    if (docView != nullptr )
    {
        WizWebEngineView* webView;
        webView = docView->web();
        return webView;
    }
}

/**
 * @brief 获取当前页面视图
 * @return
 */
WizWebEngineView* WizMainTabWidget::currentWebView() const
{
    return getWebView(currentIndex());
}

/**
 * @brief 初始化页面视图，绑定或激发信号
 * @param webView 要初始化的页面视图;
 */
void WizMainTabWidget::setupView(WizWebEngineView *webView)
{
    WizWebEnginePage *webPage = webView->getPage();

    connect(webView, &WizWebEngineView::titleChanged, [this, webView](const QString &title) {
        int index = indexOf(webView);
        if (index != -1) {
            setTabText(index, title);
            setTabToolTip(index, title);
        }
        if (currentIndex() == index)
            emit titleChanged(title);
    });
    connect(webView, &WizWebEngineView::urlChanged, [this, webView](const QUrl &url) {
        int index = indexOf(webView);
        if (index != -1)
            tabBar()->setTabData(index, url);
        if (currentIndex() == index)
            emit urlChanged(url);
    });
    connect(webView, &WizWebEngineView::loadProgress, [this, webView](int progress) {
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

    connect(webPage, &WizWebEnginePage::windowCloseRequested, [this, webView]() {
        int index = indexOf(webView);
        if (index >= 0)
            closeTab(index);
    });
    */
    //connect(webView, &WizWebEngineView::devToolsRequested, this, &TabWidget::devToolsRequested);
}
