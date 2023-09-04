#include "WizMainWindow.h"

#include <algorithm>
#include <typeinfo>
#include <QToolBar>
#include <QMenuBar>
#include <QApplication>
#include <QUndoStack>
#include <QEvent>
#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QFileDialog>
#include <QPushButton>
#include <QHostInfo>
#include <QSystemTrayIcon>
#include <QPrintDialog>
#include <QPrinter>
#include <QCheckBox>
#include <QProgressDialog>

#ifdef Q_OS_MAC
#include <Carbon/Carbon.h>
#include "mac/WizMacHelper.h"
#else
#endif
#include "WizSearchWidget.h"
#include "share/WizMessageBox.h"
#include "widgets/WizTrayIcon.h"

#include "share/WizGlobal.h"
#include "widgets/WizAboutDialog.h"

#include "WizUpgrade.h"
#include "WizConsoleDialog.h"
#include "gui/categoryviewer/WizCategoryView.h"
#include "gui/doclistviewer/WizDocumentListView.h"
#include "WizUserCipherForm.h"

#include "WizActions.h"
#include "WizPreferenceDialog.h"
#include "WizUpgradeNotifyDialog.h"
#include "core/WizNoteManager.h"
#include "share/WizCommonUI.h"
#include "share/WizUI.h"
#include "share/WizMisc.h"
#include "share/WizUIHelper.h"
#include "share/WizSettings.h"
#include "share/WizAnimateAction.h"
#include "share/WizSearch.h"
#include "share/WizObjectDataDownloader.h"
#include "utils/WizPathResolve.h"
#include "utils/WizStyleHelper.h"
#include "utils/WizMisc.h"
#include "utils/WizPinyin.h"
#include "utils/WizLogger.h"
#include "widgets/WizFramelessWebDialog.h"
#include "widgets/WizScreenShotWidget.h"
#include "widgets/WizImageButton.h"
#include "widgets/WizIAPDialog.h"
#include "widgets/WizLocalProgressWebView.h"
#include "widgets/WizTemplatePurchaseDialog.h"
#include "widgets/WizCodeEditorDialog.h"
#include "widgets/DownloadManagerWidget.h"

#include "WizNoteStyle.h"
#include "WizDocumentHistory.h"
#include "WizButton.h"

#include "WizProgressDialog.h"
#include "WizDocumentSelectionView.h"
#include "WizDocumentTransitionView.h"
#include "WizMessageListView.h"
#include "WizPopupButton.h"

#include "sync/WizApiEntry.h"
#include "sync/WizKMSync.h"
#include "sync/WizAvatarHost.h"
#include "sync/WizToken.h"

#include "WizUserVerifyDialog.h"
#include "WizMobileFileReceiver.h"
#include "WizDocTemplateDialog.h"
#include "share/WizFileMonitor.h"
#include "share/WizAnalyzer.h"
#include "share/WizTranslater.h"
#include "share/WizThreads.h"

#include "widgets/WizUserInfoWidget.h"
#include "widgets/WizShareLinkDialog.h"
#include "widgets/WizCustomToolBar.h"
#include "widgets/WizTipsWidget.h"
#include "widgets/WizExecutingActionDialog.h"
#include "widgets/WizUserServiceExprDialog.h"
#include "widgets/FileExportWizard.h"

#include "WizPositionDelegate.h"
#include "core/WizAccountManager.h"
#include "share/WizWebEngineView.h"
#include "share/jsoncpp/json/json.h"
#include "WizCellButton.h"
#include "WizFileImporter.h"

#include "api/ApiWizExplorerApp.h"
#include "api/PublicAPIsServer.h"
#include "jsplugin/JSPluginManager.h"
#include "jsplugin/JSPluginSpec.h"
#include "jsplugin/JSPlugin.h"
#include "jsplugin/JSRepl.h"

#include "gui/tabbrowser/WizWebsiteView.h"
#include "gui/tabbrowser/WizMainTabBrowser.h"

#include "gui/documentviewer/WizDocumentView.h"
#include "gui/documentviewer/WizTitleBar.h"
#include "gui/documentviewer/WizSingleDocumentView.h"
#include "gui/documentviewer/WizDocumentWebView.h"
#include "gui/documentviewer/WizEditorToolBar.h"
#include "gui/documentviewer/WizSvgEditorDialog.h"
#include "gui/documentviewer/CollaborationDocView.h"
#include "gui/documentviewer/DocumentLoaderSaver.h"

#define MAINWINDOW  "MainWindow"


static WizMainWindow* windowInstance = 0;

WizMainWindow::WizMainWindow(WizDatabaseManager& dbMgr, QWidget *parent)
    : _baseClass(parent, true)
    , m_dbMgr(dbMgr)
    , m_progress(new WizProgressDialog(this))
    , m_settings(new WizUserSettings(dbMgr.db()))
    , m_syncQuick(new WizKMSyncThread(dbMgr.db(), true, this))
    , m_syncFull(new WizKMSyncThread(dbMgr.db(), false, this))
    , m_searcher(new WizSearcher(m_dbMgr, this))
    , m_console(nullptr)
    , m_userVerifyDialog(nullptr)
    , m_iapDialog(nullptr)
    , m_templateIAPDialog(nullptr)
    , m_menuBar(nullptr)
    , m_dockMenu(nullptr)
    , m_windowListMenu(nullptr)
#ifndef Q_OS_MAC
    , m_labelNotice(nullptr)
    , m_optionsAction(nullptr)
#endif
    , m_newNoteExtraMenu(nullptr)
    , m_toolBar(new QToolBar("Main", titleBar()))
    , m_menu(new QMenu(clientWidget()))
    , m_spacerForToolButtonAdjust(nullptr)
    , m_actions(new WizActions(*this, this))
    , m_category(new WizCategoryView(*this, this))
    , m_documents(new WizDocumentListView(*this, this))
    , m_noteListWidget(nullptr)
    , m_msgList(new WizMessageListView(dbMgr, this))
    , m_documentSelection(new WizDocumentSelectionView(*this, this))
    //, m_doc(new WizDocumentView(*this)) // 初始化文档视图，就把这个成员当成活动文档视图，QTabWidget说不要指定parent
    , m_mainTabBrowser(new WizMainTabBrowser(*this, this))
    , m_documentPanel(nullptr)
    , m_history(new WizDocumentViewHistory())
    , m_animateSync(new WizAnimateAction(this))
    , m_singleViewMgr(new WizSingleDocumentViewManager(*this, this))
    , m_bRestart(false)
    , m_bLogoutRestart(false)
    , m_bUpdatingSelection(false)
    , m_tray(nullptr)
    , m_trayMenu(nullptr)
    , m_mobileFileReceiver(nullptr)
    , m_bQuickDownloadMessageEnable(false)
    , m_quiting(false)
    , m_IWizExplorerApp(new ApiWizExplorerApp(this, this))
    , m_externalEditorLauncher(new ExternalEditorLauncher(*this, this))
{
#ifdef QT_DEBUG
    int ret = WizToolsSmartCompare("H", "d");
    qDebug() << ret;
#endif

    WizGlobal::setMainWindow(this);
    windowInstance = this;
    qRegisterMetaType<WIZGROUPDATA>("WIZGROUPDATA");

    m_publicAPIsServer = new PublicAPIsServer(
        {{"WizExplorerApp", m_IWizExplorerApp}}, this);

    initSyncQuick();

    initQuitHandler();

    // 多线程设置
    //-------------------------------------------------------------------

    // search and full text search
    initSearcher();

    initSyncFull();

    connect(m_documents, SIGNAL(addDocumentToShortcutsRequest(WIZDOCUMENTDATA)),
            m_category, SLOT(addDocumentToShortcuts(WIZDOCUMENTDATA)));
    //
    connect(&m_dbMgr, SIGNAL(favoritesChanged(QString)), m_category,
            SLOT(on_shortcutDataChanged(QString)));
#if QT_VERSION > 0x050400
    connect(&m_dbMgr, &WizDatabaseManager::userIdChanged, [](const QString& oldId, const QString& newId){
        WizAvatarHost::deleteAvatar(oldId);
        WizAvatarHost::load(oldId, false);
        WizAvatarHost::load(newId, false);
    });
#endif

    // 初始化GUI
    //-------------------------------------------------------------------

    // 根据列表来初始化所有动作
    initActions();

    // 初始化菜单
#ifdef Q_OS_MAC
    initMenuBar();
    initDockMenu();
#else
    initMenuList();
#endif

    initToolBar(); // 主菜单工具栏上的组件: <用户信息, 搜索栏...>
    initClient(); // 主界面容器组件: <文件夹树, 笔记列表, 多标签浏览...>

    setWindowTitle(tr("WizNotePlus"));

    restoreStatus(); // 恢复上一次窗口设置

    // 检查更新、同步
    //-------------------------------------------------------------------

    // upgrade check
#ifndef BUILD4APPSTORE
    if (userSettings().autoCheckUpdate())
    {
        checkWizUpdate();
    }
#endif

#ifdef Q_OS_MAC
    setupFullScreenMode(this);
#endif

    // 开启同步进程
    m_syncFull->start(QThread::IdlePriority);
    m_syncQuick->start(QThread::IdlePriority);
    // 设置系统托盘图标
    setSystemTrayIconVisible(userSettings().showSystemTrayIcon());
    // 设置接收手机文件传输
    setMobileFileReceiverEnable(userSettings().receiveMobileFile());
    // 开始新特征指南
    if (needShowNewFeatureGuide())
    {
        m_settings->setNewFeatureGuideVersion(WIZ_NEW_FEATURE_GUIDE_VERSION);
        QTimer::singleShot(3000, this, SLOT(showNewFeatureGuide()));
    }

    // 检查提醒消息
    if (dbMgr.db().hasBiz())
    {
        QTimer* syncMessageTimer = new QTimer(this);
        connect(syncMessageTimer, SIGNAL(timeout()), m_syncFull, SLOT(quickDownloadMesages()));
        syncMessageTimer->setInterval(3 * 1000 * 60);
        syncMessageTimer->start(3 * 1000 * 60);
    }

    connect(Utils::WizLogger::logger(), &Utils::WizLogger::notifyRequested,
            this, &WizMainWindow::showBubbleNotification);

    m_docSaver = new WizDocumentSaverThread(dbMgr, this);
    m_docLoader = new WizDocumentLoaderThread(dbMgr, this);

    insertScrollbarStyleSheet(QWebEngineProfile::defaultProfile());
    connect(QWebEngineProfile::defaultProfile(), &QWebEngineProfile::downloadRequested,
            &DownloadManagerWidget::instance(), &DownloadManagerWidget::downloadRequested);

    auto &jsmgr = JSPluginManager::instance();
    jsmgr.setMainWindow(this);
}


bool WizMainWindow::eventFilter(QObject* watched, QEvent* event)
{
    // Qt issue: issue? User quit for mac dock send close event to qApp?
    // I was throught close event only send to widget.
    if (watched == qApp) {
        if (event->type() == QEvent::Close)
        {
            qApp->quit();
            return true;

        }
        else if (event->type() == QEvent::FileOpen)
        {
            if (QFileOpenEvent* fileEvent = dynamic_cast<QFileOpenEvent*>(event))
            {
                if (!fileEvent->url().isEmpty())
                {
                    QString strUrl = fileEvent->url().toString();
                    if (strUrl.left(5) == "file:")
                    {
                        strUrl.remove(0, 5);
                        strUrl.replace("open_document%3F", "open_document?");
                    }

                    if (IsWizKMURL(strUrl))
                    {
                        viewDocumentByWizKMURL(strUrl);
                        return true;
                    }
                }
            }
        }
        else if (event->type() == QEvent::ApplicationActivate)
        {
            // It will cause bugs to full screen mode on Linux/KDE. 
            // When WizMainWindow has been hidden, it will show again 
            // if any other windows/dialogs is activated.
#ifdef Q_OS_MAC
                return processApplicationActiveEvent();
#endif
        }
        else
        {
            return false;
        }
    }

    //
    return _baseClass::eventFilter(watched, event);
}

void WizMainWindow::on_application_aboutToQuit()
{
    cleanOnQuit();
}

/**
 * @brief 退出前的同步工作
 */
void WizMainWindow::cleanOnQuit()
{
    m_quiting = true;

    int i = 0;
    m_quitProgress = new QProgressDialog(this);
    m_quitProgress->setWindowTitle(tr("Cleaning on Quit"));
    m_quitProgress->setCancelButtonText(tr("Quit"));
    m_quitProgress->setWindowModality(Qt::WindowModal);
    m_quitProgress->setFixedWidth(400);
    m_quitProgress->setMinimum(0);
    m_quitProgress->setMaximum(11);
    m_quitProgress->setValue(0);

    connect(m_quitProgress, &QProgressDialog::canceled, [this] {
        m_syncQuick->quit();
        m_syncFull->quit();
    });

    // 处理所有标签
    m_quitProgress->setLabelText(tr("Closing opened documents..."));
    processAllDocumentViews([=](WizDocumentView* docView){
        docView->waitForDone();
    });
    m_quitProgress->setValue(++i);

    m_quitProgress->setLabelText(tr("Shutting object downloader host..."));
    WizObjectDownloaderHost::instance()->waitForDone();
    m_quitProgress->setValue(++i);

    m_quitProgress->setLabelText(tr("Saving category tree expand state..."));
    m_category->saveExpandState();
    saveStatus();
    m_quitProgress->setValue(++i);

    m_quitProgress->setLabelText(tr("Shutting full sync thread..."));
    m_syncFull->waitForDone();
    m_quitProgress->setValue(++i);

    m_quitProgress->setLabelText(tr("Shutting quick sync thread..."));
    WizKMSyncThread::setQuickThread(nullptr);
    m_syncQuick->waitForDone();
    m_quitProgress->setValue(++i);

    m_quitProgress->setLabelText(tr("Shutting search thread..."));
    m_searcher->waitForDone();
    m_quitProgress->setValue(++i);

    m_quitProgress->setLabelText(tr("Shutting doc loader thread..."));
    if (m_docLoader) {
        m_docLoader->waitForDone();
        m_docLoader = nullptr;
    }
    m_quitProgress->setValue(++i);

    m_quitProgress->setLabelText(tr("Shutting doc saver thread..."));
    if (m_docSaver) {
        m_docSaver->waitForDone();
        m_docSaver = nullptr;
    }
    m_quitProgress->setValue(++i);

    m_quitProgress->setLabelText(tr("Shutting external editor launcher thread..."));
    m_externalEditorLauncher->waitForDone();
    m_quitProgress->setValue(++i);

    m_quitProgress->setLabelText(tr("Shutting mobile file receiver thread..."));
    if (m_mobileFileReceiver)
    {
        m_mobileFileReceiver->waitForDone();
    }
    m_quitProgress->setValue(++i);

    m_quitProgress->setLabelText(tr("Clearing thread pool..."));
    WizQueuedThreadsShutdown();
    m_quitProgress->setValue(++i);
}

/**
 * @brief 处理已打开标签中的所有文档视图
 * @param callback 要执行的函数，类型为void(WizDocumentView*)
 */
void WizMainWindow::processAllDocumentViews(std::function<void(WizDocumentView*)> callback)
{
    for (int i = 0; i < m_mainTabBrowser->count(); ++i) {
        WizDocumentView* docView = qobject_cast<WizDocumentView*>(m_mainTabBrowser->widget(i));
        if ( docView == nullptr ) {
            continue;
        } else {
            callback(docView);
        }

    }
}

WizSearcher* WizMainWindow::searcher()
{
    return m_searcher;
}

WizMainWindow* WizMainWindow::instance()
{
    return windowInstance;
}

QNetworkDiskCache* WizMainWindow::webViewNetworkCache()
{
    return 0;
    //    return m_doc->web()->networkCache();
}

/**
 * @brief 返回当前文档视图
 * @return
 */
WizDocumentView* WizMainWindow::docView()
{

    //return m_doc;
    return qobject_cast<WizDocumentView*>(m_mainTabBrowser->currentWidget());
}

/**
 * @brief 检查笔记是否载入，尝试保存当前文档
 * @param callback 保存后的回调函数
 */
void WizMainWindow::trySaveCurrentNote(std::function<void(const QVariant &)> callback)
{
    WizDocumentView* curDocView = qobject_cast<WizDocumentView*>(m_mainTabBrowser->currentWidget());
    if (curDocView && curDocView->noteLoaded()) {
        curDocView->web()->trySaveDocument(curDocView->note(), false, callback);
    } else {
        callback(QVariant(true));
    }
}

void WizMainWindow::closeEvent(QCloseEvent* event)
{
#ifdef Q_OS_MAC
    if (event->spontaneous())
    {
//        wizMacHideCurrentApplication();
        if (windowState() & Qt::WindowFullScreen)
        {
            setWindowState(windowState() & ~Qt::WindowFullScreen);
            event->ignore();
            // wait for process finished
            QTimer::singleShot(1500, this, SLOT(hide()));
            return;
        }
        
        setVisible(false);
        event->ignore();
        //
        WizDatabase::clearCertPassword();
        //
        return;
    }
#else
    if (m_settings->showSystemTrayIcon())
    {
        setVisible(false);
        event->ignore();
    }
#endif
}

void WizMainWindow::mousePressEvent(QMouseEvent* event)
{
    _baseClass::mousePressEvent(event);
}

void WizMainWindow::mouseMoveEvent(QMouseEvent* event)
{
    _baseClass::mouseMoveEvent(event);
}

void WizMainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    _baseClass::mouseReleaseEvent(event);
}

void WizMainWindow::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::ActivationChange
            && isActiveWindow())
    {
        QTimer::singleShot(0, this, SLOT(windowActived()));
    }
    else if (event->type() == QEvent::WindowStateChange)
    {
        if (isMinimized())
        {
            WizDatabase::clearCertPassword();
        }
    }

    _baseClass::changeEvent(event);
}

void WizMainWindow::moveEvent(QMoveEvent* ev)
{
    _baseClass::changeEvent(ev);

    WizPositionDelegate& delegate = WizPositionDelegate::instance();
    delegate.mainwindowPositionChanged(ev->oldPos(), ev->pos());
}

void WizMainWindow::on_actionExit_triggered()
{
    //FIXME: 增加一个进度条，显示正在处理的收尾工作
    WizGetAnalyzer().logAction("MenuBarExit");
    //
    qApp->exit();
}

void WizMainWindow::on_actionClose_triggered()
{
    WizGetAnalyzer().logAction("MenuBarClose");

#ifdef Q_OS_MAC
    QWidget* wgt = qApp->activeWindow();
    if (wgt && wgt != this)
    {
        //FIXME:  窗口全屏时直接关闭会造成黑屏，此处改为先取消全屏然后关闭。

        if (wgt->windowState() & Qt::WindowFullScreen)
        {
            wgt->setWindowState(wgt->windowState() & ~Qt::WindowFullScreen);
        }
       wgt->close();
       wgt->deleteLater();
    }
    else
    {
        wizMacHideCurrentApplication();
//        setVisible(false);
    }
#else
    QWidget* wgt = qApp->activeWindow();
    if (wgt && wgt != this)
    {
       wgt->close();
    }
    else
    {
        if (m_settings->showSystemTrayIcon())
        {
            setVisible(false);
        }
    }

#endif
}

void WizMainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    update();
}

/**
 * @brief Show upgrade notification.
 * 
 * @param bUpgradeAvaliable 
 */
void WizMainWindow::on_checkUpgrade_finished(QJsonObject latestStable, QJsonObject latestTest)
{
    if (latestStable.isEmpty() && latestTest.isEmpty())
        return;

    QString strUrl, strMarkdown;
    // Stable release is preferred
    if (!latestStable.isEmpty()) {
        strUrl = latestStable["html_url"].toString();
        strMarkdown = latestStable["body"].toString();
    } else if (!latestTest.isEmpty()) {
        strUrl = latestTest["html_url"].toString();
        strMarkdown = latestTest["body"].toString();
    } else {
        return;
    }

    WizUpgradeNotifyDialog notifyDialog(this);
    notifyDialog.showMarkdownContent(strMarkdown);
    if (QDialog::Accepted == notifyDialog.exec()) {
        QDesktopServices::openUrl(QUrl(strUrl));
    }

}

bool isXMLRpcErrorCodeRelatedWithUserAccount(int nErrorCode)
{
    return //WIZKM_XMLRPC_ERROR_INVALID_TOKEN == nErrorCode ||
            WIZKM_XMLRPC_ERROR_INVALID_USER == nErrorCode ||
            WIZKM_XMLRPC_ERROR_INVALID_PASSWORD == nErrorCode ||
            WIZKM_XMLRPC_ERROR_SYSTEM_ERROR == nErrorCode;
}

void WizMainWindow::on_TokenAcquired(const QString& strToken)
{
    //Token::instance()->disconnect(this);
    disconnect(WizToken::instance(), SIGNAL(tokenAcquired(QString)), this,
            SLOT(on_TokenAcquired(QString)));

    if (strToken.isEmpty())
    {
        int nErrorCode = WizToken::lastErrorCode();
        // network unavailable
        if (QNetworkReply::ProtocolUnknownError == nErrorCode)
        {
            QMessageBox::critical(this, tr("Info"), tr("Connection is not available, please check your network connection."));
        }
        else if (isXMLRpcErrorCodeRelatedWithUserAccount(nErrorCode))
        {
            // disable quick download message to stop request token again
            m_bQuickDownloadMessageEnable = false;

            //try to relogin wiz server, but failed. may be password error
            m_settings->setPassword("");

            qDebug() << "username or password error, need relogin.";
            if (nErrorCode == WIZKM_XMLRPC_ERROR_SYSTEM_ERROR)
            {
                WizMessageBox::warning(this, tr("Info"), WizToken::lastErrorMessage());
            }
            else
            {
                WizMessageBox::warning(this, tr("Info"), tr("Username / password error. Please login again."));
            }
            on_actionLogout_triggered();
        }
    }
}

void WizMainWindow::on_quickSync_request(const QString& strKbGUID)
{
    m_syncQuick->addQuickSyncKb(strKbGUID);
}

void WizMainWindow::setSystemTrayIconVisible(bool bVisible)
{
    if (!m_tray)
    {
        m_tray = new WizTrayIcon(*this, QApplication::windowIcon(), this);
        initTrayIcon(m_tray);
        m_tray->show();
    }

    m_tray->setVisible(bVisible);
    // We don't want to quit application when the last window is closed if we
    // are using SystemTrayIcon.
    qApp->setQuitOnLastWindowClosed(!bVisible);
}

/**
 * @brief 从系统托盘处弹出通知
 * @param strTitle 通知标题
 * @param strInfo 通知信息
 */
void WizMainWindow::showBubbleNotification(const QString& strTitle, const QString& strInfo)
{
    if (m_tray && m_tray->isVisible())
    {
        m_tray->showMessage(strTitle, strInfo, QSystemTrayIcon::Information);
    }
}

/**
 * @brief 显示托盘图标菜单
 */
void WizMainWindow::showTrayIconMenu()
{
    if (m_trayMenu)
    {
        m_trayMenu->popup(QCursor::pos());
    }
}

/**
 * @brief 根据messageID来浏览消息
 *
 * 应该是接收消息弹窗的点击信号
 * @param messageID 消息ID
 */
void WizMainWindow::on_viewMessage_request(qint64 messageID)
{
    // 主窗口状态
    if (windowState() & Qt::WindowMinimized)
    {
        setWindowState(windowState() & ~Qt::WindowMinimized);
        show();
    }
    // 获得要浏览的分类树
    WizCategoryViewItemBase* pBase = m_category->findAllMessagesItem();
    if (!pBase)
        return;
    // 显示消息分类
    WizCategoryViewMessageItem* pItem = dynamic_cast<WizCategoryViewMessageItem*>(pBase);
    showMessageList(pItem);
    m_msgList->selectMessage(messageID);
}


void WizMainWindow::on_viewMessage_requestNormal(QVariant messageData)
{
    if (messageData.type() == QVariant::Bool)
    {
        QString strUrl = WizOfficialApiEntry::standardCommandUrl("link");
        if (!strUrl.startsWith("http")) {
            return;
        }
        strUrl = strUrl + "&site=wiznote";
        strUrl += "&name=mac-sync-error-solution";
        QDesktopServices::openUrl(QUrl(strUrl));
    }
    else if (messageData.type() == QVariant::Int)
    {
        if (WIZKM_XMLRPC_ERROR_VIP_SERVICE_EXPR == messageData
                || WIZKM_XMLRPC_ERROR_FREE_SERVICE_EXPR == messageData)
        {
            showVipUpgradePage();
        }
    }
}

/**
 * @brief 根据消息数据浏览消息
 * @param msg
 */
void WizMainWindow::on_viewMessage_request(const WIZMESSAGEDATA& msg)
{
    WIZDOCUMENTDATA doc;
    if (
            (msg.nMessageType < WIZ_USER_MSG_TYPE_REQUEST_JOIN_GROUP
             || msg.nMessageType == WIZ_USER_MSG_TYPE_LIKE
             || msg.nMessageType == WIZ_USER_MSG_TYPE_REMIND
             || msg.nMessageType == WIZ_USER_MSG_TYPE_REMIND_CREATE
             )
            && !m_dbMgr.db(msg.kbGUID).documentFromGuid(msg.documentGUID, doc)
            )
    {
        QMessageBox::information(this, tr("Warning"), tr("Can't find note %1 , may be it has been deleted.").arg(msg.title));
        return;
    }

    if (msg.nMessageType == WIZ_USER_MSG_TYPE_COMMENT ||
            msg.nMessageType == WIZ_USER_MSG_TYPE_CALLED_IN_COMMENT||
            msg.nMessageType == WIZ_USER_MSG_TYPE_COMMENT_REPLY)
    {
        //  show comments
        //viewDocument(doc, true);
        viewDocument(doc); // 在新标签浏览
        showCommentWidget();
    }
    else if (msg.nMessageType < WIZ_USER_MSG_TYPE_REQUEST_JOIN_GROUP
             || msg.nMessageType == WIZ_USER_MSG_TYPE_LIKE
             || msg.nMessageType == WIZ_USER_MSG_TYPE_REMIND
             || msg.nMessageType == WIZ_USER_MSG_TYPE_REMIND_CREATE
             )
    {
        //viewDocument(doc, true);
        viewDocument(doc);
    }
}

void WizMainWindow::on_dockMenuAction_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        QString guid = action->data().toString();
        if (guid == MAINWINDOW)
        {
            bringWidgetToFront(this);
        }
        else
        {
            WizSingleDocumentViewer* viewer = m_singleViewMgr->getDocumentViewer(guid);
            if (viewer)
            {
                bringWidgetToFront(viewer);
            }
        }
    }
}

void WizMainWindow::on_trayIcon_newDocument_clicked()
{
    setVisible(true);
    QApplication::setActiveWindow(this);
    raise();

    on_actionNewNote_triggered();
}

void WizMainWindow::on_hideTrayIcon_clicked()
{
    setSystemTrayIconVisible(false);
    userSettings().setShowSystemTrayIcon(false);
}

void WizMainWindow::handleTrayIconActived(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
#ifdef Q_OS_MAC
// On macOS, the context menu opens on mouse press, so we use middle click to show main window.
    case QSystemTrayIcon::MiddleClick:
        shiftVisableStatus();
        break;
#else
    case QSystemTrayIcon::Trigger:
        shiftVisableStatus();
        break;
#endif
    default:
        break;
    }
}

void WizMainWindow::shiftVisableStatus()
{
    switch(windowState()) {
        case Qt::WindowNoState:
            // Normal window, but de-activated
            showNormal();
            break;
        case Qt::WindowMinimized:
            // Normal window, but minimized
            showNormal();
            break;
        case Qt::WindowMaximized:
            // Maximized window, but de-activated
            showMaximized();
            break;
        case Qt::WindowMaximized | Qt::WindowMinimized:
            // Maximized window, but minimized
            showMaximized();
            break;
        default:
            showNormal();
            break;
    }

    if (isVisible()) {
        // Actovate main window
        activateWindow();
        raise();
    }
}

#ifdef WIZ_OBOSOLETE
void WizMainWindow::on_upgradeThread_finished()
{
    QString strUrl = m_upgrade->whatsNewUrl();
    if (strUrl.isEmpty()) {
        return;
    }

    WizUpgradeNotifyDialog notifyDialog(strUrl, this);

    if (QDialog::Accepted == notifyDialog.exec()) {
        QFile file(::WizGetUpgradePath() + "WIZNOTE_READY_FOR_UPGRADE");
        file.open(QIODevice::WriteOnly | QIODevice::Truncate);
        file.close();

        // restart immediately
        m_bRestart = true;
        on_actionExit_triggered();
    } else {
        // skip for this session
        QFile file(::WizGetUpgradePath() + "WIZNOTE_SKIP_THIS_SESSION");
        file.open(QIODevice::WriteOnly | QIODevice::Truncate);
        file.close();
    }
}
#endif

WizMainWindow::~WizMainWindow()
{
    delete m_history;
}

/**
 * @brief 保存主窗口布局信息
 */
void WizMainWindow::saveStatus()
{
    QSettings* settings = WizGlobal::globalSettings();
    settings->setValue("Window/Geometry", saveGeometry());
    settings->setValue("Window/Splitter", m_splitter->saveState());
}

/**
 * @brief 恢复主窗口布局信息
 */
void WizMainWindow::restoreStatus()
{
    QSettings* settings = WizGlobal::globalSettings();
    QByteArray geometry = settings->value("Window/Geometry").toByteArray();
    QByteArray splitterState = settings->value("Window/Splitter").toByteArray();
    // main window
    if (geometry.isEmpty()) {

        bool isHighPix = WizIsHighPixel();
        QSettings defaulSettings(Utils::WizPathResolve::skinResourcesPath(Utils::WizStyleHelper::themeName()) + "skin.ini", QSettings::IniFormat);
        if (isHighPix)
        {
            geometry = defaulSettings.value("Window/GeometryHighPix").toByteArray();
            splitterState = defaulSettings.value("Window/SplitterHighPix").toByteArray();
        }
        else
        {
            geometry = defaulSettings.value("Window/GeometryNormal").toByteArray();
            splitterState = defaulSettings.value("Window/SplitterNormal").toByteArray();
        }
    }

    restoreGeometry(geometry);
    m_splitter->restoreState(splitterState);
}

void WizMainWindow::initQuitHandler()
{
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(on_application_aboutToQuit()));
    qApp->installEventFilter(this);

#ifdef Q_OS_MAC
    installEventFilter(this);

    if (systemWidgetBlurAvailable())
    {
        setAutoFillBackground(false);
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

#endif
}

/**
 * @brief Init searcher thread when application stat.
 */
void WizMainWindow::initSearcher()
{
    m_searcher->start(QThread::HighPriority);
    connect(m_searcher, SIGNAL(searchProcess(const QString&, const CWizDocumentDataArray&, bool, bool)),
        SLOT(on_searchProcess(const QString&, const CWizDocumentDataArray&, bool, bool)));
}

void WizMainWindow::initSyncFull()
{
    m_syncFull->setFullSyncInterval(userSettings().syncInterval());
    connect(m_syncFull, SIGNAL(processLog(const QString&)), SLOT(on_syncProcessLog(const QString&)));
    connect(m_syncFull, SIGNAL(promptMessageRequest(int, const QString&, const QString&)),
            SLOT(on_promptMessage_request(int, QString, QString)));
    connect(m_syncFull, SIGNAL(promptFreeServiceExpr(WIZGROUPDATA)), SLOT(on_promptFreeServiceExpr(WIZGROUPDATA)));
    connect(m_syncFull, SIGNAL(promptVipServiceExpr(WIZGROUPDATA)), SLOT(on_promptVipServiceExpr(WIZGROUPDATA)));

    connect(m_syncFull, SIGNAL(bubbleNotificationRequest(const QVariant&)),
            SLOT(on_bubbleNotification_request(const QVariant&)));
    connect(m_syncFull, SIGNAL(syncStarted(bool)), SLOT(on_syncStarted(bool)));
    connect(m_syncFull, SIGNAL(syncFinished(int, bool, QString, bool)), SLOT(on_syncDone(int, bool, QString, bool)));
    // 如果没有禁止自动同步，则在打开软件后立即同步一次
    if (m_settings->syncInterval() > 0)
    {
        QTimer::singleShot(15 * 1000, m_syncFull, SLOT(syncAfterStart()));
    }
}

void WizMainWindow::initSyncQuick()
{
    WizKMSyncThread::setQuickThread(m_syncQuick);

    connect(m_syncQuick, SIGNAL(promptFreeServiceExpr(WIZGROUPDATA)), SLOT(on_promptFreeServiceExpr(WIZGROUPDATA)));
    connect(m_syncQuick, SIGNAL(promptVipServiceExpr(WIZGROUPDATA)), SLOT(on_promptVipServiceExpr(WIZGROUPDATA)));
}

/**
 * @brief 初始化所有动作
 */
void WizMainWindow::initActions()
{
#ifdef Q_OS_MAC
    m_actions->init();
#else
    m_actions->init(true);
#endif
    m_animateSync->setAction(m_actions->actionFromName(WIZACTION_GLOBAL_SYNC));
    m_animateSync->setSingleIcons("sync");

    connect(m_actions, SIGNAL(insertTableSelected(int,int)), SLOT(on_actionMenuFormatInsertTable(int,int)));
    //FIXME: 已经不是唯一文档视图，不应该在初始化阶段绑定
    //connect(m_doc->web(), SIGNAL(statusChanged(const QString&)), SLOT(on_editor_statusChanged(const QString&)));
}

void setActionCheckState(const QList<QAction*>& actionList, int type)
{
    for (int i = 0; i < actionList.count(); i++)
    {
        QAction* action = actionList.at(i);
        if (action->data().toInt() == type)
        {
            action->setChecked(true);
            break;
        }
    }
}

/*!
    WizNote custom style menu list initialization
 */
void WizMainWindow::initMenuList()
{
    m_actions->buildMenuList(m_menu, Utils::WizPathResolve::resourcesPath() + "files/mainmenu.ini", m_windowListMenu);

    initMenuActionState();

    initViewTypeActionGroup();
    initSortTypeActionGroup();
}

/*!
    System style menu bar initialization    
 */
void WizMainWindow::initMenuBar()
{
    m_menuBar = new QMenuBar(this);
    setMenuBar(m_menuBar);
    m_actions->buildMenuBar(m_menuBar, Utils::WizPathResolve::resourcesPath() + "files/mainmenu.ini", m_windowListMenu);

    initMenuActionState();

    initViewTypeActionGroup();
    initSortTypeActionGroup();

}

void WizMainWindow::initMenuActionState()
{
    connect(m_windowListMenu, SIGNAL(aboutToShow()), SLOT(resetWindowMenu()));
    connect(m_singleViewMgr, SIGNAL(documentViewerClosed(QString)),
            SLOT(removeWindowsMenuItem(QString)));

    m_actions->actionFromName(WIZCATEGORY_OPTION_MESSAGECENTER)->setCheckable(true);
    m_actions->actionFromName(WIZCATEGORY_OPTION_SHORTCUTS)->setCheckable(true);
    m_actions->actionFromName(WIZCATEGORY_OPTION_QUICKSEARCH)->setCheckable(true);
    m_actions->actionFromName(WIZCATEGORY_OPTION_FOLDERS)->setCheckable(true);
    m_actions->actionFromName(WIZCATEGORY_OPTION_TAGS)->setCheckable(true);
    m_actions->actionFromName(WIZCATEGORY_OPTION_BIZGROUPS)->setCheckable(true);
    m_actions->actionFromName(WIZCATEGORY_OPTION_PERSONALGROUPS)->setCheckable(true);
    m_actions->actionFromName(WIZACTION_GLOBAL_SHOW_SUBFOLDER_DOC)->setCheckable(true);
    m_actions->actionFromName(WIZLAYOUT_CATEGORY_VIEW)->setCheckable(true);
    m_actions->actionFromName(WIZLAYOUT_DOCUMENTLIST_VIEW)->setCheckable(true);
    m_actions->actionFromName(WIZLAYOUT_TAB_BROWSER)->setCheckable(true);

    bool checked = m_category->isSectionVisible(Section_MessageCenter);
    m_actions->actionFromName(WIZCATEGORY_OPTION_MESSAGECENTER)->setChecked(checked);
    checked = m_category->isSectionVisible(Section_Shortcuts);
    m_actions->actionFromName(WIZCATEGORY_OPTION_SHORTCUTS)->setChecked(checked);
    checked = m_category->isSectionVisible(Section_QuickSearch);
    m_actions->actionFromName(WIZCATEGORY_OPTION_QUICKSEARCH)->setChecked(checked);
    checked = m_category->isSectionVisible(Section_Folders);
    m_actions->actionFromName(WIZCATEGORY_OPTION_FOLDERS)->setChecked(checked);
    m_actions->actionFromName(WIZCATEGORY_OPTION_FOLDERS)->setEnabled(false);
    checked = m_category->isSectionVisible(Section_Tags);
    m_actions->actionFromName(WIZCATEGORY_OPTION_TAGS)->setChecked(checked);
    checked = m_category->isSectionVisible(Section_BizGroups);
    m_actions->actionFromName(WIZCATEGORY_OPTION_BIZGROUPS)->setChecked(checked);
    checked = m_category->isSectionVisible(Section_PersonalGroups);
    m_actions->actionFromName(WIZCATEGORY_OPTION_PERSONALGROUPS)->setChecked(checked);
    checked = m_settings->showSubFolderDocuments();
    m_actions->actionFromName(WIZACTION_GLOBAL_SHOW_SUBFOLDER_DOC)->setChecked(checked);

    // GUI Layout
    checked = m_settings->showLayoutCategoryView();
    m_actions->actionFromName(WIZLAYOUT_CATEGORY_VIEW)->setChecked(checked);
    checked = m_settings->showLayoutDocumentListView();
    m_actions->actionFromName(WIZLAYOUT_DOCUMENTLIST_VIEW)->setChecked(checked);
    checked = m_settings->showLayoutTabBrowser();
    m_actions->actionFromName(WIZLAYOUT_TAB_BROWSER)->setChecked(checked);

    initViewTypeActionGroup();
    initSortTypeActionGroup();

}

/**
 * @brief 初始化笔记列表视图动作组
 */
void WizMainWindow::initViewTypeActionGroup() {
    // 笔记列表视图
    // 从所有Actions中搜寻，并添加到m_viewTypeActions成员中
    m_viewTypeActions = new QActionGroup(m_menuBar);
    QAction* action = m_actions->actionFromName(WIZCATEGORY_OPTION_THUMBNAILVIEW);
    action->setCheckable(true);
    action->setData(WizDocumentListView::TypeThumbnail);
    m_viewTypeActions->addAction(action);
    action = m_actions->actionFromName(WIZCATEGORY_OPTION_SEARCHRESULTVIEW);
    action->setCheckable(true);
    action->setData(WizDocumentListView::TypeSearchResult);
    m_viewTypeActions->addAction(action);
    action = m_actions->actionFromName(WIZCATEGORY_OPTION_TWOLINEVIEW);
    action->setCheckable(true);
    action->setData(WizDocumentListView::TypeTwoLine);
    m_viewTypeActions->addAction(action);
    action = m_actions->actionFromName(WIZCATEGORY_OPTION_ONELINEVIEW);
    action->setCheckable(true);
    action->setData(WizDocumentListView::TypeOneLine);
    m_viewTypeActions->addAction(action);
    int viewType = userSettings().get("VIEW_TYPE").toInt();
    setActionCheckState(m_viewTypeActions->actions(), viewType);
}

/**
 * @brief 初始化笔记列表筛选排序动作组
 */
void WizMainWindow::initSortTypeActionGroup() {
    // 笔记列表排序类型视图
    // 从所有Actions中搜寻，并添加到m_sortTypeActions成员中
    m_sortTypeActions = new QActionGroup(m_menuBar);
    QAction* action = m_actions->actionFromName(WIZDOCUMENT_SORTBY_CREATEDTIME);
    action->setData(SortingByCreatedTime);
    m_sortTypeActions->addAction(action);
    action = m_actions->actionFromName(WIZDOCUMENT_SORTBY_UPDATEDTIME);
    action->setData(SortingByModifiedTime);
    m_sortTypeActions->addAction(action);
    action = m_actions->actionFromName(WIZDOCUMENT_SORTBY_ACCESSTIME);
    action->setData(SortingByAccessedTime);
    m_sortTypeActions->addAction(action);
    action = m_actions->actionFromName(WIZDOCUMENT_SORTBY_TITLE);
    action->setData(SortingByTitle);
    m_sortTypeActions->addAction(action);
    action = m_actions->actionFromName(WIZDOCUMENT_SORTBY_FOLDER);
    action->setData(SortingByLocation);
    m_sortTypeActions->addAction(action);
    action = m_actions->actionFromName(WIZDOCUMENT_SORTBY_SIZE);
    action->setData(SortingBySize);
    m_sortTypeActions->addAction(action);
    for (QAction* actionItem : m_sortTypeActions->actions())
    {
        actionItem->setCheckable(true);
    }
    int sortType = qAbs(userSettings().get("SORT_TYPE").toInt());
    setActionCheckState(m_sortTypeActions->actions(), sortType);
}


void WizMainWindow::initDockMenu()
{
#ifdef Q_OS_MAC
    m_dockMenu = new QMenu(this);
    m_dockMenu->setAsDockMenu();

    connect(m_dockMenu, SIGNAL(aboutToShow()), SLOT(resetDockMenu()));
#endif
}

void WizMainWindow::on_editor_statusChanged(const QString& currentStyle)
{
}

/*!
    Download templates data if necessary
 */
void WizMainWindow::createNoteByTemplate(const TemplateData& tmplData)
{
    QFileInfo info(tmplData.strFileName);
    if (info.exists() || tmplData.type == BuildInTemplate)
    {
        createNoteByTemplateCore(tmplData);
    }
    else
    {
        qDebug() << "template file not exists : " << tmplData.strFileName;
        WizExecutingActionDialog::executeAction(tr("Downloading template..."), WIZ_THREAD_DEFAULT, [=]{
            bool ret = WizNoteManager::downloadTemplateBlocked(tmplData);
            ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
                if (ret) {
                    createNoteByTemplateCore(tmplData);
                } else {
                    QMessageBox::warning(this,
                        tr("Error"), tr("Can't download template from server. Please try again later."));
                }
            });
        });
    }
}

/*!
    Create document from template \a tmplData.
 */
void WizMainWindow::createNoteByTemplateCore(const TemplateData& tmplData)
{
    QFileInfo info(tmplData.strFileName);
    initVariableBeforCreateNote();

    QString kbGUID;
    WIZTAGDATA currTag;
    QString currLocation;
    m_category->getAvailableNewNoteTagAndLocation(kbGUID, currTag, currLocation);

    if (currLocation.isEmpty())
        currLocation = m_dbMgr.db(kbGUID).getDefaultNoteLocation();

    WIZDOCUMENTDATAEX data;
    data.strKbGUID = kbGUID;
    data.strType = tmplData.buildInName;
    data.strTitle = tmplData.strTitle.isEmpty() ? info.completeBaseName() : tmplData.strTitle;

    // Journal {date}({week})
    if (tmplData.strTitle.isEmpty()) {
        data.strTitle = tmplData.strName;
    } else {
        WizOleDateTime dt;
        data.strTitle.replace("{date}", dt.toLocalLongDate());
        data.strTitle.replace("{date_time}", dt.toLocalLongDate() + " " + dt.toString("hh:mm:ss"));
        QLocale local;
        data.strTitle.replace("{week}", local.toString(dt.toLocalTime(), "ddd"));
    }

    // Personal
    if (kbGUID.isEmpty()) {
        data.strKbGUID = m_dbMgr.db().kbGUID();
        data.strLocation = tmplData.strFolder;

        if (data.strLocation.isEmpty()) {
            data.strLocation = currLocation;
        } else {
            data.strLocation.replace("{year}", QDate::currentDate().toString("yyyy"));
            data.strLocation.replace("{month}", QDate::currentDate().toString("MM"));

            if (WizCategoryViewFolderItem* folder = m_category->findFolder(data.strLocation, true, false))
            {
                if (m_category->currentItem() != folder)
                {
                    m_category->setCurrentItem(folder);
                }
            }
        }
    } else {
        data.strLocation = currLocation;
    }

    // Actually create the note
    WizNoteManager noteManager(m_dbMgr);
    if (data.strType == "collaboration") {
        CollaborationDocView *newView = new CollaborationDocView(data, *this, this);
        newView->createDocument(currTag);
        m_mainTabBrowser->createTab(newView);
        return;
    } else {
        if (!noteManager.createNoteByTemplate(data, currTag, tmplData.strFileName))
            return;
    }

    if (data.strType == "svgpainter") {
        createHandwritingNote(m_dbMgr, data, this);
    }

    setFocusForNewNote(data);

    locateDocument(data);

    CWizDocumentDataArray arrayDocument;
    m_documents->getSelectedDocuments(arrayDocument);
    if (arrayDocument.size() == 1 && arrayDocument[0].strGUID == data.strGUID)
    {
        //already selected
    }
    else
    {
        viewDocument(data);
    }

    quickSyncKb(kbGUID);
}

/**
 * @brief 接收移动端发送的图片等文件
 * @param strFile 文件名
 */
void WizMainWindow::on_mobileFileRecived(const QString& strFile)
{
    WizDocumentView* docView = currentDocumentView();
    if (docView->web()->isEditing())
    {
        QImageReader imageReader(strFile);
        bool isImageFile = imageReader.canRead();

        if (isImageFile)
        {
            QString strHtml;
            if (WizImage2Html(strFile, strHtml, docView->web()->noteResourcesPath()))
            {
                docView->web()->editorCommandExecuteInsertHtml(strHtml, false);
            }
        }
        else
        {
            const WIZDOCUMENTDATA& doc = docView->note();
            WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
            WIZDOCUMENTATTACHMENTDATA attach;
            db.addAttachment(doc, strFile, attach);
        }
    }
    else
    {
        QFile::remove(strFile);
    }
}

void WizMainWindow::on_shareDocumentByLink_request(const QString& strKbGUID, const QString& strGUID)
{
    WizAccountManager account(m_dbMgr);
    WizDatabase& db = m_dbMgr.db(strKbGUID);
    if (db.isGroup())
    {
        if (db.isPersonalGroup())
            return;

        if (account.isPaidGroup(strKbGUID))
        {
            if (!db.isGroupSuper())
            {
                WizMessageBox::information(this, tr("Info"), tr("Your permission is insufficient, super member or group administrators can share notes."));
                return;
            }
        }
        else
        {
            WizMessageBox::information(this, tr("Info"), tr("You are using the free version of the service, upgrade now, you can share notes."));
            return;
        }
    }
    else if (!account.isVip())
    {
        openVipPageInWebBrowser();
        return;
    }

    //
    WIZDOCUMENTDATA doc;
    if (!m_dbMgr.db(strKbGUID).documentFromGuid(strGUID, doc))
    {
        qDebug() << "[ShareLink] can not find doc " << strGUID;
        return;
    }

    if (doc.nProtected == 1)
    {
        WizMessageBox::information(this, tr("Info"), tr("Can not share encrpyted notes."));
        return;
    }

    WizShareLinkDialog dlg(userSettings());
    dlg.shareDocument(doc);
    int result = dlg.exec();
    qDebug() << result;
    if (result == 100) {
        //upgrade vip
        showVipUpgradePage();
    } else if (result == 200) {
        //mobile
        m_userInfoWidget->showAccountSettings();
    }
}

void WizMainWindow::openVipPageInWebBrowser()
{
    QMessageBox msg(this);
    msg.setWindowTitle(tr("Upgrading to VIP"));
    msg.setIcon(QMessageBox::Information);
    msg.setText(tr("Only VIP user can create link, please retry after upgrading to VIP and syncing to server."));
    msg.addButton(tr("Cancel"), QMessageBox::NoRole);
    QPushButton *actionBuy = msg.addButton(tr("Upgrade now"), QMessageBox::YesRole);
    msg.setDefaultButton(actionBuy);
    msg.exec();

    if (msg.clickedButton() == actionBuy)
    {
        showVipUpgradePage();
    }
}

/**
 * @brief 通过用户GUID载入消息
 * @param guid 用户标识
 */
void WizMainWindow::loadMessageByUserGuid(const QString& guid)
{
    CWizMessageDataArray arrayMsg;
    if(guid.isEmpty())
    {
        if (m_msgListTitleBar->isUnreadMode())
        {
            m_dbMgr.db().getUnreadMessages(arrayMsg);
        }
        else
        {
            m_dbMgr.db().getAllMessages(arrayMsg);
        }
    }
    else
    {
        if (m_msgListTitleBar->isUnreadMode())
        {
            m_dbMgr.db().unreadMessageFromUserGUID(guid, arrayMsg);
        }
        else
        {
            m_dbMgr.db().messageFromUserGUID(guid, arrayMsg);
        }
    }
    //
    m_msgList->setMessages(arrayMsg);
}

/**
 * @brief 通过GUID获得动作
 * @param actionList 要检索的动作指针列表
 * @param guid 目标动作GUID
 * @return
 */
QAction* actionByGuid(const QList<QAction*>& actionList, const QString guid)
{
    for (QAction* action : actionList)
    {
        if (action->data().toString() == guid)
            return action;
    }

    return nullptr;
}

bool caseInsensitiveLessThan(QAction* action1, QAction* action2) {
    //
    const QString k1 = action1->text().toLower();
    const QString k2 = action2->text().toLower();

    return WizToolsSmartCompare(k1, k2) < 0;
}


void WizMainWindow::resetWindowListMenu(QMenu* menu, bool removeExists)
{
    QList<QAction*> actionList = menu->actions();
    QWidget * activeWidget = QApplication::activeWindow();
    // if current app is not active, there will no activewindow. remenber last active window to set menu item checkstate
    static QWidget * lastActiveWidget = activeWidget;
    activeWidget == nullptr ? (activeWidget = lastActiveWidget) : (lastActiveWidget = activeWidget);
//    QIcon icon = Utils::StyleHelper::loadIcon("actionSaveAsHtml");

    QList<QAction*> newActions;
    QAction* action = nullptr;
    if (removeExists)
    {
        action = actionByGuid(actionList, MAINWINDOW);
        menu->removeAction(action);
    }

    action = new QAction(tr("WizNote"), menu);
    action->setData(MAINWINDOW);
    action->setCheckable(true);
    action->setChecked((activeWidget == nullptr || activeWidget == this));
    newActions.append(action);

    QMap<QString, WizSingleDocumentViewer*>& viewerMap = m_singleViewMgr->getDocumentViewerMap();
    QList<QString> keys = viewerMap.keys();
    for (int i = 0; i < keys.count(); i++)
    {
        WizSingleDocumentViewer* viewer = viewerMap.value(keys.at(i));
        if (removeExists)
        {
            action = actionByGuid(actionList, keys.at(i));
            menu->removeAction(action);
        }
        action = new QAction(viewer->windowTitle(), menu);
        action->setData(keys.at(i));
        action->setCheckable(true);
        action->setChecked(viewer == activeWidget);
        newActions.append(action);
    }

    qSort(newActions.begin(), newActions.end(), caseInsensitiveLessThan);
    for (QAction* action : newActions)
    {
        connect(action, SIGNAL(triggered()), SLOT(on_dockMenuAction_triggered()));
    }
    menu->addActions(newActions);
}

void WizMainWindow::changeDocumentsSortTypeByAction(QAction* action)
{
    if (action)
    {
        int type = action->data().toInt();
        m_documents->resetItemsSortingType(type);
        emit documentsSortTypeChanged(type);
    }
}

bool WizMainWindow::processApplicationActiveEvent()
{
    QMap<QString, WizSingleDocumentViewer*>& viewerMap = m_singleViewMgr->getDocumentViewerMap();
    QList<WizSingleDocumentViewer*> singleViewrList = viewerMap.values();
    for (WizSingleDocumentViewer* viewer : singleViewrList)
    {
        if (viewer->isVisible())
            return true;
    }

    if (!isVisible())
    {
        show();
        shiftVisableStatus();
    }

    return true;
}

void WizMainWindow::resetDockMenu()
{
#ifdef Q_OS_MAC
    m_dockMenu->clear();
    resetWindowListMenu(m_dockMenu, false);
#endif
}

void WizMainWindow::resetWindowMenu()
{
    resetWindowListMenu(m_windowListMenu, true);
}

void WizMainWindow::removeWindowsMenuItem(QString guid)
{
    QList<QAction*> actionList = m_windowListMenu->actions();
    QAction* action = actionByGuid(actionList, guid);
    if (action)
    {
        m_windowListMenu->removeAction(action);
    }

    resetDockMenu();
}

void WizMainWindow::showVipUpgradePage()
{
#ifndef BUILD4APPSTORE
        WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
            QString strToken = WizToken::token();
            QString strUrl = WizCommonApiEntry::makeUpUrlFromCommand("vip", strToken);
            WizExecuteOnThread(WIZ_THREAD_MAIN, [=](){
                WizShowWebDialogWithToken(tr("Account settings"), strUrl, this);
            });
        });
#else
    WizIAPDialog* dlg = iapDialog();
    dlg->loadIAPPage();
    dlg->exec();
#endif
}

void WizMainWindow::showTemplateIAPDlg(const TemplateData& tmpl)
{
#ifdef BUILD4APPSTORE
    if (!m_templateIAPDialog)
    {
        m_templateIAPDialog = new WizTemplatePurchaseDialog(this);
        m_templateIAPDialog->setModal(true);
        connect(m_templateIAPDialog, &WizTemplatePurchaseDialog::purchaseSuccess, [=](){
            WizNoteManager manager(m_dbMgr);
            manager.downloadTemplatePurchaseRecord();
        });
    }
    m_templateIAPDialog->showTemplateInfo(tmpl.id, tmpl.strName, tmpl.strThumbUrl);
    m_templateIAPDialog->open();
#endif
}

/**
 * @brief 准备新建笔记菜单栏
 */
void WizMainWindow::prepareNewNoteMenu()
{
    if (!m_newNoteExtraMenu)
    {
        m_newNoteExtraMenu = new QMenu(this);

        QList<TemplateData> tmplList;
        getTemplateListFroNewNoteMenu(tmplList);
        for (TemplateData tmpl : tmplList)
        {
            if (tmpl.strName == "-")
            {
                m_newNoteExtraMenu->addSeparator();
            }
            else
            {
                QAction* action = m_newNoteExtraMenu->addAction(tmpl.strName, this, SLOT(on_newNoteByExtraMenu_request()));
                action->setData(tmpl.toQVariant());
            }
        }
    }
}


/*!
    Decode templates data from action
 */
void WizMainWindow::on_newNoteByExtraMenu_request()
{
    if (QAction* action = qobject_cast<QAction*>(sender()))
    {
        TemplateData tmpl;
        tmpl.fromQVariant(action->data());
        if (isTemplateUsable(tmpl, m_dbMgr))
        {
            createNoteByTemplate(tmpl);
        }
        else
        {
            WizTemplateUpgradeResult result = showTemplateUnusableDialog(this);
            switch (result) {
            case UpgradeResult_None:
                return;
                break;
            case UpgradeResult_UpgradeVip:
                showVipUpgradePage();
                return;
                break;
            case UpgradeResult_PurchaseTemplate:
            {
                showTemplateIAPDlg(tmpl);
                return;
            }
                break;
            default:
                Q_ASSERT(0);
                break;
            }
        }
    }
}

void WizMainWindow::windowActived()
{
    if (m_quiting)
        return;
    //
    static  bool isBizUser = m_dbMgr.db().meta("BIZS", "COUNT").toInt() > 0;
    if (!isBizUser || !m_bQuickDownloadMessageEnable)
        return;
    //
    if (!m_syncFull)
        return;

    m_syncFull->quickDownloadMesages();
    WizGetAnalyzer().logAction("bizUserQuickDownloadMessage");
}

/**
 * @brief 用系统默认浏览器打开链接
 * @param strUrl 目标链接
 */
void WizMainWindow::OpenURLInDefaultBrowser(const QString& strUrl)
{
    QDesktopServices::openUrl(strUrl);
}

/**
 * @brief web页面调用该方法，token失效时重新获取token
 * @param strFunctionName
 */
void WizMainWindow::GetToken(const QString& strFunctionName)
{
    WizDocumentView* docView = currentDocumentView();
    CString functionName(strFunctionName);
    ::WizExecuteOnThread(WIZ_THREAD_NETWORK, [=] {
        //
        QString strToken = WizToken::token();
        if (strToken.isEmpty())
            return;
        //
        ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=] {

            QString strExec = functionName + QString("('%1')").arg(strToken);
            qDebug() << "cpp get token callled : " << strExec;
            docView->commentView()->page()->runJavaScript(strExec);
        });
    });
}

/**
 * @brief web页面调用该方法，将页面的结果返回
 * @param result  web页面返回结果，如需更新笔记数据，会返回1
 */
void WizMainWindow::SetDialogResult(int nResult)
{
    WizDocumentView* docView = currentDocumentView();
    if (nResult > 0)
    {
        //
        const WIZDOCUMENTDATA& doc = docView->note();
        if (doc.strKbGUID.isEmpty())
            return;

        m_dbMgr.db(doc.strKbGUID).setObjectDataDownloaded(doc.strGUID, "document", false);
        docView->viewNote(doc, false);
    }
}

void WizMainWindow::AppStoreIAP()
{
#ifdef BUILD4APPSTORE
    WizIAPDialog* dlg = iapDialog();
    dlg->loadIAPPage();
    if (!dlg->isActiveWindow())
    {
        dlg->exec();
    }
#endif
}

void WizMainWindow::copyLink(const QString& link)
{
    Utils::WizMisc::copyTextToClipboard(link);
}

void WizMainWindow::onClickedImage(const QString& src, const QString& list)
{

    Json::Value d;
    Json::Reader reader;
    if (reader.parse(list.toUtf8().constData(), d))
    {
        CWizStdStringArray files;
        if (d.isArray())
        {
            for (int i = 0; i < d.size(); i++)
            {
                QString file = QString::fromStdString(d[i].asString());
                files.push_back(file);
            }
        }
        //
        if (!files.empty())
        {
            files.insert(files.begin(), src);
            //
            CWizStdStringArray sl;
            sl.push_back("open");
            sl.push_back("-a");
            sl.push_back("Preview");
            //
            QString workingPath;
            //
            for (auto it = files.begin(); it != files.end(); it++)
            {
                QString fileUrl = *it;
                QUrl url(fileUrl);
                QString path = url.toLocalFile();
                //
                if (workingPath.isEmpty())
                {
                    workingPath = Utils::WizMisc::extractFilePath(path);
                }
                else
                {
                    QString currentPath = Utils::WizMisc::extractFilePath(path);
                    if (currentPath != workingPath)
                    {
                        continue;
                    }
                }
                //
                QString fileName = Utils::WizMisc::extractFileName(path);
                sl.push_back("\"" + fileName + "\"");
            }
            //
            CString commandLine;
            WizStringArrayToText(sl, commandLine, " ");
            //
            QProcess* process = new QProcess(this);
            process->setWorkingDirectory(workingPath);
            process->start(commandLine);
            //
            return;
        }
    }
    //

    QUrl url = QUrl(src);
    QDesktopServices::openUrl(url);
}

/**
 * @brief 布局标题栏
 */
void WizMainWindow::layoutTitleBar()
{
    WizWindowTitleBar* title = titleBar();
    title->titleLabel()->setVisible(false);

    QLayout* layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* layoutTitle = new QHBoxLayout();
    layoutTitle->setContentsMargins(0, 0, 0, 0);

    // 标题栏里组件区域大小，通过 margin 来控制
    QLayout* layoutToolBarContainer = new QHBoxLayout();

    int margin = 4;

    layoutToolBarContainer->setContentsMargins(0, margin, 0, margin);
    layoutToolBarContainer->addWidget(m_toolBar);
    layoutTitle->addItem(layoutToolBarContainer);
    //layoutTitle->addStretch();

    QHBoxLayout* layoutRight = new QHBoxLayout();
    layoutTitle->addItem(layoutRight);

    QLayout* layoutBtnBox = new QHBoxLayout();
    layoutBtnBox->setContentsMargins(0, 0, 10, 0);
    layoutBtnBox->setSpacing(8);
    layoutRight->addItem(layoutBtnBox);

    m_menuButton = new QToolButton(this);
    m_menuButton->setObjectName("window-menu-btn");
#ifndef Q_OS_MAC
    connect(m_menuButton, &QToolButton::clicked, this, &WizMainWindow::on_menuButtonClicked);
    setHitTestVisible(m_menuButton);
    layoutBtnBox->addWidget(m_menuButton);
    layoutBtnBox->addWidget(title->minButton());
    layoutBtnBox->addWidget(title->maxButton());
    layoutBtnBox->addWidget(title->closeButton());
#else
    rootWidget()->setContentsMargins(0, 0, 0, 0);
    titleBar()->maxButton()->setVisible(false);
    titleBar()->minButton()->setVisible(false);
    titleBar()->closeButton()->setVisible(false);
    m_menuButton->setVisible(false);
#endif // Q_OS_MAC

    QSize buttonSize = QSize(WizSmartScaleUI(16), WizSmartScaleUI(16));

    layout->addItem(layoutTitle);
    title->setLayout(layout);
}

/**
 * @brief 初始化顶部主工具条
 */
void WizMainWindow::initToolBar()
{
    // m_toolBar will be added to titleBar
    layoutTitleBar();
    setFrameBorderWidth(1);

    // main button size
    QSize iconSize = QSize(WizSmartScaleUI(16), WizSmartScaleUI(16));
    m_toolBar->setIconSize(iconSize);
    m_toolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    m_toolBar->setMovable(false);
    //m_toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly); // 通过addWidget添加的QToolButton会忽视这个设置
    // align with categoryview's root item.
    m_toolBar->addWidget(new WizFixedSpacer(QSize(3, 1), m_toolBar));

    // 用户信息
    WizUserInfoWidget* info = new WizUserInfoWidget(*this, m_toolBar);
    setHitTestVisible(info);
    m_toolBar->addWidget(info);

    m_toolBar->addWidget(new WizFixedSpacer(QSize(5, 1), m_toolBar));
    // 同步按钮
    WizButton* buttonSync = new WizButton(m_toolBar);
    setHitTestVisible(buttonSync);
    buttonSync->setIconSize(QSize(16,16));
    buttonSync->setAction(m_actions->actionFromName(WIZACTION_GLOBAL_SYNC));
    m_toolBar->addWidget(buttonSync);

    m_spacerForToolButtonAdjust = new WizFixedSpacer(QSize(10, 1), m_toolBar);
    m_toolBar->addWidget(m_spacerForToolButtonAdjust);

    // 搜索栏，值得注意搜索栏的长度随着笔记列表宽度而变化
    m_searchWidget = new WizSearchView(this);
    setHitTestVisible(m_searchWidget);
    m_searchWidget->setFixedWidth(200);
    m_toolBar->addWidget(m_searchWidget);

    m_toolBar->addWidget(new WizFixedSpacer(QSize(10, 1), m_toolBar));

    /*
    // 前一篇文档
    WizButton* buttonBack = new WizButton(m_toolBar);
    buttonBack->setIconSize(iconSize);
    buttonBack->setAction(m_actions->actionFromName(WIZACTION_GLOBAL_GOBACK));
    m_toolBar->addWidget(buttonBack);
    // 后一篇文档
    WizButton* buttonForward = new WizButton(m_toolBar);
    buttonForward->setIconSize(iconSize);
    buttonForward->setAction(m_actions->actionFromName(WIZACTION_GLOBAL_GOFORWARD));
    m_toolBar->addWidget(buttonForward);
    */

    // 新建笔记[+]菜单
    prepareNewNoteMenu();

    QAction* newNoteAction = m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT);
    QToolButton* buttonNew = new QToolButton(m_toolBar);
    buttonNew->setObjectName("btn-newnote");
    setHitTestVisible(buttonNew);
    buttonNew->setMenu(m_newNoteExtraMenu);//选择模板的菜单
    buttonNew->setDefaultAction(newNoteAction);
    buttonNew->setPopupMode(QToolButton::MenuButtonPopup);
    buttonNew->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    buttonNew->setAutoRaise(true);

    m_toolBar->addWidget(buttonNew);
    m_toolBar->addWidget(new WizFixedSpacer(QSize(5, 1), m_toolBar));
    initToolBarPluginButtons();

    m_toolBar->addWidget(new WizSpacer(m_toolBar));

    updateHistoryButtonStatus();

    connect(m_searchWidget, SIGNAL(doSearch(const QString&)), SLOT(on_search_doSearch(const QString&)));
}


void WizMainWindow::initToolBarPluginButtons()
{
    JSPluginManager &jsPluginMgr = JSPluginManager::instance();

    auto menus = jsPluginMgr.modulesByModuleType("Menu");
    foreach (auto menuData, menus) {
        if (menuData->spec()->buttonLocation() != "Main")
            continue;
        auto btn = new QToolButton(this);
        btn->setPopupMode(QToolButton::MenuButtonPopup);
        QMenu *m = new QMenu(btn);
        foreach (int i, menuData->spec()->actionIndexes()) {
            auto parent = menuData->parentPlugin();
            auto acm = parent->module(i);
            QAction *ac = jsPluginMgr.createPluginAction(btn, acm);
            m->addAction(ac);
            connect(ac, &QAction::triggered, this,
                    [this, btn, ac] (bool checked) {
                        QRect rc = btn->rect();
                        QPoint pt = btn->mapToGlobal(QPoint(rc.width()/2, rc.height()));
                        Q_EMIT pluginPopupRequest(ac, pt);
                    }
            );
        }

        btn->setMenu(m);
        auto acs = m->actions();
        if (!acs.isEmpty()) {
            btn->setDefaultAction(acs.first());
            m_toolBar->addWidget(btn);
            setHitTestVisible(btn);
        }
    }

    QList<JSPluginModule *> modules = jsPluginMgr.modulesByKeyValue("ModuleType", "Action");
    for (auto moduleData : modules) {
        if (moduleData->spec()->buttonLocation() != "Main")
            continue;
        QAction *ac = jsPluginMgr.createPluginAction(m_toolBar, moduleData);
        connect(ac, &QAction::triggered, 
            &jsPluginMgr, &JSPluginManager::handlePluginActionTriggered);

        m_toolBar->addAction(ac);

        setHitTestVisible(m_toolBar->widgetForAction(ac));
    }

    connect(this, &WizMainWindow::pluginPopupRequest,
            &jsPluginMgr, &JSPluginManager::handlePluginPopupRequest);
}

/**
 * @brief 初始化客户端主体区域<目录树, 笔记列表, 文档页面>
 */
void WizMainWindow::initClient()
{

    setCentralWidget(rootWidget());

    QWidget* main = clientWidget();

    m_clienWgt = new QWidget(main);
    clientLayout()->addWidget(m_clienWgt);

    m_clienWgt->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    QPalette pal = m_clienWgt->palette();
    pal.setColor(QPalette::Window, QColor(Qt::transparent));
    pal.setColor(QPalette::Base, QColor(Qt::transparent));
    m_clienWgt->setPalette(pal);
    m_clienWgt->setAutoFillBackground(true);

    QVBoxLayout* layout = new QVBoxLayout(m_clienWgt);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    m_clienWgt->setLayout(layout);

    m_splitter = std::make_shared<WizSplitter>();
    layout->addWidget(m_splitter.get());

    // 绘制文件夹树
    pal.setColor(QPalette::Window, Utils::WizStyleHelper::treeViewBackground());
    m_category->setPalette(pal);
    m_category->setAutoFillBackground(true);

    // 绘制笔记浏览页面
    pal.setColor(QPalette::Window, QColor(Qt::white));
    pal.setColor(QPalette::Base, QColor(Qt::white));
    m_documentPanel = new QWidget(this); // 整个文档浏览界面板
    m_documentPanel->setObjectName("document-panel");
    m_documentPanel->setPalette(pal);
    m_documentPanel->setAutoFillBackground(true);
    QVBoxLayout* layoutDocument = new QVBoxLayout();
    layoutDocument->setContentsMargins(0, 0, 0, 0);
    layoutDocument->setSpacing(0);
    m_documentPanel->setLayout(layoutDocument);
    // WizMainTab
    layoutDocument->addWidget(m_mainTabBrowser); // 将主标签栏放在文档板布局上
    connect(m_mainTabBrowser, SIGNAL(currentChanged(int)), SLOT(on_mainTabWidget_currentChanged(int)));
    showHomePage();
    //
    layoutDocument->addWidget(m_documentSelection);
    m_documentSelection->hide();
    // append after client

    m_splitter->addWidget(m_category);

    // 创建文件列表容器
    m_docListContainer = new QWidget(this);
    m_docListContainer->setPalette(pal);
    m_docListContainer->setAutoFillBackground(true);
    QHBoxLayout* layoutList = new QHBoxLayout();
    layoutList->setContentsMargins(0, 0, 0, 0);
    layoutList->setSpacing(0);
    layoutList->addWidget(createNoteListView());
    layoutList->addWidget(createMessageListView());
    m_docListContainer->setLayout(layoutList);
    m_splitter->addWidget(m_docListContainer);
    m_splitter->addWidget(m_documentPanel);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 0);
    m_splitter->setStretchFactor(2, 1);

    m_msgListWidget->hide();

    connect(m_splitter.get(), SIGNAL(splitterMoved(int, int)), SLOT(on_client_splitterMoved(int, int)));

    bool show = m_settings->showLayoutCategoryView();
    m_category->setVisible(show);
    show = m_settings->showLayoutDocumentListView();
    m_docListContainer->setVisible(show);
    show = m_settings->showLayoutTabBrowser();
    m_documentPanel->setVisible(show);
}

/* Document list view with additional toolbar. **/
QWidget* WizMainWindow::createNoteListView()
{
    m_noteListWidget = new QWidget(this);
    //m_noteListWidget->setMinimumWidth(100);

    QVBoxLayout* layoutList = new QVBoxLayout();
    layoutList->setContentsMargins(0, 0, 0, 0);
    layoutList->setSpacing(0);
    m_noteListWidget->setLayout(layoutList);

    QPalette pal = m_noteListWidget->palette();
    pal.setColor(QPalette::Window, QColor("#F5F5F5"));
    pal.setColor(QPalette::Base, QColor("#F5F5F5"));
    m_noteListWidget->setPalette(pal);
    m_noteListWidget->setAutoFillBackground(true);

    // A Toolbar to control document list behavior.
    auto noteButtonsContainer = new QWidget(this);
    noteButtonsContainer->setObjectName("note-buttons-container");
    noteButtonsContainer->setFixedHeight(30);
    noteButtonsContainer->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* layoutActions = new QHBoxLayout();
    layoutActions->setContentsMargins(0, 0, 0, 0);
    layoutActions->setSpacing(0);
    noteButtonsContainer->setLayout(layoutActions);

    // Control document list view type.
    WizViewTypePopupButton* viewBtn = new WizViewTypePopupButton(*this, this);
    viewBtn->setObjectName("btn-view-type");
    connect(viewBtn, SIGNAL(viewTypeChanged(int)), SLOT(on_documents_viewTypeChanged(int)));
    connect(this, SIGNAL(documentsViewTypeChanged(int)), viewBtn, SLOT(on_viewTypeChanged(int)));
    layoutActions->addWidget(viewBtn);

    // Control document list sorting type.
    WizSortingPopupButton* sortBtn = new WizSortingPopupButton(*this, this);
    sortBtn->setObjectName("btn-sorting");
    connect(sortBtn, SIGNAL(sortingTypeChanged(int)), SLOT(on_documents_sortingTypeChanged(int)));
    connect(this, SIGNAL(documentsSortTypeChanged(int)), sortBtn, SLOT(on_sortingTypeChanged(int)));
    layoutActions->addWidget(sortBtn);
    layoutActions->addStretch(0);

    // A label to remind group user of unreading documents.
    m_labelDocumentsHint = new QLabel(this);
    m_labelDocumentsHint->setObjectName("label-documents-hint");
    m_labelDocumentsHint->setText(tr("Unread documents"));
    layoutActions->addWidget(m_labelDocumentsHint);
//    connect(m_category, SIGNAL(documentsHint(const QString&)), SLOT(on_documents_hintChanged(const QString&)));

//    m_labelDocumentsCount = new QLabel("", this);
//    m_labelDocumentsCount->setMargin(5);
//    layoutActions->addWidget(m_labelDocumentsCount);
//    connect(m_documents, SIGNAL(documentCountChanged()), SLOT(on_documents_documentCountChanged()));
    connect(m_documents, SIGNAL(changeUploadRequest(QString)), SLOT(on_quickSync_request(QString)));

    // A 'check' icon to mark all documents read
    m_btnMarkDocumentsReaded = new WizImageButton(this);
    m_btnMarkDocumentsReaded->setObjectName("btn-mark-documents-readed");
    QIcon btnIcon = ::WizLoadSkinIcon(Utils::WizStyleHelper::themeName(), "actionMarkMessagesRead");
    m_btnMarkDocumentsReaded->setIcon(btnIcon);
    m_btnMarkDocumentsReaded->setFixedSize(QSize(18, 18));
    m_btnMarkDocumentsReaded->setToolTip(tr("Mark all documents read"));
    connect(m_btnMarkDocumentsReaded, SIGNAL(clicked()), SLOT(on_btnMarkDocumentsRead_triggered()));
    layoutActions->addWidget(m_btnMarkDocumentsReaded);
    layoutActions->addSpacing(4);

    m_labelDocumentsHint->setVisible(false);
    m_btnMarkDocumentsReaded->setVisible(false);

    layoutList->addWidget(noteButtonsContainer);
    layoutList->addWidget(m_documents);

    return m_noteListWidget;
}

QWidget* WizMainWindow::createMessageListView()
{
    m_msgListWidget = new QWidget(this);
    //m_msgListWidget->setMinimumWidth(100);
    QVBoxLayout* layoutList = new QVBoxLayout();
    layoutList->setContentsMargins(0, 0, 0, 0);
    layoutList->setSpacing(0);
    m_msgListWidget->setLayout(layoutList);
    QPalette pal = m_msgListWidget->palette();
    pal.setColor(QPalette::Window, QColor("#F5F5F5"));
    pal.setColor(QPalette::Base, QColor("#F5F5F5"));
    m_msgListWidget->setPalette(pal);
    m_msgListWidget->setAutoFillBackground(true);

    m_msgListTitleBar = new WizMessageListTitleBar(*this, this);
    connect(m_msgListTitleBar, SIGNAL(messageSelector_senderSelected(QString)),
            SLOT(on_messageSelector_senderSelected(QString)));
    connect(m_msgListTitleBar, SIGNAL(markAllMessageRead_request(bool)),
            SLOT(on_actionMarkAllMessageRead_triggered(bool)));


    QHBoxLayout* titleBarLayout = new QHBoxLayout();
    titleBarLayout->setContentsMargins(0, 0, 0, 0);
    titleBarLayout->setSpacing(0);
    titleBarLayout->addWidget(m_msgListTitleBar);

    QHBoxLayout* layout2 = new QHBoxLayout();
    layout2->setContentsMargins(0, 0, 0, 0);
    layout2->setSpacing(0);
    titleBarLayout->addLayout(layout2);

    layoutList->addLayout(titleBarLayout);
    layoutList->addWidget(m_msgList);
    m_msgList->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    return m_msgListWidget;
}

QWidget*WizMainWindow::client() const
{
    WizDocumentView* docView = qobject_cast<WizDocumentView*>(m_mainTabBrowser->currentWidget());
    return docView->client();
}

WizDocumentView* WizMainWindow::documentView() const
{
    WizDocumentView* docView = qobject_cast<WizDocumentView*>(m_mainTabBrowser->currentWidget());
    return docView;
}

WizObjectDownloaderHost* WizMainWindow::downloaderHost() const
{
    return WizObjectDownloaderHost::instance();
}

WizIAPDialog*WizMainWindow::iapDialog()
{
#ifdef Q_OS_MAC
    if (m_iapDialog == 0) {
        m_iapDialog = new WizIAPDialog(this);
    }
    return m_iapDialog;
#else
    return 0;
#endif
}

/**
 * @brief Get ApiWizExplorerApp for the main window.
 * @return
 */
QObject* WizMainWindow::publicAPIsObject()
{
    return m_IWizExplorerApp;
}

//void MainWindow::on_documents_documentCountChanged()
//{
//    QString text;
//    int count = m_documents->documentCount();
//    if (count == 1)
//    {
//        text = tr("1 note");
//    }
//    else if (count > 1)
//    {
//        if (count >= 1000)
//        {
//            text = tr("%1 notes").arg("1000+");
//        }
//        else
//        {
//            text = tr("%1 notes").arg(count);
//        }
//    }
//    m_labelDocumentsCount->setText(text);
//}

/**
 * @brief 当笔记列表最后一个文档删除后，发出关闭笔记请求信号
 */
void WizMainWindow::on_documents_lastDocumentDeleted()
{
    //FIXME: 此处应该关闭标签页和释放当前视图的内存，在mainTab里添加一个槽函数用于关闭当前标签
    WizDocumentView* curDocView = qobject_cast<WizDocumentView*>(m_mainTabBrowser->currentWidget());
    if (curDocView) {
        WizGlobal::instance()->emitCloseNoteRequested(curDocView);
    }

}

void WizMainWindow::on_btnMarkDocumentsRead_triggered()
{
    m_btnMarkDocumentsReaded->setVisible(false);
    m_labelDocumentsHint->setVisible(false);
    //
    QSet<QString> setKb;
    for (int i = 0; i < m_documents->count(); i++)
    {
        if (WizDocumentListViewDocumentItem* item = m_documents->documentItemAt(i))
        {
            setKb.insert(item->document().strKbGUID);
        }
    }

    for (QString kb : setKb)
    {
        WizDatabase& db = m_dbMgr.db(kb);
        db.setGroupDocumentsReaded();
    }

    m_documents->clearAllItems();
}

//void MainWindow::on_documents_hintChanged(const QString& strHint)
//{
//    QFontMetrics fmx(font());
//    QString strMsg = fmx.elidedText(strHint, Qt::ElideRight, 150);
//    m_labelDocumentsHint->setText(strMsg);
//}

void WizMainWindow::on_documents_viewTypeChanged(int type)
{
    WizGetAnalyzer().logAction("DocumentsViewTypeChanged");
    m_documents->resetItemsViewType(type);

    setActionCheckState(m_viewTypeActions->actions(), type);
}

void WizMainWindow::on_documents_sortingTypeChanged(int type)
{
    WizGetAnalyzer().logAction("DocumentsSortTypeChanged");
    m_documents->resetItemsSortingType(type);

    setActionCheckState(m_sortTypeActions->actions(), type);
}

void WizMainWindow::init()
{
    connect(m_category, SIGNAL(itemSelectionChanged()), SLOT(on_category_itemSelectionChanged()));
    connect(m_category, SIGNAL(newDocument()), SLOT(on_actionNewNote_triggered()));
    connect(m_category, SIGNAL(categoryItemPositionChanged(QString)), SLOT(on_quickSync_request(QString)));
    connect(m_category, SIGNAL(unreadButtonClicked()), SLOT(on_categoryUnreadButton_triggered()));
    m_category->init();

    connect(m_msgList, SIGNAL(viewMessageRequest(WIZMESSAGEDATA)), SLOT(on_viewMessage_request(WIZMESSAGEDATA)));
    connect(m_msgList, SIGNAL(loacteDocumetRequest(QString,QString)), SLOT(locateDocument(QString,QString)));
    connect(m_msgList, SIGNAL(viewNoteInSparateWindowRequest(WIZDOCUMENTDATA)),
            SLOT(viewNoteInSeparateWindow(WIZDOCUMENTDATA)));
    connect(m_documents, SIGNAL(documentsSelectionChanged()), SLOT(on_documents_itemSelectionChanged()));
    connect(m_documents, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(on_documents_itemDoubleClicked(QListWidgetItem*)));
    //FIXME: 应该监听目录树列表|文档列表的文档删除信号，以便关闭已打开的标签
    connect(m_documents, SIGNAL(lastDocumentDeleted()), SLOT(on_documents_lastDocumentDeleted()));
    connect(m_documents, SIGNAL(shareDocumentByLinkRequest(QString,QString)),
            SLOT(on_shareDocumentByLink_request(QString,QString)));
    connect(m_documents, SIGNAL(loacteDocumetRequest(WIZDOCUMENTDATA)), SLOT(locateDocument(WIZDOCUMENTDATA)));

    connect(m_documents, SIGNAL(groupDocumentReadCountChanged(QString)), m_category,
            SLOT(on_groupDocuments_unreadCount_modified(QString)));

    QTimer::singleShot(100, this, SLOT(adjustToolBarLayout()));
    //
    QTimer::singleShot(1000 * 3, this, SLOT(on_actionSync_triggered()));
}

void WizMainWindow::on_actionAutoSync_triggered()
{
    m_syncFull->startSyncAll();
}

void WizMainWindow::on_actionSync_triggered()
{
    WizGetAnalyzer().logAction("ToolBarSyncAll");
    //
    if (m_animateSync->isPlaying())
    {
        on_actionConsole_triggered();
        return;
    }

//    if (::WizIsOffline())
//    {
//        QMessageBox::information(this, tr("Info"), tr("Connection is not available, please check your network connection."));
//    }
//    else
//    {
        syncAllData();
//    }
}

void WizMainWindow::on_syncLogined()
{
    // FIXME: show user notify message send from server
}

void WizMainWindow::on_syncStarted(bool syncAll)
{
    if (!m_animateSync->isPlaying())
    {
        m_animateSync->startPlay();
    }

    if (syncAll)
    {
        qDebug() << "[Sync] Syncing all notes...";
    }
    else
    {
        qDebug() << "[Sync] Quick syncing notes...";
    }
}

void WizMainWindow::on_syncDone(int nErrorCode, bool isNetworkError, const QString& strErrorMsg, bool isBackground)
{
    m_animateSync->stopPlay();

    if (isXMLRpcErrorCodeRelatedWithUserAccount(nErrorCode))
    {
        qDebug() << "sync done reconnectServer";
        reconnectServer();
        return;
    }
    else if (S_OK == nErrorCode)
    {
        // set quick download message enable
        m_bQuickDownloadMessageEnable = true;
    }
    else if (WIZKM_XMLRPC_ERROR_BIZ_SERVICE_EXPR == nErrorCode && !isBackground)
    {
        //当用户的企业付费到期并且有待上传的内容的时候，进行弹框提示
        WizMessageBox::information(this, tr("Info"), strErrorMsg);
    }
    else
    {
        QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Critical;
        int delay = 30 * 1000;
        QVariant param(isNetworkError);

        if (isNetworkError) {
            m_tray->showMessage(tr("Sync failed"), tr("Bad network connection, can not sync now. Please try again later. (code: %1)").arg(nErrorCode), icon, delay, param);
            return;
        } else {
            QString message = tr("There is something wrong with sync service. Please try again later. (code: %1)").arg(nErrorCode);
            if (WIZKM_XMLRPC_ERROR_VIP_SERVICE_EXPR == nErrorCode) {
                message = QObject::tr("VIP service of has expired, please renew to VIP.");
                param = QVariant((int)nErrorCode);
            } else if (WIZKM_XMLRPC_ERROR_FREE_SERVICE_EXPR == nErrorCode) {
                message = QObject::tr("User service of has expired, please upgrade to VIP.");
                param = QVariant((int)nErrorCode);
            } else if (WIZKM_XMLRPC_ERROR_BIZ_SERVICE_EXPR == nErrorCode) {
                message = WizFormatString0(QObject::tr("Your {p} business service has expired."));
            } else if (WIZKM_XMLRPC_ERROR_NOTE_COUNT_LIMIT == nErrorCode) {
                message = WizFormatString0(QObject::tr("Group notes count limit exceeded!"));
            }

            m_tray->showMessage(tr("Sync failed"), message, icon, delay, param);
            return;
        }
    }

    m_documents->viewport()->update();
    m_category->updateGroupsData();
    m_category->viewport()->update();
}

void WizMainWindow::on_syncDone_userVerified()
{

    if (m_dbMgr.db().setPassword(m_userVerifyDialog->password())) {
        m_syncFull->clearCurrentToken();
        m_syncQuick->clearCurrentToken();
        syncAllData();
    }
}

void WizMainWindow::on_syncProcessLog(const QString& strMsg)
{
    Q_UNUSED(strMsg);
}

void WizMainWindow::on_promptMessage_request(int nType, const QString& strTitle, const QString& strMsg)
{
    switch (nType) {
    case wizSyncMessageNormal:
        WizMessageBox::information(this, strTitle.isEmpty() ? tr("Info") : strTitle, strMsg);
        break;
    case wizSyncMessageWarning:
        WizMessageBox::warning(this, strTitle.isEmpty() ? tr("Info") : strTitle, strMsg);
        break;
    case wizSyncMeesageError:
        WizMessageBox::critical(this, strTitle.isEmpty() ? tr("Info") : strTitle, strMsg);
        break;
    default:
        break;
    }
}



void WizMainWindow::promptServiceExpr(bool free, WIZGROUPDATA group)
{
    static int lastPrompt = 0;
    if (lastPrompt != 0)
    {
        int now = WizGetTickCount();
        int span = now - lastPrompt;
        if (span < 60 * 60 * 1000)
            return;
    }
    //
    static bool in = false;
    if (in)
        return;
    //
    in = true;
    //
    lastPrompt = WizGetTickCount();

    WizDatabase& db = m_dbMgr.db("");
    bool isBizUser = db.hasBiz();
    //
    WizUserServiceExprDialog dlg(NULL);
    dlg.setUserInfo(free, isBizUser, group);
    if (0 != dlg.exec() && !group.isGroup())
    {
        showVipUpgradePage();
    }
    in  = false;
}

void WizMainWindow::on_promptFreeServiceExpr(WIZGROUPDATA group)
{
    promptServiceExpr(true, group);
}


void WizMainWindow::on_promptVipServiceExpr(WIZGROUPDATA group)
{
    promptServiceExpr(false, group);
}


void WizMainWindow::on_bubbleNotification_request(const QVariant& param)
{
    m_tray->showMessage(param);
}

void WizMainWindow::on_actionNewNote_triggered()
{
    WizGetAnalyzer().logAction("newNote");

    initVariableBeforCreateNote();
    WIZDOCUMENTDATA data;
    if (!m_category->createDocument(data))
    {
        return;
    }

    setFocusForNewNote(data);
    m_history->addHistory(data);
}

void WizMainWindow::on_actionNewNoteByTemplate_triggered()
{
    WizGetAnalyzer().logAction("newNoteByTemplate");

    //通过模板创建笔记
    WizDocTemplateDialog dlg(m_dbMgr);
    connect(&dlg, SIGNAL(documentTemplateSelected(TemplateData)), SLOT(createNoteByTemplate(TemplateData)));
    connect(&dlg, SIGNAL(upgradeVipRequest()), SLOT(showVipUpgradePage()));
    dlg.exec();
}

void WizMainWindow::on_actionEditingUndo_triggered()
{
    WizDocumentView* docView = currentDocumentView();
    if (WizCodeEditorDialog::undo())
        return;
    //
    if (docView->commentView()->hasFocus())
    {
        docView->commentView()->triggerPageAction(QWebEnginePage::Undo);
    }
    else
    {
        getActiveEditor()->undo();
    }
}

void WizMainWindow::on_actionEditingRedo_triggered()
{
    WizDocumentView* docView = currentDocumentView();
    if (docView->commentView()->hasFocus())
    {
        docView->commentView()->triggerPageAction(QWebEnginePage::Redo);
    }
    else
    {
        getActiveEditor()->redo();
    }
}

void WizMainWindow::on_actionEditingCut_triggered()
{
    WizDocumentView* docView = currentDocumentView();
    if (WizCodeEditorDialog::cut())
        return;
    //
    if (docView->commentView()->hasFocus())
    {
        docView->commentView()->triggerPageAction(QWebEnginePage::Cut);
    }
    else
    {
        getActiveEditor()->triggerPageAction(QWebEnginePage::Cut);
    }
}

void WizMainWindow::on_actionEditingCopy_triggered()
{
    WizDocumentView* docView = currentDocumentView();
    if (WizCodeEditorDialog::copy())
        return;
    //
    if (docView->commentView()->hasFocus())
    {
        docView->commentView()->triggerPageAction(QWebEnginePage::Copy);
    }
    else
    {
        getActiveEditor()->triggerPageAction(QWebEnginePage::Copy);
    }
}

void WizMainWindow::on_actionEditingPaste_triggered()
{
    WizDocumentView* docView = currentDocumentView();
    if (WizCodeEditorDialog::paste())
        return;
    //
    if (docView->commentView()->hasFocus())
    {
        docView->commentView()->triggerPageAction(QWebEnginePage::Paste);
    }
    else
    {
        getActiveEditor()->triggerPageAction(QWebEnginePage::Paste);
    }
}

void WizMainWindow::on_actionEditingPastePlain_triggered()
{
    WizDocumentView* docView = currentDocumentView();
    if (!docView)
        return;
    if (docView->commentView() && docView->commentView()->hasFocus())
    {
        docView->commentView()->triggerPageAction(QWebEnginePage::Paste);
    }
    else
    {
        getActiveEditor()->editorCommandExecutePastePlainText();
    }
}

void WizMainWindow::on_actionEditingSelectAll_triggered()
{
    WizDocumentView* docView = currentDocumentView();
    if (WizCodeEditorDialog::selectAll())
        return;
    //
    if (docView->commentView()->hasFocus())
    {
        docView->commentView()->triggerPageAction(QWebEnginePage::SelectAll);
    }
    else
    {
        getActiveEditor()->triggerPageAction(QWebEnginePage::SelectAll);
    }
}


void WizMainWindow::on_actionEditingDelete_triggered()
{
    qDebug() << "delete...";
}


void WizMainWindow::on_actionViewToggleCategory_triggered()
{
    static bool off = true;

    if (off) {
        if (userSettings().showLayoutCategoryView())
            on_actionLayoutCategoryView_triggered();
        if (userSettings().showLayoutDocumentListView())
            on_actionLayoutDocumentListView_triggered();
    } else {
        if (!userSettings().showLayoutCategoryView())
            on_actionLayoutCategoryView_triggered();
        if (!userSettings().showLayoutDocumentListView())
            on_actionLayoutDocumentListView_triggered();
    }

    m_actions->toggleActionText(WIZACTION_GLOBAL_TOGGLE_CATEGORY);
    off = !off;
}

void WizMainWindow::on_actionLayoutCategoryView_triggered()
{
    bool show = !userSettings().showLayoutCategoryView();
    userSettings().setShowLayoutCategoryView(show);

    m_category->setVisible(show);
    m_actions->actionFromName(WIZLAYOUT_CATEGORY_VIEW)->setChecked(show);
}

void WizMainWindow::on_actionLayoutDocumentListView_triggered()
{
    bool show = !userSettings().showLayoutDocumentListView();
    userSettings().setShowLayoutDocumentListView(show);

    m_docListContainer->setVisible(show);
    m_actions->actionFromName(WIZLAYOUT_DOCUMENTLIST_VIEW)->setChecked(show);
}

void WizMainWindow::on_actionLayoutTabBrowser_triggered()
{
    bool show = !userSettings().showLayoutTabBrowser();
    userSettings().setShowLayoutTabBrowser(show);

    m_documentPanel->setVisible(show);
    m_actions->actionFromName(WIZLAYOUT_TAB_BROWSER)->setChecked(show);
}

void WizMainWindow::on_actionViewShowSubFolderDocuments_triggered()
{
    bool show = !userSettings().showSubFolderDocuments();
    userSettings().setShowSubFolderDocuments(show);
    on_category_itemSelectionChanged();

    actions()->actionFromName(WIZACTION_GLOBAL_SHOW_SUBFOLDER_DOC)->setChecked(show);
}

#ifdef Q_OS_MAC
void WizMainWindow::on_actionViewToggleClientFullscreen_triggered()
{
    toggleFullScreenMode(this);
}
#endif // Q_OS_MAC

void WizMainWindow::on_actionViewToggleFullscreen_triggered()
{
    WizGetAnalyzer().logAction("MenuBarFullscreen");

    m_mainTabBrowser->triggeredFullScreen();
}

void WizMainWindow::on_actionViewMinimize_triggered()
{
    WizGetAnalyzer().logAction("MenuBarMinimize");

    QWidget* wgt = qApp->activeWindow();
    if (wgt == 0)
        return;

    wgt->setWindowState(wgt->windowState() | Qt::WindowMinimized);
}

void WizMainWindow::on_actionZoom_triggered()
{
    WizGetAnalyzer().logAction("MenuBarZoom");
    QWidget* wgt = qApp->activeWindow();
    if (!wgt)
        return;

    if (wgt->windowState() & Qt::WindowMaximized)
    {
        wgt->setWindowState(wgt->windowState() & ~Qt::WindowMaximized);
    }
    else
    {
        wgt->setWindowState(wgt->windowState() | Qt::WindowMaximized);
    }
}

void WizMainWindow::on_actionBringFront_triggered()
{
    WizGetAnalyzer().logAction("MenuBarBringFront");
#ifdef Q_OS_MAC
    wizMacShowCurrentApplication();

    if (!isVisible())
    {
        bringWidgetToFront(this);
    }


#endif
//    QWindowList widgetList = qApp->allWindows();
//    for (QWindow* wgt : widgetList)
//    {
//        wgt->setVisible(true);
//    }
}

void WizMainWindow::on_actionOpenWelcomePage_triggered()
{
    showHomePage();
}

void WizMainWindow::on_actionCategoryMessageCenter_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        m_category->setSectionVisible(Section_MessageCenter, action->isChecked());
    }
}

void WizMainWindow::on_actionCategoryShortcuts_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        m_category->setSectionVisible(Section_Shortcuts, action->isChecked());
    }
}

void WizMainWindow::on_actionCategoryQuickSearch_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        m_category->setSectionVisible(Section_QuickSearch, action->isChecked());
    }
}

void WizMainWindow::on_actionCategoryFolders_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        m_category->setSectionVisible(Section_Folders, action->isChecked());
    }
}

void WizMainWindow::on_actionCategoryTags_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        m_category->setSectionVisible(Section_Tags, action->isChecked());
    }
}

void WizMainWindow::on_actionCategoryBizGroups_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        m_category->setSectionVisible(Section_BizGroups, action->isChecked());
    }
}

void WizMainWindow::on_actionCategoryPersonalGroups_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        m_category->setSectionVisible(Section_PersonalGroups, action->isChecked());
    }
}

void WizMainWindow::on_actionThumbnailView_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        int type = action->data().toInt();
        m_documents->resetItemsViewType(type);
        emit documentsViewTypeChanged(type);
    }
}

void WizMainWindow::on_actionSearchResultView_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        int type = action->data().toInt();
        m_documents->resetItemsViewType(type);
        emit documentsViewTypeChanged(type);
    }
}

void WizMainWindow::on_actionTwoLineView_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        int type = action->data().toInt();
        m_documents->resetItemsViewType(type);
        emit documentsViewTypeChanged(type);
    }
}

void WizMainWindow::on_actionOneLineView_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        int type = action->data().toInt();
        m_documents->resetItemsViewType(type);
        emit documentsViewTypeChanged(type);
    }
}

void WizMainWindow::on_actionSortByCreatedTime_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    changeDocumentsSortTypeByAction(action);
}

void WizMainWindow::on_actionSortByUpdatedTime_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    changeDocumentsSortTypeByAction(action);
}

void WizMainWindow::on_actionSortByAccessTime_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    changeDocumentsSortTypeByAction(action);
}

void WizMainWindow::on_actionSortByTitle_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    changeDocumentsSortTypeByAction(action);
}

void WizMainWindow::on_actionSortByFolder_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    changeDocumentsSortTypeByAction(action);
}

void WizMainWindow::on_actionSortBySize_triggered()
{
    QAction* action = qobject_cast<QAction*>(sender());
    changeDocumentsSortTypeByAction(action);
}

void WizMainWindow::on_actionSkinReloadStyleSheet_triggered()
{
    qApp->setStyleSheet(WizLoadSkinStyleSheet(userSettings().skin()));
}

#define MARKDOCUMENTSREADCHECKED       "MarkDocumentsReadedChecked"
#include <functional>

std::function<void()> tipBindFunction = [](){
    if (WizMainWindow* mainWindow = WizMainWindow::instance())
    {
        mainWindow->userSettings().set(MARKDOCUMENTSREADCHECKED, "1");
    }
};

void WizMainWindow::on_categoryUnreadButton_triggered()
{
    m_btnMarkDocumentsReaded->setVisible(true);
    m_labelDocumentsHint->setVisible(true);

    bool showTips = userSettings().get(MARKDOCUMENTSREADCHECKED).toInt() == 0;
    if (showTips && !WizTipsWidget::isTipsExists(MARKDOCUMENTSREADCHECKED))
    {
        WizTipsWidget* tipWidget = new WizTipsWidget(MARKDOCUMENTSREADCHECKED, this);
        connect(m_btnMarkDocumentsReaded, SIGNAL(clicked(bool)), tipWidget, SLOT(on_targetWidgetClicked()));
        tipWidget->setAttribute(Qt::WA_DeleteOnClose, true);
        tipWidget->setText(tr("Mark all as readed"), tr("Mark all documents as readed."));
        tipWidget->setSizeHint(QSize(280, 60));
        tipWidget->setButtonVisible(false);
        tipWidget->bindCloseFunction(tipBindFunction);
        //
        tipWidget->bindTargetWidget(m_btnMarkDocumentsReaded, 0, 2);
        tipWidget->on_showRequest();
    }
}

void WizMainWindow::on_actionMarkAllMessageRead_triggered(bool removeItems)
{
    WizGetAnalyzer().logAction("markAllMessagesRead");

    m_msgList->markAllMessagesReaded(removeItems);
}

void WizMainWindow::on_messageSelector_senderSelected(QString userGUID)
{
    WizGetAnalyzer().logAction("messageSelector");
    loadMessageByUserGuid(userGUID);
}

void WizMainWindow::on_actionMenuFormatJustifyLeft_triggered()
{
    WizGetAnalyzer().logAction("MenuBarJustifyLeft");
    getActiveEditor()->editorCommandExecuteJustifyLeft();
}

void WizMainWindow::on_actionMenuFormatJustifyRight_triggered()
{
    WizGetAnalyzer().logAction("MenuBarJustifyRight");
    getActiveEditor()->editorCommandExecuteJustifyRight();
}

void WizMainWindow::on_actionMenuFormatJustifyCenter_triggered()
{
    WizGetAnalyzer().logAction("MenuBarJustifyCenter");
    getActiveEditor()->editorCommandExecuteJustifyCenter();
}

void WizMainWindow::on_actionMenuFormatJustifyJustify_triggered()
{
    WizGetAnalyzer().logAction("MenuBarJustifyJustify");
    getActiveEditor()->editorCommandExecuteJustifyJustify();
}

void WizMainWindow::on_actionMenuFormatInsertOrderedList_triggered()
{
    WizGetAnalyzer().logAction("MenuBarOrderedList");
    getActiveEditor()->editorCommandExecuteInsertOrderedList();
}

void WizMainWindow::on_actionMenuFormatInsertUnorderedList_triggered()
{
    WizGetAnalyzer().logAction("MenuBarUnorderedList");
    getActiveEditor()->editorCommandExecuteInsertUnorderedList();
}


void WizMainWindow::on_actionMenuFormatInsertLink_triggered()
{
    WizGetAnalyzer().logAction("MenuBarInsertLink");
    getActiveEditor()->editorCommandExecuteLinkInsert();
}

void WizMainWindow::on_actionMenuFormatBold_triggered()
{
    WizGetAnalyzer().logAction("MenuBarBold");
    getActiveEditor()->editorCommandExecuteBold();
}

void WizMainWindow::on_actionMenuFormatItalic_triggered()
{
    WizGetAnalyzer().logAction("MenuBarItalic");
    getActiveEditor()->editorCommandExecuteItalic();
}

void WizMainWindow::on_actionMenuFormatUnderLine_triggered()
{
    WizGetAnalyzer().logAction("MenuBarUnderLine");
    getActiveEditor()->editorCommandExecuteUnderLine();
}

void WizMainWindow::on_actionMenuFormatStrikeThrough_triggered()
{
    WizGetAnalyzer().logAction("MenuBarStrikeThrough");
    getActiveEditor()->editorCommandExecuteStrikeThrough();
}

void WizMainWindow::on_actionMenuFormatSubscript_triggered()
{
    WizGetAnalyzer().logAction("MenuBarSubscript");
    getActiveEditor()->editorCommandExecuteSubScript();
}

void WizMainWindow::on_actionMenuFormatSuperscript_triggered()
{
    WizGetAnalyzer().logAction("MenuBarSuperscript");
    getActiveEditor()->editorCommandExecuteSuperScript();
}

void WizMainWindow::on_actionMenuFormatInsertHorizontal_triggered()
{
    WizGetAnalyzer().logAction("MenuBarInsertHorizontal");
    getActiveEditor()->editorCommandExecuteInsertHorizontal();
}

void WizMainWindow::on_actionMenuFormatInsertDate_triggered()
{
    WizGetAnalyzer().logAction("MenuBarInsertDate");
    getActiveEditor()->editorCommandExecuteInsertDate();
}

void WizMainWindow::on_actionMenuFormatInsertTime_triggered()
{
    WizGetAnalyzer().logAction("MenuBarInsertTime");
    getActiveEditor()->editorCommandExecuteInsertTime();
}

void WizMainWindow::on_actionMenuFormatInsertTable(int row, int col)
{
    ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{

        WizGetAnalyzer().logAction("MenuBarInsertTable");
        getActiveEditor()->editorCommandExecuteTableInsert(row, col);

    });
}

void WizMainWindow::on_actionMenuFormatIndent_triggered()
{
    WizGetAnalyzer().logAction("MenuBarIndent");
    getActiveEditor()->editorCommandExecuteIndent();
}

void WizMainWindow::on_actionMenuFormatOutdent_triggered()
{
    WizGetAnalyzer().logAction("MenuBarOutdent");
    getActiveEditor()->editorCommandExecuteOutdent();
}

void WizMainWindow::on_actionMenuFormatRemoveFormat_triggered()
{
    WizGetAnalyzer().logAction("MenuBarRemoveFormat");
    getActiveEditor()->editorCommandExecuteRemoveFormat();
}

void WizMainWindow::on_actionMenuFormatInsertCheckList_triggered()
{
    WizGetAnalyzer().logAction("MenuBarInsertCheckList");
    getActiveEditor()->editorCommandExecuteInsertCheckList();
}

void WizMainWindow::on_actionMenuFormatInsertCode_triggered()
{
    WizGetAnalyzer().logAction("MenuBarInsertCode");
    getActiveEditor()->editorCommandExecuteInsertCode();
}

void WizMainWindow::on_actionMenuFormatInsertImage_triggered()
{
    WizGetAnalyzer().logAction("MenuBarInsertImage");
    getActiveEditor()->editorCommandExecuteInsertImage();
}

void WizMainWindow::on_actionMenuFormatScreenShot_triggered()
{
    WizGetAnalyzer().logAction("MenuBarScreenShot");
    getActiveEditor()->editorCommandExecuteScreenShot();
}

void WizMainWindow::on_actionConsole_triggered()
{
    if (!m_console) {
      m_console = new WizConsoleDialog(*this, window());
    }

    m_console->show();
    m_console->raise();

    WizGetAnalyzer().logAction("MenuBarConsole");
}

void WizMainWindow::on_actionLogout_triggered()
{
    WizGetAnalyzer().logAction("MenuBarLogout");

    // save state
    m_settings->setAutoLogin(false);
    m_bLogoutRestart = true;
    on_actionExit_triggered();
}

void WizMainWindow::on_actionAbout_triggered()
{
    WizGetAnalyzer().logAction("MenuBarAboutWiz");

    WizAboutDialog dialog(this);
    dialog.exec();
}

void WizMainWindow::on_actionPreference_triggered()
{
    WizGetAnalyzer().logAction("MenuBarPreference");

    WizPreferenceWindow preference(*this, this);

    connect(&preference, SIGNAL(settingsChanged(WizOptionsType)), SLOT(on_options_settingsChanged(WizOptionsType)));
    connect(&preference, SIGNAL(restartForSettings()), SLOT(on_options_restartForSettings()));
    preference.exec();
}

void WizMainWindow::on_actionFeedback_triggered()
{
    QString strUrl = WizOfficialApiEntry::standardCommandUrl("feedback");

    if (strUrl.isEmpty())
        return;

    //FIXME: special handle for support.html, shuold append displayName in url.
    WizDatabase& personDb = m_dbMgr.db();
    QString strUserName = "Unkown";
    personDb.getUserDisplayName(strUserName);
    strUrl.replace(QHostInfo::localHostName(), QUrl::toPercentEncoding(strUserName));

    QDesktopServices::openUrl(strUrl);
    WizGetAnalyzer().logAction("MenuBarFeedback");
}

void WizMainWindow::on_actionSupport_triggered()
{
    QString strUrl = WizOfficialApiEntry::standardCommandUrl("support");

    if (strUrl.isEmpty())
        return;

    QDesktopServices::openUrl(strUrl);

    WizGetAnalyzer().logAction("MenuBarSupport");
}

void WizMainWindow::on_actionManual_triggered()
{
    QString strUrl = WizOfficialApiEntry::standardCommandUrl("link");

    if (strUrl.isEmpty())
        return;

    strUrl += "&site=www&name=manual/mac/index.html";
    QDesktopServices::openUrl(strUrl);

    WizGetAnalyzer().logAction("MenuBarManual");
}

void WizMainWindow::on_actionSearch_triggered()
{
    m_searchWidget->focus();

    WizGetAnalyzer().logAction("MenuBarSearch");
}

void WizMainWindow::resetSearchStatus()
{
    quitSearchStatus();
    m_searchWidget->clear();
    m_category->restoreSelection();
}

void WizMainWindow::on_actionDownloadManager_triggered()
{
    DownloadManagerWidget::instance().show();
}

void WizMainWindow::on_actionJSConsole_triggered()
{
    auto repl = new JSRepl({{"WizExplorerApp", publicAPIsObject()}});
    repl->show();
}

void WizMainWindow::on_actionResetSearch_triggered()
{
    resetSearchStatus();
    m_searchWidget->focus();
    //
    WizGetAnalyzer().logAction("MenuBarResetSearch");
}

void WizMainWindow::on_actionFindReplace_triggered()
{
    WizDocumentWebView *webView = getActiveEditor();
    if (!webView)
        return;
    webView->editorCommandExecuteFindReplace();
    WizGetAnalyzer().logAction("MenuBarFindReplace");
}

void WizMainWindow::on_actionSaveAsPDF_triggered()
{
    if (WizDocumentWebView* editor = getActiveEditor())
    {
        editor->saveAsPDF();
    }
    WizGetAnalyzer().logAction("MenuBarSaveAsPDF");
}

void WizMainWindow::on_actionSaveAsHtml_triggered()
{
    if (WizDocumentWebView* editor = getActiveEditor())
    {
        editor->saveAsHtml();
    }
    WizGetAnalyzer().logAction("MenuBarSaveAsHtml");
}


void WizMainWindow::on_actionSaveAsMarkdown_triggered()
{
    if (WizDocumentWebView* editor = getActiveEditor())
    {
        editor->saveAsMarkdown();
    }
    WizGetAnalyzer().logAction("MenuBarSaveAsMarkdown");
}

void WizMainWindow::on_actionImportFile_triggered()
{
    if (m_category)
    {
        m_category->on_action_importFile();
    }
    WizGetAnalyzer().logAction("MenuBarImportFile");
}

void WizMainWindow::on_actionExportFile_triggered()
{
    FileExportWizard dialog(*this, this);
    dialog.exec();
    WizGetAnalyzer().logAction("MenuBarExportFile");
}

void WizMainWindow::on_actionPrintMargin_triggered()
{
    WizPreferenceWindow preference(*this, this);
    preference.showPrintMarginPage();
    connect(&preference, SIGNAL(settingsChanged(WizOptionsType)), SLOT(on_options_settingsChanged(WizOptionsType)));
    connect(&preference, SIGNAL(restartForSettings()), SLOT(on_options_restartForSettings()));
    preference.exec();
    WizGetAnalyzer().logAction("MenuBarPrintMargin");
}

//void MainWindow::on_searchDocumentFind(const WIZDOCUMENTDATAEX& doc)
//{
//    m_documents->addDocument(doc, true);
//    on_documents_itemSelectionChanged();
//}

void WizMainWindow::on_search_doSearch(const QString& keywords)
{
    m_category->saveSelection();
    //
    QString kbGuid = m_category->storedSelectedItemKbGuid();
    m_searchWidget->setCurrentKb(kbGuid);
    //
    m_strSearchKeywords = keywords;
    if (keywords.isEmpty()) {
        resetSearchStatus();
        return;
    }
    //
    if (IsWizKMURL(keywords)) {
        QString strUrl = keywords;
        strUrl.remove("\n");
        viewDocumentByWizKMURL(strUrl);
        return;
    } else if (IsHttpURL(keywords)) {
        QString strUrl = keywords;
        strUrl.remove("\n");
        m_mainTabBrowser->createTab(QUrl::fromUserInput(strUrl));
        return;
    }
    //
    m_noteListWidget->show();
    m_msgListWidget->hide();
    //
    m_settings->appendRecentSearch(keywords);
    //m_searcher->search(keywords, 500);
    startSearchStatus();
    //
    QString key = keywords;
    //
    ::WizExecutingActionDialog::executeAction(tr("Searching..."), WIZ_THREAD_SEARCH, [=]{

        CWizDocumentDataArray arrayDocument;
        m_searcher->onlineSearch(kbGuid, key, arrayDocument);
        //
        ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{

            m_documents->clearAllItems();
            m_documents->setDocuments(arrayDocument, true);
            //
        });
    });
    //
}


void WizMainWindow::on_searchProcess(const QString& strKeywords, const CWizDocumentDataArray& arrayDocument, bool bStart, bool bEnd)
{
    if (bStart) {
        m_documents->setLeadInfoState(DocumentLeadInfo_SearchResult);
        m_documents->setDocuments(arrayDocument);
    } else {
        m_documents->appendDocuments(arrayDocument);
    }
    on_documents_itemSelectionChanged();
}

#ifndef Q_OS_MAC
void WizMainWindow::on_actionPopupMainMenu_triggered()
{
    QAction* pAction = m_actions->actionFromName("actionPopupMainMenu");
    QRect rc = m_toolBar->actionGeometry(pAction);
    QPoint pt = m_toolBar->mapToGlobal(QPoint(rc.left(), rc.bottom()));

    WizSettings settings(Utils::WizPathResolve::resourcesPath() + "files/mainmenu.ini");

    QMenu* pMenu = new QMenu(this);
    m_actions->buildMenu(pMenu, settings, pAction->objectName(), false);

    pMenu->popup(pt);
}

void WizMainWindow::on_menuButtonClicked()
{
    QWidget* wgt = qobject_cast<QWidget*>(sender());
    if (wgt)
    {
        QPoint popupPoint = clientWidget()->mapToGlobal(
            QPoint(wgt->pos().x(), wgt->pos().y() + wgt->height()));
        m_menu->popup(popupPoint);
    }
}

#endif

void WizMainWindow::on_client_splitterMoved(int pos, int index)
{
#ifndef Q_OS_MAC
    adjustToolBarLayout();
#endif
}

void WizMainWindow::on_actionGoBack_triggered()
{
    WizDocumentView* docView = currentDocumentView();
    WizGetAnalyzer().logAction("ToolBarGoBack");

    if (!m_history->canBack())
        return;

    WIZDOCUMENTDATA data = m_history->back();
    WizDatabase &db = m_dbMgr.db(data.strKbGUID);
    if (db.documentFromGuid(data.strGUID, data) && !db.isInDeletedItems(data.strLocation))
    {
        viewDocument(data, false);
        if (m_documents->isVisible())
        {
            locateDocument(data);
        }
    }
    else
    {
        on_actionGoBack_triggered();
    }

    updateHistoryButtonStatus();
    docView->setFocus();
}

void WizMainWindow::on_actionGoForward_triggered()
{
    WizDocumentView* docView = currentDocumentView();
    WizGetAnalyzer().logAction("ToolBarGoForward");

    if (!m_history->canForward())
        return;

    WIZDOCUMENTDATA data = m_history->forward();
    WizDatabase &db = m_dbMgr.db(data.strKbGUID);
    if (db.documentFromGuid(data.strGUID, data) && !db.isInDeletedItems(data.strLocation))
    {
        viewDocument(data, false);
        if (m_documents->isVisible())
        {
            locateDocument(data);
        }
    }
    else
    {
        on_actionGoForward_triggered();
    }

    updateHistoryButtonStatus();
    docView->setFocus();
}

void WizMainWindow::on_category_itemSelectionChanged()
{
    WizCategoryBaseView* category = m_category;
    if (!category)
        return;
    quitSearchStatus();
    /*
     * 在点击MessageItem的时候,为了重新刷新当前消息,强制发送了itemSelectionChanged消息
     * 因此需要在这个地方避免重复刷新两次消息列表
     */
    if (!category->currentItem())
        return;

    static QTime lastTime(0, 0, 0);
    QTreeWidgetItem *currentItem = category->currentItem();
    static QTreeWidgetItem *oldItem = currentItem;
    QTime last = lastTime;
    QTime now = QTime::currentTime();
    lastTime = now;
    if (last.msecsTo(now) < 300 && oldItem == currentItem) {
        return;
    } else {
        oldItem = currentItem;
    }
    //
    if (WizCategoryViewTrashItem* pItem = dynamic_cast<WizCategoryViewTrashItem *>(currentItem))
    {
        m_category->on_action_deleted_recovery();
        return;
    }

    QTreeWidgetItem* categoryItem = category->currentItem();
    switch (categoryItem->type()) {
    case Category_MessageItem:
    {
        WizCategoryViewMessageItem* pItem = dynamic_cast<WizCategoryViewMessageItem*>(categoryItem);
        if (pItem)
        {
            showMessageList(pItem);
            //
            m_syncFull->quickDownloadMesages();
            WizGetAnalyzer().logAction("categoryMessageRootSelected");
        }
    }
        break;
    case Category_ShortcutItem:
    {
        WizCategoryViewShortcutItem* pShortcut = dynamic_cast<WizCategoryViewShortcutItem*>(categoryItem);
        if (pShortcut)
        {
            viewDocumentByShortcut(pShortcut);
            WizGetAnalyzer().logAction("categoryShortcutItem");
        }
    }
        break;
    case Category_QuickSearchItem:
    {
        WizCategoryViewSearchItem* pSearchItem = dynamic_cast<WizCategoryViewSearchItem*>(categoryItem);
        if (pSearchItem)
        {
            searchNotesBySQL(pSearchItem->getSQLWhere());
            WizGetAnalyzer().logAction("categoryBuildInQuickSearchItem");
        }
    }
        break;
    case Category_QuickSearchCustomItem:
    {
        WizCategoryViewCustomSearchItem* pSearchItem = dynamic_cast<WizCategoryViewCustomSearchItem*>(categoryItem);
        if (pSearchItem)
        {
            searchNotesBySQLAndKeyword(pSearchItem->getSQLWhere(), pSearchItem->getKeyword(), pSearchItem->searchScope());
            WizGetAnalyzer().logAction("categoryCustomQuickSearchItem");
        }
    }
        break;
    default:
        showDocumentList(category);
        break;
    }
    //
    QString kbGuid = m_category->selectedItemKbGUID();
    m_searchWidget->setCurrentKb(kbGuid);
}

/**
 * @brief 处理文档列表选区变化信号，如果单选则浏览该文档
 */
void WizMainWindow::on_documents_itemSelectionChanged()
{
    CWizDocumentDataArray arrayDocument;
    m_documents->getSelectedDocuments(arrayDocument);

    if (arrayDocument.size() == 1)
    {
        // 如果选中单个文档则浏览该文档
        if (!m_bUpdatingSelection)
        {
            // 增加一个判断，根据用户设置选择在当前标签还是新标签浏览笔记
            //viewDocument(arrayDocument[0], true);
            viewDocument(arrayDocument[0]);
            resortDocListAfterViewDocument(arrayDocument[0]);
        }
    }

    updateHistoryButtonStatus();

    m_documents->viewport()->update();
}

void WizMainWindow::on_documents_itemDoubleClicked(QListWidgetItem* item)
{
    WizDocumentListViewDocumentItem* pItem = dynamic_cast<WizDocumentListViewDocumentItem*>(item);
    if (pItem)
    {
        WIZDOCUMENTDATA doc = pItem->document();
        if (doc.strType == "collaboration") {
            viewDocument(doc);
        } else if (m_dbMgr.db(doc.strKbGUID).isDocumentDownloaded(doc.strGUID)) {
            viewNoteInSeparateWindow(doc);
            resortDocListAfterViewDocument(doc);
        }
    }
}

/**
 * @brief WizMainWindow::on_options_settingsChanged
 * @param type
 */
void WizMainWindow::on_options_settingsChanged(WizOptionsType type)
{
    switch (type) {
    case wizoptionsNoteView:
        processAllDocumentViews([=](WizDocumentView* docView){
            docView->settingsChanged();
        });
        break;
    case wizoptionsSync:
        m_syncFull->setFullSyncInterval(userSettings().syncInterval());
        break;
    case wizoptionsFont:
    {
        processAllDocumentViews([=](WizDocumentView* docView){
            docView->web()->editorResetFont();
        });
        //m_doc->web()->editorResetFont();
        QMap<QString, WizSingleDocumentViewer*>& viewerMap = m_singleViewMgr->getDocumentViewerMap();
        QList<WizSingleDocumentViewer*> singleViewrList = viewerMap.values();
        for (WizSingleDocumentViewer* viewer : singleViewrList)
        {
            viewer->docView()->web()->editorResetFont();
        }
    }
        break;
    case wizoptionsFolders:
        m_category->sortItems(0, Qt::AscendingOrder);
        break;
    case wizoptionsSpellCheck:
        processAllDocumentViews([=](WizDocumentView* docView){
            docView->web()->editorResetSpellCheck();
        });
        break;
    default:
        break;
    }
}

void WizMainWindow::on_options_restartForSettings()
{
    m_bRestart = true;
    on_actionExit_triggered();
}

void WizMainWindow::resetPermission(const QString& strKbGUID, const QString& strOwner)
{
    int nPerm = m_dbMgr.db(strKbGUID).permission();
    bool isGroup = m_dbMgr.db().kbGUID() != strKbGUID;

    // Admin, Super, do anything
    if (nPerm == WIZ_USERGROUP_ADMIN || nPerm == WIZ_USERGROUP_SUPER)
    {
        // enable editing
        //m_doc->setReadOnly(false, isGroup);

        // enable create tag

        // enable new document
        m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT)->setEnabled(true);

        // enable delete document
        //m_actions->actionFromName("actionDeleteCurrentNote")->setEnabled(true);

        // Editor, only disable create tag
    }
    else if (nPerm == WIZ_USERGROUP_EDITOR)
    {
        //m_doc->setReadOnly(false, isGroup);
        m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT)->setEnabled(true);
        //m_actions->actionFromName("actionDeleteCurrentNote")->setEnabled(true);

        // Author
    }
    else if (nPerm == WIZ_USERGROUP_AUTHOR)
    {
        m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT)->setEnabled(true);

        // author is owner
        //QString strUserId = m_dbMgr.db().getUserId();
        //if (strOwner == strUserId) {
        //    m_doc->setReadOnly(false, isGroup);
        //    //m_actions->actionFromName("actionDeleteCurrentNote")->setEnabled(true);

        //// not owner
        //} else {
        //    m_doc->setReadOnly(true, isGroup);
        //    //m_actions->actionFromName("actionDeleteCurrentNote")->setEnabled(false);
        //}

        // reader
    }
    else if (nPerm == WIZ_USERGROUP_READER)
    {
        //m_doc->setReadOnly(true, isGroup);
        m_actions->actionFromName(WIZACTION_GLOBAL_NEW_DOCUMENT)->setEnabled(false);
        //m_actions->actionFromName("actionDeleteCurrentNote")->setEnabled(false);
    }
    else
    {
       Q_ASSERT(0);
    }
}

void WizMainWindow::setCurrentDocumentView(WizDocumentView* docView)
{
    m_doc = docView;
}

void WizMainWindow::on_mainTabWidget_currentChanged(int pageIndex)
{
    WizDocumentView* docView = qobject_cast<WizDocumentView*>(m_mainTabBrowser->widget(pageIndex));
    if ( docView ) {
        setCurrentDocumentView(docView);
        // check if some actions should get enabled.
        m_actions->actionFromName(WIZACTION_GLOBAL_SAVE_AS_MARKDOWN)->setEnabled(WizIsMarkdownNote(docView->note()));
        m_actions->actionFromName(WIZACTION_GLOBAL_SAVE_AS_PDF)->setEnabled(true);
        m_actions->actionFromName(WIZACTION_GLOBAL_SAVE_AS_HTML)->setEnabled(true);
    } else {
        m_actions->actionFromName(WIZACTION_GLOBAL_SAVE_AS_MARKDOWN)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_GLOBAL_SAVE_AS_PDF)->setEnabled(false);
        m_actions->actionFromName(WIZACTION_GLOBAL_SAVE_AS_HTML)->setEnabled(false);
    }
}

WizDocumentView* WizMainWindow::currentDocumentView()
{
    return qobject_cast<WizDocumentView*>(m_mainTabBrowser->currentWidget());
}

WizMainTabBrowser* WizMainWindow::mainTabView()
{
    return m_mainTabBrowser;
}

void WizMainWindow::showHomePage()
{
    // Get welcome.html
    QString welcomeHtml = Utils::WizPathResolve::resourcesPath() + "files/welcomepage/index.html";
    // Setup WebEngine
    WizWebEngineView *webView = new WizWebEngineView(
        {{"WizExplorerApp", WizMainWindow::instance()->publicAPIsObject()}}, nullptr);
    QPointer<WizWebsiteView> websiteView = new WizWebsiteView(webView, *this);
    websiteView->viewHtml(QUrl::fromLocalFile(welcomeHtml));
    // Show in tab browser
    m_mainTabBrowser->createTab(websiteView);
}

/*!
    Create document view and setup signal-slot connections.
 */
WizDocumentView* WizMainWindow::createDocumentView()
{
    //FIXME: This function will take about 1200 milliseconds.
    WizDocumentView* newDocView = new WizDocumentView(*this);
    auto newWebView = newDocView->web();

    // Binding signals
    //-------------------------------------------------------------------

    connect(newWebView, &WizDocumentWebView::loadDocumentRequested, m_docLoader, &WizDocumentLoaderThread::load);
    connect(m_docLoader, &WizDocumentLoaderThread::loaded, newWebView, &WizDocumentWebView::onDocumentReady, Qt::QueuedConnection);
    connect(m_docLoader, &WizDocumentLoaderThread::loadFailed, [this] {
        QMessageBox::critical(this, tr("Error"), tr("Can't view note: (Can't unzip note data)"));
    });

    connect(newWebView, &WizDocumentWebView::saveDocumentRequested, m_docSaver, &WizDocumentSaverThread::save);
    connect(m_docSaver, &WizDocumentSaverThread::saved, newWebView, &WizDocumentWebView::onDocumentSaved, Qt::QueuedConnection);

    connect(newDocView, &WizDocumentView::shareDocumentByLinkRequest,
            this, &WizMainWindow::on_shareDocumentByLink_request);
    connect(newDocView, SIGNAL(documentSaved(QString,WizDocumentView*)),
            m_singleViewMgr, SIGNAL(documentChanged(QString,WizDocumentView*)));
    connect(m_singleViewMgr, SIGNAL(documentChanged(QString,WizDocumentView*)),
            newDocView, SLOT(on_document_data_changed(QString,WizDocumentView*)));
    connect(newDocView->titleBar(), SIGNAL(viewNoteInSeparateWindow_request()),
            SLOT(viewCurrentNoteInSeparateWindow()));
    connect(newDocView->web(), SIGNAL(statusChanged(const QString&)), SLOT(on_editor_statusChanged(const QString&)));

    connect(newDocView, &WizDocumentView::viewNoteInExternalEditorRequest,
            m_externalEditorLauncher, &ExternalEditorLauncher::handleViewNoteInExternalEditorRequest);

    // Setup document view UI
    //-------------------------------------------------------------------

    newDocView->web()->setInSeperateWindow(false);
    //newDocView->commentWidget()->setMinimumWidth(195);
    //newDocView->web()->setMinimumWidth(576);

    return newDocView;
}

/**
 * @brief 在当前文档视图中载入笔记数据
 * @param data 文档数据
 * @param addToHistory 是否添加到历史记录
 */
void WizMainWindow::viewDocument(const WIZDOCUMENTDATAEX& data, bool addToHistory)
{
    Q_ASSERT(!data.strGUID.isEmpty());

    // 如果目标文档GUID等于当前文档GUID
    if (data.strGUID == m_doc->note().strGUID)
    {
        //
        m_doc->reviewCurrentNote();
        return;
    }

    // 重置许可
    resetPermission(data.strKbGUID, data.strOwner);
    // 如果文档正好是要编辑的
    bool forceEditing = false;
    if (data.strGUID == m_documentForEditing.strGUID)
    {
        forceEditing = true;
        m_documentForEditing = WIZDOCUMENTDATA();
    }
    // 指定为当前笔记视图，并发送浏览笔记请求信号
    WizGlobal::emitViewNoteRequested(m_doc, data, forceEditing);

    if (addToHistory) {
        m_history->addHistory(data);
    }
    //
    m_actions->actionFromName(WIZACTION_GLOBAL_SAVE_AS_MARKDOWN)->setEnabled(WizIsMarkdownNote(data));
    //
}

/**
 * @brief 载入文档数据，在文档视图中浏览文档
 * @param data 文档数据
 * @param addToHistory 是否添加到历史记录
 */
void WizMainWindow::viewDocument(const WIZDOCUMENTDATAEX& data)
{
    Q_ASSERT(!data.strGUID.isEmpty());

    // 遍历tab，查找已经打开的标签中是否有该文档
    for (int i = 0; i < m_mainTabBrowser->count(); ++i) {
        AbstractDocumentView* docView = qobject_cast<AbstractDocumentView*>(m_mainTabBrowser->widget(i));
        if ( docView == nullptr ) {
            continue;
        } else {
            if ( data.strGUID == docView->note().strGUID ) {
                m_mainTabBrowser->setCurrentWidget(docView);
                return;
            }
        }

    }

    resetPermission(data.strKbGUID, data.strOwner);

    bool forceEditing = false;
    if (data.strGUID == m_documentForEditing.strGUID)
    {
        forceEditing = true;
        m_documentForEditing = WIZDOCUMENTDATA();
    }

    AbstractDocumentView *view = nullptr;
    if (data.strType == "collaboration") {
        CollaborationDocView *newView = new CollaborationDocView(data, *this, this);
        newView->setEditorMode(modeReader);
        newView->loadDocument();
        int index = m_mainTabBrowser->createTab(newView);
        m_mainTabBrowser->setTabText(index, data.strTitle);
        view = newView;
    } else  {
        WizDocumentView* newDocView = createDocumentView();
        int index = m_mainTabBrowser->createTab(newDocView);
        m_mainTabBrowser->setTabText(index, data.strTitle);
        //TODO: directly invoke newDocView->viewNote() instead of signaling.
        WizGlobal::emitViewNoteRequested(newDocView, data, forceEditing);
        setCurrentDocumentView(newDocView); //FIXME: do not keep m_doc
        view = newDocView;
    }

    connect(view, &AbstractDocumentView::locateDocumentRequest,
            this, QOverload<const WIZDOCUMENTDATA&>::of(&WizMainWindow::locateDocument));

    m_actions->actionFromName(WIZACTION_GLOBAL_SAVE_AS_MARKDOWN)->setEnabled(WizIsMarkdownNote(data));

    return;
}

void WizMainWindow::viewAttachment(const WIZDOCUMENTATTACHMENTDATA &attachment)
{
    WizDatabase &db = m_dbMgr.db(attachment.strKbGUID);
    bool bIsLocal = db.isObjectDataDownloaded(attachment.strGUID, "attachment");
    QString strFileName = db.getAttachmentFileName(attachment.strGUID);
    bool bExists = WizPathFileExists(strFileName);

    if (!bIsLocal || !bExists)
    {
        downloadAttachment(attachment);
        // try to set the attachement read-only.
        QFile file(strFileName);
        if (file.exists() && !db.canEditAttachment(attachment) && (file.permissions() & QFileDevice::WriteUser))
        {
            QFile::Permissions permissions = file.permissions();
            permissions = permissions & ~QFileDevice::WriteOwner & ~QFileDevice::WriteUser
                    & ~QFileDevice::WriteGroup & ~QFileDevice::WriteOther;
            file.setPermissions(permissions);
        }
    }

    openAttachment(attachment, strFileName);
}

void WizMainWindow::titleChanged()
{
    WizDocumentView* docView = currentDocumentView();
    m_actions->actionFromName(WIZACTION_GLOBAL_SAVE_AS_MARKDOWN)->setEnabled(WizIsMarkdownNote(docView->note()));
}

void WizMainWindow::locateDocument(const WIZDOCUMENTDATA& data)
{
    try
    {
        m_bUpdatingSelection = true;
        if (m_category->setCurrentIndex(data))
        {
            m_bUpdatingSelection = false;
            m_documents->blockSignals(true);
            m_documents->addAndSelectDocument(data);
            m_documents->blockSignals(false);
        }
    }
    catch (...)
    {

    }

    m_bUpdatingSelection = false;
    raise();
}

void WizMainWindow::locateDocument(const QString& strKbGuid, const QString& strGuid)
{
    WIZDOCUMENTDATA doc;
    if (m_dbMgr.db(strKbGuid).documentFromGuid(strGuid, doc))
    {
        locateDocument(doc);
    }
}

QWidget*WizMainWindow::mainWindow()
{
    return this;
}

QObject*WizMainWindow::object()
{
    return this;
}

WizCategoryBaseView&WizMainWindow::category()
{
    return *m_category;
}

WizUserSettings&WizMainWindow::userSettings()
{
    return *m_settings;
}

QObject*WizMainWindow::CategoryCtrl()
{
    return m_category;
}

WizCategoryView* WizMainWindow::CategoryView()
{
    return m_category;
}

void WizMainWindow::on_application_messageAvailable(const QString& strMsg)
{
    qDebug() << "application message received : " << strMsg;
    if (strMsg == WIZ_SINGLE_APPLICATION)
    {
        shiftVisableStatus();
    }
}

void WizMainWindow::checkWizUpdate()
{
#ifndef BUILD4APPSTORE
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=](){
        WizUpgradeChecker m_upgrade;
        QString currentTagName = QString("v%1-%2")
                                            .arg(WIZ_CLIENT_VERSION)
                                            .arg(WIZ_DEV_STAGE);
        m_upgrade.setTagName(currentTagName);
        connect(&m_upgrade, &WizUpgradeChecker::checkFinished, this, &WizMainWindow::on_checkUpgrade_finished);
        m_upgrade.checkUpgrade();
    });
#endif
}


void WizMainWindow::adjustToolBarLayout()
{
    if (!m_toolBar)
        return;
#ifdef Q_OS_MAC
    //
    #ifndef USECOCOATOOLBAR
    //
    QWidget* list = m_documents->isVisible() ? (QWidget*)m_documents : (QWidget*)m_msgList;
    //
    QPoint ptSearch = list->mapToGlobal(QPoint(0, 0));
    QPoint ptSpacerBeforeSearch = m_spacerForToolButtonAdjust->mapToGlobal(QPoint(0, 0));
    //
    int spacerWidth = ptSearch.x() - ptSpacerBeforeSearch.x();
    spacerWidth += list->size().width();
    if (spacerWidth < 0)
        return;
    //
    m_spacerForToolButtonAdjust->adjustWidth(spacerWidth);
    //
    #endif
#else
    QWidget* list = m_documents->isVisible() ? (QWidget*)m_documents : (QWidget*)m_msgList;
    //
    QPoint ptSearch = list->mapToGlobal(QPoint(0, 0));
    QPoint ptSpacerBeforeSearch = m_spacerForToolButtonAdjust->mapToGlobal(QPoint(0, 0));
    //
    int spacerWidth = ptSearch.x() - ptSpacerBeforeSearch.x();
    int searchWidth = list->size().width();
    if (spacerWidth > 0)
    {
        //m_spacerForToolButtonAdjust->adjustWidth(spacerWidth);
    }
    else
    {
        searchWidth += spacerWidth;
    }
    //
    if (searchWidth > 100)
    {
        // 禁用宽度调整
        //m_searchWidget->setFixedWidth(searchWidth);
    }
#endif
}



//================================================================================
// WizExplorerApp APIs
//================================================================================

QObject* WizMainWindow::DocumentsCtrl()
{
    return m_documents;
}

QObject* WizMainWindow::DatabaseManager()
{
    return &m_dbMgr;
}

QObject* WizMainWindow::CurrentDocumentBrowserObject()
{
    WizDocumentView* docView = currentDocumentView();
    if (docView) {
        return currentDocumentView()->web();
    } else {
        return nullptr;
    }
}

WizDatabaseManager* WizMainWindow::DatabaseManagerEx()
{
    return &m_dbMgr;
}

QObject* WizMainWindow::CreateWizObject(const QString& strObjectID)
{
    CString str(strObjectID);
    if (0 == str.compareNoCase("WizKMControls.WizCommonUI"))
    {
        static WizCommonUI* commonUI = new WizCommonUI(this);
        return commonUI;
    }

    return nullptr;
}

void WizMainWindow::SetSavingDocument(bool saving)
{
}

void WizMainWindow::ProcessClipboardBeforePaste(const QVariantMap& data)
{
    Q_UNUSED(data);
    // QVariantMap =  {html: text, textContent: text};
    //qDebug() << data.value("html").toString();
    //qDebug() << data.value("textContent").toString();

//    QClipboard* clipboard = QApplication::clipboard();
//
//#ifdef Q_OS_LINUX
//    // on X, copy action carry formats: ("TARGETS", "MULTIPLE", "text/html", "image/bmp", "SAVE_TARGETS", "TIMESTAMP", "application/x-qt-image")
//    // paste image copy from chromium or firefox will insert html to the end, we should remove all text if image exist
//    const QMimeData *mimeData = clipboard->mimeData();
//    if (mimeData->hasImage() && mimeData->hasHtml()) {
//        QImage image = clipboard->image();
//        clipboard->clear();
//        clipboard->setImage(image);
//    }
//#endif
//
//    if (!clipboard->image().isNull()) {
//        // save clipboard image to $TMPDIR
//        QString strTempPath = WizGlobal()->GetTempPath();
//        CString strFileName = strTempPath + WizIntToStr(WizGetTickCount()) + ".png";
//        if (!clipboard->image().save(strFileName)) {
//            TOLOG("ERROR: Can't save clipboard image to file");
//            return;
//        }
//
//        QString strHtml = QString("<img border=\"0\" src=\"%1\" />")
//            .arg(QUrl::fromLocalFile(strFileName).toString());
//        web()->editorCommandExecuteInsertHtml(strHtml, true);
    //    }
}

QString WizMainWindow::TranslateString(const QString& string)
{
    return ::WizTranlateString(string);
}

void WizMainWindow::syncAllData()
{
    m_syncFull->startSyncAll(false);
    m_animateSync->startPlay();
}

void WizMainWindow::reconnectServer()
{
    WizDatabase& db = m_dbMgr.db();
    WizToken::setUserId(db.getUserId());
    if (!m_settings->password().isEmpty())
    {
        WizToken::setPasswd(m_settings->password());
    }

    m_syncFull->clearCurrentToken();
    m_syncQuick->clearCurrentToken();
    connect(WizToken::instance(), SIGNAL(tokenAcquired(QString)),
            SLOT(on_TokenAcquired(QString)), Qt::QueuedConnection);
    WizToken::requestToken();
}

void WizMainWindow::setFocusForNewNote(WIZDOCUMENTDATA doc)
{
    //FIXME: 因为当前标签非文档视图，引起空指针错误
    if (doc.strType != "svgpainter") {
        m_documentForEditing = doc;
    }
    m_documents->addAndSelectDocument(doc);
    m_documents->clearFocus();
    WizDocumentView* docView = currentDocumentView();
    if (docView) {
        docView->web()->setFocus(Qt::MouseFocusReason);
        docView->web()->editorFocus();
    };
}

/**
 * @brief 通过Wiz地址协议打开为知笔记文档
 * @param strKMURL
 */
void WizMainWindow::viewDocumentByWizKMURL(const QString &strKMURL)
{
    if (GetWizUrlType(strKMURL) != WizUrl_Document)
        return;

    QString strKbGUID = GetParamFromWizKMURL(strKMURL, "kbguid");
    QString strGUID = GetParamFromWizKMURL(strKMURL, "guid");
    WizDatabase& db = m_dbMgr.db(strKbGUID);

    WIZDOCUMENTDATA document;
    if (db.documentFromGuid(strGUID, document))
    {
        //m_category->setCurrentItem();
        m_documents->blockSignals(true);
        m_documents->setCurrentItem(0);
        m_documents->blockSignals(false);
        viewDocument(document);
        locateDocument(document);
        activateWindow();
        raise();
    }
}

/**
 * @brief 通过Wiz地址协议打开为知附件
 * @param strKbGUID
 * @param strKMURL
 */
void WizMainWindow::viewAttachmentByWizKMURL(const QString& strKbGUID, const QString& strKMURL)
{

    if (GetWizUrlType(strKMURL) != WizUrl_Attachment)
        return;

    WizDatabase& db = m_dbMgr.db(strKbGUID);
    QString strGUID = GetParamFromWizKMURL(strKMURL, "guid");

    WIZDOCUMENTATTACHMENTDATA attachment;
    if (db.attachmentFromGuid(strGUID, attachment))
        viewAttachment(attachment);
    else
        WizMessageBox::information(this, tr("Info"), 
            tr("Can't find the specified attachment, may be it has been deleted."));
}

void WizMainWindow::createNoteWithAttachments(const QStringList& strAttachList)
{
    initVariableBeforCreateNote();
    WIZDOCUMENTDATA data;
    if (!m_category->createDocumentByAttachments(data, strAttachList))
        return;

    setFocusForNewNote(data);
}

void WizMainWindow::createNoteWithText(const QString& strText)
{
    initVariableBeforCreateNote();
    QString strHtml = strText.toHtmlEscaped();
    QString strTitle = strHtml.left(strHtml.indexOf("\n"));
    if (strTitle.isEmpty())
    {
        strTitle = "New note";
    }
    else if (strTitle.length() > 200)
    {
        strTitle = strTitle.left(200);
    }
    strHtml = "<div>" + strHtml + "</div>";
    strHtml.replace(" ", "&nbsp;");
    strHtml.replace("\n", "<br />");
    WIZDOCUMENTDATA data;
    if (!m_category->createDocument(data, strHtml, strTitle))
    {
        return;
    }
    setFocusForNewNote(data);
}

void WizMainWindow::showNewFeatureGuide()
{
#ifdef Q_OS_WIN
    return;
#else
    QString strUrl = WizOfficialApiEntry::standardCommandUrl("link");
    strUrl = strUrl + "&site=" + (m_settings->locale() == WizGetDefaultTranslatedLocal() ? "wiznote" : "blog" );
    strUrl += "&name=newfeature-mac.html";

    WizFramelessWebDialog *dlg = new WizFramelessWebDialog();
    dlg->loadAndShow(strUrl);
#endif
}

void WizMainWindow::showMobileFileReceiverUserGuide()
{
#ifdef Q_OS_WIN
    return;
#else
    QString strUrl = WizOfficialApiEntry::standardCommandUrl("link");
    strUrl = strUrl + "&site=" + (m_settings->locale() == WizGetDefaultTranslatedLocal() ? "wiznote" : "blog" );
    strUrl += "&name=guidemap_sendimage.html";
    qInfo() <<"open dialog with url : " << strUrl;

    WizFramelessWebDialog *dlg = new WizFramelessWebDialog();
    connect(dlg, SIGNAL(doNotShowThisAgain(bool)),
            SLOT(setDoNotShowMobileFileReceiverUserGuideAgain(bool)));
    dlg->loadAndShow(strUrl);
#endif
}

void WizMainWindow::setDoNotShowMobileFileReceiverUserGuideAgain(bool bNotAgain)
{
    m_settings->setNeedShowMobileFileReceiverUserGuide(!bNotAgain);
}

void WizMainWindow::initTrayIcon(QSystemTrayIcon* trayIcon)
{
    Q_ASSERT(trayIcon);

    // Show mainwindow when click tray
    connect(trayIcon, &QSystemTrayIcon::activated, this, &WizMainWindow::handleTrayIconActived);

    // Create context mennu
    m_trayMenu = new QMenu(this);
    QAction* actionShow = m_trayMenu->addAction(tr("Show/Hide MainWindow"));
    connect(actionShow, SIGNAL(triggered()), SLOT(shiftVisableStatus()));

    QAction* actionNewNote = m_trayMenu->addAction(tr("New Note"));
    connect(actionNewNote, SIGNAL(triggered()), SLOT(on_trayIcon_newDocument_clicked()));

    //
    m_trayMenu->addSeparator();
    QAction* actionHideTrayIcon = m_trayMenu->addAction(tr("Hide TrayIcon"));
    connect(actionHideTrayIcon, SIGNAL(triggered()), SLOT(on_hideTrayIcon_clicked()));
    //
    m_trayMenu->addSeparator();
    QAction* actionLogout = m_trayMenu->addAction(tr("Logout"));
    connect(actionLogout, SIGNAL(triggered()), SLOT(on_actionLogout_triggered()));
    QAction* actionExit = m_trayMenu->addAction(tr("Exit"));
    connect(actionExit, SIGNAL(triggered()), SLOT(on_actionExit_triggered()));

    connect(m_tray, SIGNAL(viewMessageRequest(qint64)),
            SLOT(on_viewMessage_request(qint64)));
    connect(m_tray, SIGNAL(viewMessageRequestNormal(QVariant)),
            SLOT(on_viewMessage_requestNormal(QVariant)));
    //
    //
    trayIcon->setContextMenu(m_trayMenu);
#ifdef Q_OS_MAC
    QString normal = WizGetSkinResourceFileName(userSettings().skin(), "trayIcon");
    QString selected = WizGetSkinResourceFileName(userSettings().skin(), "trayIcon_selected");
    QIcon icon;
    icon.setIsMask(true);
    icon.addFile(normal, QSize(), QIcon::Normal, QIcon::Off);
    icon.addFile(selected, QSize(), QIcon::Selected, QIcon::Off);
    if (!icon.isNull())
    {
        trayIcon->setIcon(icon);
    }
    //
    connect(m_trayMenu, &QMenu::aboutToHide, [=]{
        m_tray->setIcon(icon);
    });
    connect(m_trayMenu, &QMenu::aboutToShow, [=]{
        QIcon iconSelected;
        iconSelected.addFile(selected, QSize(), QIcon::Normal, QIcon::Off);
        m_tray->setIcon(iconSelected);
    });

#endif
}

void WizMainWindow::setMobileFileReceiverEnable(bool bEnable)
{
    if (bEnable)
    {
        if (!m_mobileFileReceiver)
        {
            m_mobileFileReceiver = new WizMobileFileReceiver(this);
            connect(m_mobileFileReceiver, SIGNAL(fileReceived(QString)),
                    SLOT(on_mobileFileRecived(QString)));
            m_mobileFileReceiver->start();
        }
    }
    else
    {
        if (m_mobileFileReceiver)
        {
            m_mobileFileReceiver->waitForDone();
            delete m_mobileFileReceiver;
            m_mobileFileReceiver = nullptr;
        }
    }
}

void WizMainWindow::startSearchStatus()
{
    m_documents->setAcceptAllSearchItems(true);
}

void WizMainWindow::quitSearchStatus()
{
    // 如果当前是搜索模式，在退出搜索模式时清除搜索框中的内容
    if (m_documents->acceptAllSearchItems())
    {
        m_searchWidget->clear();
        m_searchWidget->clearFocus();
        m_strSearchKeywords.clear();
    }

    m_documents->setAcceptAllSearchItems(false);
    if (m_category->selectedItems().count() > 0)
    {
        m_category->setFocus();
        m_category->setCurrentItem(m_category->selectedItems().first());
    }
}

void WizMainWindow::initVariableBeforCreateNote()
{
    quitSearchStatus();
}

bool WizMainWindow::needShowNewFeatureGuide()
{
    if (m_settings->serverType() == EnterpriseServer)
        return false;
    //
    QString strGuideVserion = m_settings->newFeatureGuideVersion();
    if (strGuideVserion.isEmpty())
        return true;

    return strGuideVserion.compare(WIZ_NEW_FEATURE_GUIDE_VERSION) < 0;
}

void WizMainWindow::resortDocListAfterViewDocument(const WIZDOCUMENTDATA& doc)
{
    if (m_documents->isSortedByAccessDate())
    {
        m_documents->reloadItem(doc.strKbGUID, doc.strGUID);
        m_documents->sortItems();
    }
}

void WizMainWindow::showCommentWidget()
{
    //FIXME: 不能直接在m_doc上显示
    WizDocumentView* docView = currentDocumentView();
    QWidget* commentWidget = docView->commentWidget();
    if (!commentWidget->isVisible())
    {
        QSplitter* splitter = qobject_cast<QSplitter*>(commentWidget->parentWidget());
        if (splitter)
        {
            QList<int> li = splitter->sizes();
            Q_ASSERT(li.size() == 2);
            if (li.size() == 2)
            {
                QList<int> lin;
                const int COMMENT_FRAME_WIDTH = 315;
                lin.push_back(splitter->width() - COMMENT_FRAME_WIDTH);
                lin.push_back(COMMENT_FRAME_WIDTH);
                splitter->setSizes(lin);
                commentWidget->show();
            }
        }
    }
}

WizDocumentWebView* WizMainWindow::getActiveEditor()
{
    WizDocumentView* docView = currentDocumentView();
    if (!docView)
        return nullptr;
    WizDocumentWebView* editor = docView->web();
    QWidget* activeWgt = qApp->activeWindow();
    if (activeWgt != this)
    {
        if (WizSingleDocumentViewer* singleViewer = dynamic_cast<WizSingleDocumentViewer*>(activeWgt))
        {
            editor = singleViewer->docView()->web();
        }
    }

    return editor;
}

void WizMainWindow::showDocumentList()
{
    if (!m_settings->showLayoutDocumentListView())
        return;

    if (!m_noteListWidget->isVisible())
    {
        m_docListContainer->show();
        m_noteListWidget->show();
        m_msgListWidget->hide();
    }
}

int getDocumentLeadInfoStateByCategoryItemType(int categoryItemType)
{
    switch (categoryItemType) {
    case Category_AllFoldersItem:
        return DocumentLeadInfo_PersonalRoot;
    case Category_GroupRootItem:
        return DocumentLeadInfo_GroupRoot;
    case Category_FolderItem:
        return DocumentLeadInfo_PersonalFolder;
    case Category_GroupItem:
        return DocumentLeadInfo_GroupFolder;
    case Category_TagItem:
        return DocumentLeadInfo_PersonalTag;
    default:
        break;
    }
    return DocumentLeadInfo_None;
}

void WizMainWindow::showDocumentList(WizCategoryBaseView* category)
{
    showDocumentList();
    QString kbGUID = category->selectedItemKbGUID();
    if (!kbGUID.isEmpty())
    {
        resetPermission(kbGUID, "");
    }

    CWizDocumentDataArray arrayDocument;
    category->getDocuments(arrayDocument);
    if (category->selectedItems().size() > 0)
    {
        int leadInfoState = getDocumentLeadInfoStateByCategoryItemType(category->selectedItems().first()->type());
        m_documents->setLeadInfoState(leadInfoState);
    }
    m_documents->setDocuments(arrayDocument);
    m_labelDocumentsHint->setVisible(false);
    m_btnMarkDocumentsReaded->setVisible(false);

    if (arrayDocument.empty())
    {
        on_documents_itemSelectionChanged();
    }
}

void WizMainWindow::showMessageList(WizCategoryViewMessageItem* pItem)
{
    if (!m_settings->showLayoutDocumentListView())
        return;

    if (!m_msgListWidget->isVisible())
    {
        m_docListContainer->show();
        m_msgListWidget->show();
        m_noteListWidget->hide();
    }


    CWizMessageDataArray arrayMsg;
    pItem->getMessages(m_dbMgr.db(), m_msgListTitleBar->currentSenderGUID(), arrayMsg);
    m_msgList->setMessages(arrayMsg);

    //
    int unreadCount = m_dbMgr.db().getUnreadMessageCount();
    // msg title bar
    bool showUnreadBar = pItem->hitTestUnread();
    m_msgListTitleBar->setUnreadMode(showUnreadBar, unreadCount);
}

void WizMainWindow::viewDocumentByShortcut(WizCategoryViewShortcutItem* pShortcut)
{
    showDocumentList();
    //
    WizDatabase &db = m_dbMgr.db(pShortcut->kbGUID());
    switch (pShortcut->shortcutType()) {
    case WizCategoryViewShortcutItem::Document:
    {
        WIZDOCUMENTDATA doc;
        if (db.documentFromGuid(pShortcut->guid(), doc))
        {
            WizCategoryViewItemBase* baseItem = m_category->findFolder(doc);
            if (baseItem)
            {
                CWizDocumentDataArray arrayDocument;
                baseItem->getDocuments(db, arrayDocument);
                int infoState = getDocumentLeadInfoStateByCategoryItemType(baseItem->type());
                m_documents->setLeadInfoState(infoState);
                m_documents->setDocuments(arrayDocument);
                m_documents->setAcceptAllSearchItems(true);
                m_documents->addAndSelectDocument(doc);
                m_documents->setAcceptAllSearchItems(false);
            }
            else
            {
                viewDocument(doc, true);
            }
        }
    }
        break;
    case WizCategoryViewShortcutItem::PersonalFolder:
    {
        CWizDocumentDataArray array;
        if (db.getDocumentsByLocation(pShortcut->location(), array))
        {
            m_documents->setLeadInfoState(DocumentLeadInfo_PersonalFolder);
            m_documents->setDocuments(array);
        }
    }
        break;
    case WizCategoryViewShortcutItem::PersonalTag:
    case WizCategoryViewShortcutItem::GroupTag:
    {
        CWizDocumentDataArray array;
        WIZTAGDATA tag;
        db.tagFromGuid(pShortcut->guid(), tag);
        if (db.getDocumentsByTag(tag, array))
        {
            int leadState = pShortcut->shortcutType() == WizCategoryViewShortcutItem::GroupTag?
                        DocumentLeadInfo_GroupFolder
                      : DocumentLeadInfo_PersonalTag;
            m_documents->setLeadInfoState(leadState);
            m_documents->setDocuments(array);
        }
    }
        break;
    }

}

void WizMainWindow::searchNotesBySQL(const QString& strSQLWhere)
{
    if (strSQLWhere.isEmpty())
        return;
    m_searcher->searchBySQLWhere(strSQLWhere, 500);
}

void WizMainWindow::searchNotesBySQLAndKeyword(const QString& strSQLWhere, const QString& strKeyword, int searchScope)
{
    qDebug() << "search by sql and keyword : " << strSQLWhere << strKeyword;
    if (strSQLWhere.isEmpty())
    {
        m_searcher->search(strKeyword, 500, (SearchScope)searchScope);
    }
    else if (strKeyword.isEmpty())
    {
        m_searcher->searchBySQLWhere(strSQLWhere, 500, (SearchScope)searchScope);
    }
    else
    {
        m_searcher->searchByKeywordAndWhere(strKeyword, strSQLWhere, 500, (SearchScope)searchScope);
    }
}

void WizMainWindow::updateHistoryButtonStatus()
{
    bool canGoBack = m_history->canBack();
    m_actions->actionFromName(WIZACTION_GLOBAL_GOBACK)->setEnabled(canGoBack);
    bool canGoForward = m_history->canForward();
    m_actions->actionFromName(WIZACTION_GLOBAL_GOFORWARD)->setEnabled(canGoForward);
}

/**
 * @brief 打开为知笔记附件
 * @param attachment
 * @param strFileName
 */
void WizMainWindow::openAttachment(const WIZDOCUMENTATTACHMENTDATA& attachment,
                                const QString& strFileName)
{
    if (!QDesktopServices::openUrl(QUrl::fromLocalFile(strFileName)))
    {
        qDebug() << "Can not open attachment file : " << strFileName;
    }
    //
    WizMainWindow* mainWindow = WizMainWindow::instance();
    //
    WizFileMonitor& monitor = WizFileMonitor::instance();
    connect(&monitor, SIGNAL(fileModified(QString,QString,QString,QString,QDateTime)),
            mainWindow, SLOT(onAttachmentModified(QString,QString,QString,QString,QDateTime)), Qt::UniqueConnection);

    monitor.addFile(attachment.strKbGUID, attachment.strGUID, strFileName, attachment.strDataMD5);
}

void WizMainWindow::onAttachmentModified(QString strKbGUID, QString strGUID, QString strFileName,
                          QString strMD5, QDateTime dtLastModified)
{
    WizDatabase& db = m_dbMgr.db(strKbGUID);
    //
    db.onAttachmentModified(strKbGUID, strGUID, strFileName, strMD5, dtLastModified);
    //
    quickSyncKb(strKbGUID);
}


void WizMainWindow::downloadAttachment(const WIZDOCUMENTATTACHMENTDATA& attachment)
{
    WizProgressDialog *dlg = progressDialog();
    dlg->setProgress(100,0);
    dlg->setActionString(tr("Downloading attachment file  %1 ...").arg(attachment.strName));
    dlg->setWindowTitle(tr("Downloading"));

    WizObjectDownloaderHost* downloader = WizObjectDownloaderHost::instance();
    connect(downloader, SIGNAL(downloadProgress(QString,int,int)),
            dlg, SLOT(setProgress(QString,int,int)));
    connect(downloader, SIGNAL(downloadDone(WIZOBJECTDATA,bool)),
            dlg, SLOT(accept()));
    downloader->downloadData(attachment);
    dlg->exec();
}

void WizMainWindow::viewNoteInSeparateWindow(const WIZDOCUMENTDATA& data)
{
    WizDocumentView* docView = currentDocumentView();
    if (docView && docView->note().strGUID == data.strGUID) {
        docView->web()->trySaveDocument(docView->note(), false, [=](const QVariant&){
            // Force current tab viewer to save
            docView->setEditorMode(modeReader);
            m_singleViewMgr->viewDocument(data);
            // update dock menu
            resetDockMenu();
        });
    } else {
        m_singleViewMgr->viewDocument(data);
        resetDockMenu();
    }
}

void WizMainWindow::viewCurrentNoteInSeparateWindow()
{
    WizDocumentView* docView = currentDocumentView();
    WIZDOCUMENTDATA doc = docView->note();
    viewNoteInSeparateWindow(doc);
}

void WizMainWindow::quickSyncKb(const QString& kbGuid)
{
    if (!m_syncQuick)
        return;
    //
    m_syncQuick->addQuickSyncKb(kbGuid);
}

void WizMainWindow::setNeedResetGroups()
{
    if (!m_syncQuick || !m_syncFull)
        return;

    m_syncQuick->setNeedResetGroups();
    m_syncFull->setNeedResetGroups();
}




