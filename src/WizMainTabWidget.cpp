#include "WizMainTabWidget.h"

#include <QLabel>
#include <QMenu>
#include <QTabBar>
#include <QLayout>

#include "share/WizWebEngineView.h"
#include "WizDocumentView.h"
#include "share/WizGlobal.h"
#include "WizTitleBar.h"
#include "WizWebsiteView.h"

WizMainTabWidget::WizMainTabWidget(WizExplorerApp& app, QWidget *parent)
    : QTabWidget(parent)
    , m_app(app)
{
    // 标签栏设置
    QTabBar *tabBar = this->tabBar();
    tabBar->setTabsClosable(true);
    tabBar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    tabBar->setMovable(true);
    tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
    // 设置样式
    setStyleSheet("QTabBar::tab { max-width: 300px; }");
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
    // WizMainWindow 的m_doc需要更新
    Q_UNUSED(index);
}

/**
 * @brief 处理浏览笔记信号
 * @param view 文档视图
 * @param doc 文档数据
 * @param forceEditing 是否强制编辑
 */
void WizMainTabWidget::onViewNoteRequested(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool forceEditing)
{
    Q_UNUSED(forceEditing);
    Q_UNUSED(view);
    Q_UNUSED(doc);

}

/**
 * @brief 处理viewNoteRequested信号，设置标签文本为笔记标题
 * @param view
 * @param doc
 * @param forceEditing
 */
void WizMainTabWidget::setTabTextToDocumentTitle(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool forceEditing)
{
    Q_UNUSED(forceEditing);
    int index = indexOf(view);
    if (index != -1) {
        setTabText(index, doc.strTitle);
    }
}

/**
 * @brief 处理documentSaved信号，设置标签文本为笔记标题
 * @param strGUID
 * @param view
 */
void WizMainTabWidget::setTabTextToDocumentTitle(QString strGUID, WizDocumentView* view)
{
    Q_UNUSED(strGUID);
    int index = indexOf(view);
    auto doc = view->note();
    if (index != -1) {
        setTabText(index, doc.strTitle);
    }
}

/**
 * @brief 处理titleEdited信号，设置标签文本为笔记标题
 * @param newTitle 新的标题
 */
void WizMainTabWidget::setTabTextToDocumentTitle(WizDocumentView* view, QString newTitle)
{
    int index = indexOf(view);
    if (index != -1) {
        setTabText(index, newTitle);
    }
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
    connect(WizGlobal::instance(), SIGNAL(viewNoteRequested(WizDocumentView*, const WIZDOCUMENTDATAEX&, bool)),
            SLOT(setTabTextToDocumentTitle(WizDocumentView*, const WIZDOCUMENTDATAEX&, bool)));
    connect(docView, SIGNAL(documentSaved(QString, WizDocumentView*)),
            SLOT(setTabTextToDocumentTitle(QString, WizDocumentView*)));
    connect(docView->web(), SIGNAL(titleEdited(WizDocumentView*, QString)), SLOT(setTabTextToDocumentTitle(WizDocumentView*, QString)));
    // 设置成当前部件
    setCurrentWidget(docView);
}

/**
 * @brief 通过地址来浏览页面
 * @param url 要浏览的地址，可以使本地文件地址;
 */
void WizMainTabWidget::createTab(const QUrl &url)
{
    // 创建网页视图组件
    WizWebsiteView* websiteView = new WizWebsiteView(m_app);
    setupView(websiteView);
    // 创建标签页
    addTab(websiteView, tr("Untitled"));
    websiteView->getWebView()->resize(currentWidget()->size()); // Workaround for QTBUG-61770
    // 设置地址并浏览
    websiteView->viewHtml(url);
    websiteView->getWebView()->setFocus();
    // 设置成当前部件
    setCurrentWidget(websiteView);
}

/**
 * @brief 处理标签栏发出的关闭信号
 * @param index 标签编号
 */
void WizMainTabWidget::closeTab(int index)
{
    // 每个文档视图关闭时都要执行waitForDone();
    WizDocumentView* docView = qobject_cast<WizDocumentView*>(widget(index));
    if (docView) docView->waitForDone();
    //
    removeTab(index);
    /*
    // 关闭最后一个网页后自动打开一个页面
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
WizWebEngineView* WizMainTabWidget::currentWebView() const
{
    return getWebView(currentIndex());
}

/**
 * @brief 初始化页面视图，绑定或激发信号
 * @param webView 要初始化的页面视图;
 */
void WizMainTabWidget::setupView(WizWebsiteView *websiteView)
{
    WizWebEngineView *webView = websiteView->getWebView();
    WizWebEnginePage *webPage = webView->getPage();

    connect(webView, &WizWebEngineView::titleChanged, [this, websiteView](const QString &title) {
        int index = indexOf(websiteView);
        if (index != -1) {
            setTabText(index, title);
            setTabToolTip(index, title);
        }
        if (currentIndex() == index)
            emit titleChanged(title);
    });
    connect(webView, &WizWebEngineView::urlChanged, [this, websiteView](const QUrl &url) {
        int index = indexOf(websiteView);
        if (index != -1)
            tabBar()->setTabData(index, url);
        if (currentIndex() == index)
            emit urlChanged(url);
    });
    connect(webView, &WizWebEngineView::loadProgress, [this, websiteView](int progress) {
        if (currentIndex() == indexOf(websiteView))
            emit loadProgress(progress);
    });
    connect(webPage, &WizWebEnginePage::linkHovered, [this, websiteView](const QString &url) {
        if (currentIndex() == indexOf(websiteView))
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
