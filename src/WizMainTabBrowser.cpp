#include "WizMainTabBrowser.h"

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
#include "WizTitleBar.h"
#include "WizDocumentView.h"
#include "WizWebsiteView.h"

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
    // 标签栏设置
    QTabBar *tabBar = this->tabBar();
    tabBar->setTabsClosable(false);
    tabBar->setSelectionBehaviorOnRemove(QTabBar::SelectPreviousTab);
    tabBar->setMovable(true);
    tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
    // 设置样式
    setStyleSheet("QTabBar::tab { max-width: 300px; }");
    // 如果要让标签栏下移，得设置整个QTabWidget布局，比如价格 spacer
    // 如果想要让documentMode状态下的标签栏右移，但底线会一同右移动，看来这个底线是QTabBar的
    // 而非documentMode下，这些底线是tab widget frame。
    //
    connect(tabBar, &QTabBar::customContextMenuRequested,
                    this, &WizMainTabBrowser::handleContextMenuRequested);
    connect(tabBar, &QTabBar::tabCloseRequested, this, &WizMainTabBrowser::closeTab);
    connect(&m_dbMgr, &WizDatabaseManager::documentDeleted, this, &WizMainTabBrowser::on_document_deleted);
    //
    setDocumentMode(true); // 不渲染tab widget frame
    setElideMode(Qt::ElideRight);
    //TabButton* p = new TabButton(this);
    //p->setText("Home");
    //setCornerWidget(p, Qt::TopLeftCorner);
    // 处理标签切换信号
    connect(this, &QTabWidget::currentChanged, this, &WizMainTabBrowser::handleCurrentChanged);
}

void WizMainTabBrowser::handleCurrentChanged(int index)
{
    // index 是新的当前标签
    // 发送各种信号
    // WizMainWindow 的m_doc需要更新
    if (index != -1) {
        WizDocumentWebView* docView = qobject_cast<WizDocumentWebView*>(getWebView(index));
        if (docView) {
            docView->setFocus();
        }
            
        // emit titleChanged(view->title());
        // emit loadProgress(view->loadProgress());
        // emit urlChanged(view->url());
        // emit favIconChanged(view->favIcon());
        // emit webActionEnabledChanged(QWebEnginePage::Back, view->isWebActionEnabled(QWebEnginePage::Back));
        // emit webActionEnabledChanged(QWebEnginePage::Forward, view->isWebActionEnabled(QWebEnginePage::Forward));
        // emit webActionEnabledChanged(QWebEnginePage::Stop, view->isWebActionEnabled(QWebEnginePage::Stop));
        // emit webActionEnabledChanged(QWebEnginePage::Reload,view->isWebActionEnabled(QWebEnginePage::Reload));
    } else {
        // emit titleChanged(QString());
        // emit loadProgress(0);
        // emit urlChanged(QUrl());
        // emit favIconChanged(QIcon());
        // emit webActionEnabledChanged(QWebEnginePage::Back, false);
        // emit webActionEnabledChanged(QWebEnginePage::Forward, false);
        // emit webActionEnabledChanged(QWebEnginePage::Stop, false);
        // emit webActionEnabledChanged(QWebEnginePage::Reload, true);
    }
}

/**
 * @brief 选项卡右键弹出菜单
 * @param pos
 */
void WizMainTabBrowser::handleContextMenuRequested(const QPoint &pos)
{
    QMenu menu;
    int index = tabBar()->tabAt(pos);
    TabStatusData status = tabBar()->tabData(index).toMap();
    bool isLocked = status["Locked"].toBool();
    // ensure click pos is in tab not tabbar
    if (index != -1) {
        // close actions
        QAction *action = menu.addAction(tr("&Close Tab"));
        action->setShortcut(QKeySequence::Close);
        connect(action, &QAction::triggered, this, [this,index]() {
            closeTab(index);
        });
        action = menu.addAction(tr("Close &Other Tabs"));
        connect(action, &QAction::triggered, this, [this,index]() {
            closeOtherTabs(index);
        });
        action = menu.addAction(tr("Close Left Tabs"));
        connect(action, &QAction::triggered, this, [this,index]() {
            closeLeftTabs(index);
        });
        action = menu.addAction(tr("Close Right Tabs"));
        connect(action, &QAction::triggered, this, [this,index]() {
            closeRightTabs(index);
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

/**
 * @brief 处理浏览笔记信号
 * @param view 文档视图
 * @param doc 文档数据
 * @param forceEditing 是否强制编辑
 */
void WizMainTabBrowser::onViewNoteRequested(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool forceEditing)
{
    Q_UNUSED(forceEditing);
    Q_UNUSED(view);
    Q_UNUSED(doc);

}

/**
 * @brief Remove related tab page when recieve document deletion signal.
 */
void WizMainTabBrowser::on_document_deleted(const WIZDOCUMENTDATA& data)
{
    for (int i = 0; i < count(); ++i) {
        WizDocumentView* docView = qobject_cast<WizDocumentView*>(widget(i));
        if ( docView == nullptr ) {
            continue;
        } else {
            QString noteGUID = data.strGUID;
            if (noteGUID == docView->note().strGUID)
                closeTab(i);
        }

    }
}

void WizMainTabBrowser::setupTab(QWidget *wgt)
{
    int index = indexOf(wgt);
    if (index != -1) {
        TabButton* closeBtn = new TabButton(this->tabBar());
        closeBtn->setIcon(WizLoadSkinIcon(m_strTheme, "tab_close", QSize(16, 16)));
        connect(closeBtn, &QAbstractButton::clicked, this, [this, wgt](){
            int currentIndex = indexOf(wgt);
            emit this->tabBar()->tabCloseRequested(currentIndex);
        });
        tabBar()->setTabButton(index, QTabBar::RightSide, closeBtn);
        //
        TabStatusData status;
        status["Locked"] = QVariant(false);
        tabBar()->setTabData(index, status);

    }
}

/**
 * @brief 浏览笔记文档
 * @param docView 已经构建好的文档视图
 */
int WizMainTabBrowser::createTab(WizDocumentView *docView)
{
    // 创建标签页
    int index = addTab(docView, docView->note().strTitle);
    setupTab(docView);
    setupDocView(docView);
    docView->resize(currentWidget()->size()); // Workaround for QTBUG-61770
    // 设置成当前部件
    setCurrentWidget(docView);
    //
    return index;
}

/**
 * @brief 通过地址来浏览页面
 * @param url 要浏览的地址，可以使本地文件地址;
 */
int WizMainTabBrowser::createTab(const QUrl &url)
{
    // 创建网页视图组件
    WizWebsiteView* websiteView = new WizWebsiteView(m_app);
    setupWebsiteView(websiteView);
    // 创建标签页
    int index = addTab(websiteView, tr("Untitled"));
    setupTab(websiteView);
    websiteView->getWebView()->resize(currentWidget()->size()); // Workaround for QTBUG-61770
    // 设置地址并浏览
    websiteView->viewHtml(url);
    websiteView->getWebView()->setFocus();
    // 设置成当前部件
    setCurrentWidget(websiteView);
    //
    return index;
}

/**
 * @brief 处理标签栏发出的关闭信号
 * @param index 标签编号
 */
void WizMainTabBrowser::closeTab(int index)
{
    // process document view closing.
    QWidget* p = widget(index);
    removeTab(index);
    WizDocumentView* docView = qobject_cast<WizDocumentView*>(p);
    if (docView) {
        //
        docView->waitForDone();
    }
    //
    p->deleteLater();
}

void WizMainTabBrowser::closeOtherTabs(int index)
{
    for (int i = count() - 1; i > index; --i)
        closeTab(i);
    for (int i = index - 1; i >= 0; --i)
        closeTab(i);
}

void WizMainTabBrowser::closeLeftTabs(int index)
{
    for (int i = index - 1; i >= 0; --i)
        closeTab(i);
}

void WizMainTabBrowser::closeRightTabs(int index)
{
    for (int i = count() - 1; i > index; --i)
        closeTab(i);
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

/**
 * @brief 初始化WizWebEngineView
 * @param view
 */
void WizMainTabBrowser::setupView(WizWebEngineView* view) {
    connect(view, &WizWebEngineView::viewSourceRequested, [=](QUrl url, QString title){
        int index = createTab("view-source:" + url.url());
        WizWebsiteView* webView = qobject_cast<WizWebsiteView*>(widget(index));
        if (webView) {
            // To avoid tabtext being changed when users view source of WizDocumentWebPage.
            disconnect(webView->getWebView(), &WizWebEngineView::titleChanged, nullptr, nullptr);
        }
        setTabText(index, "view-source:" + title);
    });
}

/**
 * @brief 初始化文档阅读编辑器
 * @param docView
 */
void WizMainTabBrowser::setupDocView(WizDocumentView *docView) {
    // set tab text to document title
    connect(WizGlobal::instance(), &WizGlobal::viewNoteRequested, [this](WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool forceEditing){
        Q_UNUSED(forceEditing);
        int index = indexOf(view);
        if (index != -1) {
            setTabText(index, doc.strTitle);
        }
    });
    connect(docView, &WizDocumentView::documentSaved, [this](QString strGUID, WizDocumentView* view){
        Q_UNUSED(strGUID);
        int index = indexOf(view);
        auto doc = view->note();
        if (index != -1) {
            setTabText(index, doc.strTitle);
        }
    });
    connect(docView->web(), &WizDocumentWebView::titleEdited, [this](WizDocumentView* view, QString newTitle){
        int index = indexOf(view);
        if (index != -1) {
            setTabText(index, newTitle);
        }
    });
    setupView(docView->web());
}

/**
 * @brief 初始化网页浏览器
 * @param webView 要初始化的页面视图;
 */
void WizMainTabBrowser::setupWebsiteView(WizWebsiteView *websiteView)
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
    setupView(webView);
}

void WizMainTabBrowser::paintEvent(QPaintEvent *)
{
    //Q_D(QTabWidget);
    // 是否处于文档浏览模式
    if (documentMode()) {
        QStylePainter p(this, tabBar());
        // 为左上角部件留出空间
        if (QWidget *w = cornerWidget(Qt::TopLeftCorner)) {
            QStyleOptionTabBarBase opt;
            initStyleBaseOption(&opt, tabBar(), w->size());
            opt.rect.moveLeft(w->x() + opt.rect.x());
            opt.rect.moveTop(w->y() + opt.rect.y());
            p.drawPrimitive(QStyle::PE_FrameTabBarBase, opt);
        }
        // 为右上角部件空间
        if (QWidget *w = cornerWidget(Qt::TopRightCorner)) {
            QStyleOptionTabBarBase opt;
            initStyleBaseOption(&opt, tabBar(), w->size());
            opt.rect.moveLeft(w->x() + opt.rect.x());
            opt.rect.moveTop(w->y() + opt.rect.y());
            p.drawPrimitive(QStyle::PE_FrameTabBarBase, opt);
        }
        return;
    }
    QStylePainter p(this);

    QStyleOptionTabWidgetFrame opt;
    initStyleOption(&opt);
    opt.rect = style()->subElementRect(QStyle::SE_TabWidgetTabPane, &opt, this);;
    p.drawPrimitive(QStyle::PE_FrameTabWidget, opt);
}

void WizMainTabBrowser::initStyleBaseOption(QStyleOptionTabBarBase *optTabBase, QTabBar *tabbar, QSize size)
{
    QStyleOptionTab tabOverlap;
    tabOverlap.shape = tabbar->shape();
    int overlap = tabbar->style()->pixelMetric(QStyle::PM_TabBarBaseOverlap, &tabOverlap, tabbar);
    QWidget *theParent = tabbar->parentWidget();
    optTabBase->init(tabbar);
    optTabBase->shape = tabbar->shape();
    optTabBase->documentMode = tabbar->documentMode();
    if (theParent && overlap > 0) {
        QRect rect;
        switch (tabOverlap.shape) {
        case QTabBar::RoundedNorth:
        case QTabBar::TriangularNorth:
            rect.setRect(0, size.height()-overlap, size.width(), overlap);
            break;
        case QTabBar::RoundedSouth:
        case QTabBar::TriangularSouth:
            rect.setRect(0, 0, size.width(), overlap);
            break;
        case QTabBar::RoundedEast:
        case QTabBar::TriangularEast:
            rect.setRect(0, 0, overlap, size.height());
            break;
        case QTabBar::RoundedWest:
        case QTabBar::TriangularWest:
            rect.setRect(size.width() - overlap, 0, overlap, size.height());
            break;
        }
        optTabBase->rect = rect;
    }
}
