#include "WizWebEngineView.h"

#include <QWebEngineView>
#include <QWebSocketServer>
#include <QWebChannel>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>
#include <QWebEngineProfile>
#include <QDesktopWidget>
#include <QStyle>
#include <QMenu>
#include <QKeyEvent>
#include <QApplication>
#include <QDesktopServices>
#include <QClipboard>
#include <QFileDialog>
#include <QTimer>
#include <QMessageBox>
#include <QMargins>
#include <QInputDialog>

#ifdef Q_OS_MAC
#include "mac/WizMacHelper.h"
#include <QTimer>
#include <QMimeData>
#endif

#include "WizMisc.h"
#include "WizMainWindow.h"
#include "WizDevToolsDialog.h"
#include "utils/WizPathResolve.h"
#include "utils/WizStyleHelper.h"
#include "gui/tabbrowser/WizMainTabBrowser.h"
#include "gui/documentviewer/WizDocumentView.h"
#include "share/WizSettings.h"
#include "share/WizMisc.h"
#include "widgets/DownloadManagerWidget.h"


WizWebEngineAsyncMethodResultObject::WizWebEngineAsyncMethodResultObject(QObject* parent)
    : QObject(parent)
    , m_acquired(false)
{
}

WizWebEngineAsyncMethodResultObject::~WizWebEngineAsyncMethodResultObject()
{
}

void WizWebEngineAsyncMethodResultObject::setResult(const QVariant& result)
{
    m_acquired = true;
    m_result = result;
    emit resultAcquired(m_result);
}

/*!
    \brief Create a Web Engine Profile object

    Exposing C++ APIs in this way makes it impossible for JavaScript code to
    predict when APIs \a objects will be created. Therefore we prefer to let
    the JavaScript code create the APIs objects itself. This function is only
    used to run some of the WizNote web services.
*/
QWebEngineProfile* createWebEngineProfile(const WizWebEngineInjectObjectCollection& objects, QObject* parent)
{
    if (objects.empty())
        return nullptr;

    // Create a new profile
    QWebEngineProfile *profile = new QWebEngineProfile("WizNoteWebEngineProfile", parent);

    // Load qtwebchannel library
    QString jsWebChannelFileName = Utils::WizPathResolve::resourcesPath() + "files/webengine/wizwebchannel.js";
    QString jsWebChannel;
    WizLoadUnicodeTextFromFile(jsWebChannelFileName, jsWebChannel);

    // Load javascript initialization script
    QString initFileName = Utils::WizPathResolve::resourcesPath() + "files/webengine/wizwebengineviewinit.js";
    QString jsInit;
    WizLoadUnicodeTextFromFile(initFileName, jsInit);

    // Get all names of published objects
    CWizStdStringArray names;
    WizWebEngineInjectObjectCollection::const_iterator inject = objects.constBegin();
    while (inject != objects.constEnd()) {
        names.push_back("\"" + inject.key() + "\"");
        ++inject;
    }

    // Generate js scripts for these objects
    CString objectNames;
    WizStringArrayToText(names, objectNames, ", ");
    jsInit.replace("__objectNames__", objectNames);

    // Combine scripts
    QString jsAll = jsWebChannel + "\n" + jsInit;

    {
        QWebEngineScript script;
        script.setSourceCode(jsAll);
        script.setName("webchannel.js");
        script.setWorldId(QWebEngineScript::MainWorld);
        script.setInjectionPoint(QWebEngineScript::DocumentCreation);
        script.setRunsOnSubFrames(false); // if set True, it will cause some error in javascript.
        profile->scripts()->insert(script);
    }

    insertScrollbarStyleSheet(profile);

    QObject::connect(profile, &QWebEngineProfile::downloadRequested,
                     &DownloadManagerWidget::instance(), &DownloadManagerWidget::downloadRequested);

    return profile;
}

void insertStyleSheet(QWebEngineProfile *profile, const QString &name, const QString &source)
{
      QWebEngineScript script;
      QString s = QString::fromLatin1("(function() {"\
                                      "    css = document.createElement('style');"\
                                      "    css.type = 'text/css';"\
                                      "    css.id = '%1';"\
                                      "    document.head.appendChild(css);"\
                                      "    css.innerText = '%2';"\
                                      "})()").arg(name).arg(source.simplified());

      script.setName(name);
      script.setSourceCode(s);
      script.setInjectionPoint(QWebEngineScript::DocumentReady);
      script.setRunsOnSubFrames(true);
      script.setWorldId(QWebEngineScript::ApplicationWorld);
      profile->scripts()->insert(script);
}

void insertScrollbarStyleSheet(QWebEngineProfile *profile)
{
    QString strTheme = Utils::WizStyleHelper::themeName();
    QDir skinFolder = WizGetSkinResourcePath(strTheme);
    if (skinFolder.exists("webkit_scrollbar.css")) {
        QString source;
        bool ok = WizLoadUnicodeTextFromFile(
            skinFolder.absoluteFilePath("webkit_scrollbar.css"),
            source, "UTF-8");
        if (ok)
            insertStyleSheet(profile, "webkit_scrollbar", source);
    }

}

WizWebEnginePage::WizWebEnginePage(const WizWebEngineInjectObjectCollection& objects, QWebEngineProfile *profile, QObject* parent)
    : QWebEnginePage(profile, parent)
    , m_continueNavigate(true)
{
    if (!objects.isEmpty()) {
        QWebChannel *channel = new QWebChannel(this);
        channel->registerObjects(objects);
        setWebChannel(channel);
    }
}

void WizWebEnginePage::javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID)
{
    qDebug() << message;
}

bool WizWebEnginePage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    if (NavigationTypeLinkClicked != type)
        return true;
    //
    m_continueNavigate = true;
    emit linkClicked(url, type, isMainFrame, this);

    return m_continueNavigate;
}

void WizWebEnginePage::triggerAction(WizWebEnginePage::WebAction action, bool checked /*= false*/)
{
    QWebEnginePage::triggerAction(action, checked);
    //
    if (action == Copy)
    {
#ifdef Q_OS_MAC
        //fix
        processCopiedData();
#endif
    }
}

void WizWebEnginePage::processCopiedData()
{
    //从webengine复制的文字，粘贴到mac的备忘录的时候，中文会乱码。
    //webengine复制到剪贴板的纯文字有bug，编码有问题。因此延迟等到webengine处理完成后再重新粘贴纯文本
    //避免这个错误
    //
    //
#ifdef Q_OS_MAC
    QTimer::singleShot(500, [=]{
        //
        QClipboard* clipboard = QApplication::clipboard();
        const QMimeData *mimeData = clipboard->mimeData();
        QMimeData* newData = new QMimeData();
        for (auto format : mimeData->formats()) {
            //
            if (format == "text/html") {
                //
                QByteArray htmlData = mimeData->data(format);
                QString html = QString::fromUtf8(htmlData);
                html = "<meta content=\"text/html; charset=utf-8\" http-equiv=\"Content-Type\">" + html;
                newData->setHtml(html);
                //
            } else {
                newData->setData(format, mimeData->data(format));
            }
        }
        //
        clipboard->setMimeData(newData);
    });
#endif
}

void WizWebEnginePage::javaScriptAlert(const QUrl &securityOrigin, const QString& msg)
{
    QMessageBox::information(view(), QStringLiteral("Javascript Alert"), msg);
}

bool WizWebEnginePage::javaScriptConfirm(const QUrl &securityOrigin, const QString& msg)
{
    auto ret = QMessageBox::information(view(), QStringLiteral("Javascript Confirm"), msg,
                                        QMessageBox::Ok, QMessageBox::Cancel);
    return ret == QMessageBox::Ok;
}

bool WizWebEnginePage::javaScriptPrompt(const QUrl &securityOrigin, const QString& msg,
                                        const QString& defaultValue, QString* result)
{
    bool ret = false;
    if (result)
        *result = QInputDialog::getText(view(), QStringLiteral("Javascript Prompt"), msg,
                                        QLineEdit::Normal, defaultValue, &ret);
    return ret;
}


WizWebEngineView::WizWebEngineView(const WizWebEngineInjectObjectCollection& objects, QWidget* parent)
    : QWebEngineView(parent)
{
    // Initialize m_viewActions to nullptr
    memset(m_viewActions, 0, sizeof(m_viewActions));

    setupPage(new WizWebEnginePage(objects, this));

    connect(this, &QWebEngineView::renderProcessTerminated,
            [this](QWebEnginePage::RenderProcessTerminationStatus termStatus, int statusCode) {
        QString status;
        switch (termStatus) {
        case QWebEnginePage::NormalTerminationStatus:
            status = tr("Render process normal exit");
            break;
        case QWebEnginePage::AbnormalTerminationStatus:
            status = tr("Render process abnormal exit");
            break;
        case QWebEnginePage::CrashedTerminationStatus:
            status = tr("Render process crashed");
            break;
        case QWebEnginePage::KilledTerminationStatus:
            status = tr("Render process killed");
            break;
        }
        QMessageBox::StandardButton btn = QMessageBox::question(window(), status,
                                                   tr("Render process exited with code: %1\n"
                                                      "Do you want to reload the page ?").arg(statusCode));
        if (btn == QMessageBox::Yes)
            QTimer::singleShot(0, [this] { reload(); });
    });
}

WizWebEngineView::~WizWebEngineView()
{
    
}

QVariant WizWebEngineView::ExecuteScript(QString script)
{
    auto result = QSharedPointer<WizWebEngineAsyncMethodResultObject>(new WizWebEngineAsyncMethodResultObject(nullptr), &QObject::deleteLater);
    //
    page()->runJavaScript(script, [=](const QVariant &v) {
        result->setResult(v);
        QTimer::singleShot(1000, [=]{
            auto r = result;
            r = nullptr;
        });
    });

    QVariant v;
    v.setValue<QObject*>(result.data());
    return v;
}

QVariant WizWebEngineView::ExecuteScriptFile(QString fileName)
{
    QString script;
    if (!WizLoadUnicodeTextFromFile(fileName, script)) {
        return QVariant();
    }
    return ExecuteScript(script);
}

QVariant WizWebEngineView::ExecuteFunction0(QString function)
{
    QString script = QString("%1();").arg(function);
    return ExecuteScript(script);
}

QString toArgument(const QVariant& v)
{
    switch (v.type()) {
    case QVariant::Bool:
        return v.toBool() ? "true" : "false";
    case QVariant::Int:
    case QVariant::UInt:
    case QVariant::LongLong:
    case QVariant::ULongLong:
        return QString("%1").arg(v.toLongLong());
    case QVariant::Double: {
        double f = v.toDouble();
        QString str;
        str.sprintf("%f", f);
        return str;
    }
    case QVariant::Date:
    case QVariant::Time:
    case QVariant::DateTime:
        return QString("new Date(%1)").arg(v.toDateTime().toTime_t() * 1000);
    case QVariant::String: {
            QString s = v.toString();
            s.replace("\\", "\\\\");
            s.replace("\r", "\\r");
            s.replace("\n", "\\n");
            s.replace("\t", "\\t");
            s.replace("\"", "\\\"");
            return "\"" + s + "\"";
        }
    default:
        qDebug() << "Unsupport type: " << v.type();
        return "undefined";
    }
}

QVariant WizWebEngineView::ExecuteFunction1(QString function, const QVariant& arg1)
{
    QString script = QString("%1(%2);")
            .arg(function)
            .arg(toArgument(arg1))
            ;
    return ExecuteScript(script);
}

QVariant WizWebEngineView::ExecuteFunction2(QString function, const QVariant& arg1, const QVariant& arg2)
{
    QString script = QString("%1(%2, %3);")
            .arg(function)
            .arg(toArgument(arg1))
            .arg(toArgument(arg2))
            ;
    return ExecuteScript(script);
}

QVariant WizWebEngineView::ExecuteFunction3(QString function, const QVariant& arg1, const QVariant& arg2, const QVariant& arg3)
{
    QString script = QString("%1(%2, %3, %4);")
            .arg(function)
            .arg(toArgument(arg1))
            .arg(toArgument(arg2))
            .arg(toArgument(arg3))
            ;
    return ExecuteScript(script);
}

QVariant WizWebEngineView::ExecuteFunction4(QString function, const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, const QVariant& arg4)
{
    QString script = QString("%1(%2, %3, %4, %5);")
            .arg(function)
            .arg(toArgument(arg1))
            .arg(toArgument(arg2))
            .arg(toArgument(arg3))
            .arg(toArgument(arg4))
            ;
    return ExecuteScript(script);
}

/*!
    Set the zoom \a percent of this page. The range from 25 to 500. The default
    factor is 100.
 */
void WizWebEngineView::SetZoom(int percent)
{
    if ( percent < 25 && percent > 500)
        return;
    qreal factor = static_cast<qreal>(percent) / 100;
    setZoomFactor(factor);
    Q_EMIT zoomFactorChanged(factor);
}

/*!
    Get the zoom percentage of this page.
 */
int WizWebEngineView::GetZoom()
{
    qreal factor = zoomFactor();
    int percent = static_cast<int>( qRound(factor * 100) );
    return percent;
}

double WizWebEngineView::scaleUp()
{
    // zoom in
    qreal factor = zoomFactor();
    factor += 0.05;
    factor = (factor > 5.0) ? 5.0 : factor;
    setZoomFactor(factor);

    displayZoomWidget();
    Q_EMIT zoomFactorChanged(factor);

    return zoomFactor();
}

double WizWebEngineView::scaleDown()
{
    // zoom out
    qreal factor = zoomFactor();
    factor -= 0.05;
    factor = (factor < 0.5) ? 0.5 : factor;
    setZoomFactor(factor);

    displayZoomWidget();
    Q_EMIT zoomFactorChanged(factor);

    return zoomFactor();
}

void WizWebEngineView::hideEvent(QHideEvent *event)
{
    if (m_zoomWgt)
        m_zoomWgt->close();
    QWebEngineView::hideEvent(event);
}

void WizWebEngineView::createZoomWidget()
{
    m_zoomWgt = new WebPageZoomWidget(zoomFactor(), this);
    connect(this, &WizWebEngineView::zoomFactorChanged,
            m_zoomWgt, &WebPageZoomWidget::onZoomFactorChanged);
    connect(m_zoomWgt, &WebPageZoomWidget::scaleUpRequested,
            this, &WizWebEngineView::scaleUp);
    connect(m_zoomWgt, &WebPageZoomWidget::scaleDownRequested,
            this, &WizWebEngineView::scaleDown);
    connect(m_zoomWgt, &WebPageZoomWidget::resetZoomFactorRequested,
            this, [&] { SetZoom(100); });
    connect(m_zoomWgt, &WebPageZoomWidget::zoomFinished,
            this, &WizWebEngineView::zoomWidgetFinished);
}

void WizWebEngineView::displayZoomWidget()
{
    if (!m_zoomWgt) {
        createZoomWidget();
        QPoint leftTop = this->pos();
        if (parentWidget())
            leftTop = parentWidget()->mapToGlobal(leftTop);
        QRect loc(leftTop, size());
        QMargins margin = m_zoomWgt->layout()->contentsMargins();
        loc.moveTop(loc.top() - margin.top() + 4);
        m_zoomWgt->setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignHCenter | Qt::AlignTop,
                m_zoomWgt->sizeHint(),
                loc
            )
        );
        m_zoomWgt->show();
    }
}

void WizWebEngineView::handleShowZoomWidgetRequest(bool show, const QRect &btnLocation)
{
    if (show) {
        if (!m_zoomWgt)
            createZoomWidget();

        m_zoomWgt->clearTimer();
        m_zoomWgt->setTimeOut(0);
        m_zoomWgt->setPopup(true);

        QRect loc = btnLocation;
        loc.moveTop(loc.top() + loc.height());
        QMargins margin = m_zoomWgt->layout()->contentsMargins();
        loc.moveRight(loc.right() + margin.right());
        loc.moveTop(loc.top() - margin.top());
        m_zoomWgt->setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignRight | Qt::AlignTop,
                m_zoomWgt->sizeHint(),
                loc
            )
        );
        m_zoomWgt->show();

    } else {
        if (m_zoomWgt)
            m_zoomWgt->close();
    }
}

QAction *WizWebEngineView::viewAction(ViewAction action) const
{
    if (m_viewActions[action])
        return m_viewActions[action];

    QString text;

    switch (action) {
    case OpenDevTools:
        text = tr("Open DevTools");
        break;
    case OpenTempFileLocation:
        text = tr("Open temporary file's location");
        break;
    case ViewActionCount:
        Q_UNREACHABLE();
        break;
    }

    QAction *a = new QAction(const_cast<WizWebEngineView*>(this));
    a->setText(text);

    m_viewActions[action] = a;

    return a;
}

/*!
    We should avoid calling setPage() directly, because WizWebEngineView will
    setup something after subclass of WizWebEnginePage is created.
 */
void WizWebEngineView::setupPage(WizWebEnginePage *page)
{
    setPage(page);
    connect(page, &WizWebEnginePage::openLinkInNewWindow,
            this, &WizWebEngineView::openLinkInDefaultBrowser);
    connect(page, &WizWebEnginePage::loadFinished,
            this, &WizWebEngineView::innerLoadFinished);

    setupWebActions();
}

void WizWebEngineView::setupWebActions()
{

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    // Create a new action to open DevTools dialog.
    viewAction(OpenDevTools)->setShortcut(QKeySequence("F12"));
    addAction(viewAction(OpenDevTools));
    connect(viewAction(OpenDevTools), &QAction::triggered,
            this, &WizWebEngineView::openDevTools);
    connect(pageAction(QWebEnginePage::InspectElement), &QAction::triggered,
            this, &WizWebEngineView::openDevTools);
#endif

}

/*!
    Create basic context menu for web view.

    The parent of returned QMenu is corresponding QWebEngineView,
    here means WizWebEngineView. This QMenu has Qt::WA_DeleteOnClose
    attrbute, so we don't need to delete it manually.
 */
QMenu* WizWebEngineView::createStandardContextMenu()
{
    QMenu *menu = page()->createStandardContextMenu();
    const QList<QAction *> actions = menu->actions();

    // remove ViewSource
    auto viewSource = std::find(actions.cbegin(), actions.cend(), page()->action(QWebEnginePage::ViewSource));
    if (viewSource != actions.cend())
        menu->removeAction(*viewSource);

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    // add Open DevTools action
    auto inspectElement = std::find(actions.cbegin(), actions.cend(), page()->action(QWebEnginePage::InspectElement));
    if (inspectElement == actions.cend()) {
        menu->addAction(viewAction(OpenDevTools));
    } else {
        // If DevTools have been opened before, InspectElement will beadd to
        // ContextMenu automatically.
        (*inspectElement)->setText(tr("Inspect element"));
    }
#endif

    return menu;
}

void WizWebEngineView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    menu->popup(event->globalPos());
}


void WizWebEngineView::innerLoadFinished(bool ret)
{
    emit loadFinishedEx(ret);
}

void WizWebEngineView::openLinkInDefaultBrowser(QUrl url)
{
    QDesktopServices::openUrl(url);
}

QString WizWebEngineView::documentTitle()
{
    return title();
}

/*!
    \brief  Publish C++ object to javascript clients.

    This \a obj can be accessed by \a name within javascript context.
 */
void WizWebEngineView::addObjectToJavaScriptClient(QString name, QObject* obj)
{
    QWebEnginePage *webPage = page();
    QWebChannel *channel = webPage->webChannel();
    if (!channel) {
        channel = new QWebChannel(webPage);
        webPage->setWebChannel(channel);
    }

    WizWebEngineInjectObjectCollection r_objs = channel->registeredObjects();
    if (!obj)
        return;
    if (!r_objs.contains(name) && obj != nullptr)
        channel->registerObject(name, obj);
}

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
void WizWebEngineView::openDevTools()
{
    if (!m_devToolsWindow)
    {
        m_devToolsWindow = new WizDevToolsDialog(this);

        QString title = documentTitle();
        m_devToolsWindow->setWindowTitle("DevTools - " + title);

        m_devToolsWindow->web()->page()->setInspectedPage(this->page());
        // align on center of the screen
        m_devToolsWindow->setGeometry(
            QStyle::alignedRect(
                Qt::LeftToRight,
                Qt::AlignCenter,
                QSize(800, 500),
                this->screen()->availableGeometry()
            )
        );
    }

    page()->triggerAction(QWebEnginePage::InspectElement);

    m_devToolsWindow->show();
    m_devToolsWindow->raise();
}
#endif

WizWebEnginePage* WizWebEngineView::getPage() {
    return qobject_cast<WizWebEnginePage*>(page());
}

/**
 * @brief Create new window according to the request triggered within web page.
 * 
 * @param type 
 * @return QWebEngineView* 
 */
QWebEngineView *WizWebEngineView::createWindow(QWebEnginePage::WebWindowType type)
{
    WizNavigationForwarderView *forwarder = new WizNavigationForwarderView(this, this);
    forwarder->setVisible(false);
    return forwarder->forward(type);
}

static QWebEngineView* getActiveWeb()
{
    QWidget* focusWidget = qApp->focusWidget();
    if (!focusWidget)
        return nullptr;
    //
    while (focusWidget) {
        QWebEngineView* web =  dynamic_cast<QWebEngineView *>(focusWidget);
        if (web)
            return web;
        //
        focusWidget = focusWidget->parentWidget();
    }
    return nullptr;
}

WizWebEngineViewContainerDialog::WizWebEngineViewContainerDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{

}

void WizWebEngineView::childEvent(QChildEvent *ev)
{
    if (ev->added()) {
        ev->child()->installEventFilter(this);
    } else if (ev->removed()) {
        ev->child()->removeEventFilter(this);
    }

    QWebEngineView::childEvent(ev);
}

bool WizWebEngineView::eventFilter(QObject *obj, QEvent *ev)
{
    // work around QTBUG-43602

#ifndef Q_OS_MAC
    if (ev->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(ev);
        if (keyEvent->modifiers() && keyEvent->key()) {
            if (keyEvent->modifiers() & Qt::ControlModifier
                    && keyEvent->key() == Qt::Key_Up) {
                scaleUp();
                return true;
            } else if (keyEvent->modifiers() & Qt::ControlModifier
                            && keyEvent->key() == Qt::Key_Down) {
                scaleDown();
                return true;
            }
        }
    }
#endif // Q_OS_MAC

    if (ev->type() == QEvent::Wheel) {
        QWheelEvent *whellEvent = static_cast<QWheelEvent *>(ev);
        if (whellEvent->modifiers() == Qt::ControlModifier) {
            if (whellEvent->delta() > 0) {
                scaleUp();
                return true;
            } else {
                scaleDown();
                return true;
            }
        }
    }

    return QWebEngineView::eventFilter(obj, ev);
}

/**
 * @brief Construct a new WizNavigationForwarder.
 * 
 *      This is used to forward Url navigation request. If the url is WizNote internal one,
 *      it will be passed to WizNote handler.
 * 
 * @param ownerView 
 * @param parent 
 */
WizNavigationForwarderView::WizNavigationForwarderView(QWebEngineView *ownerView, QWidget* parent)
    : QWebEngineView(parent)
{
    WizNavigationForwarderPage *page = new WizNavigationForwarderPage(ownerView, this);
    m_page = page;
    setPage(page);
}


QWebEngineView *WizNavigationForwarderView::forward(QWebEnginePage::WebWindowType type)
{
    m_page->setWebWindowType(type);
    return this;
}

WizNavigationForwarderPage::WizNavigationForwarderPage(QWebEngineView *ownerView, QObject *parent)
    : QWebEnginePage(parent)
    , m_ownerView(ownerView)
{

}

bool WizNavigationForwarderPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    if (!isMainFrame)
        return false;

    QString protocol = url.scheme().toLower();
    QString strUrl = url.toString();
    if (strUrl.isEmpty())
        return false;

    WizMainWindow *mainWindow = WizMainWindow::instance();
    if (!mainWindow)
        return false;

    if (protocol == "wiz") {
        // WizNote internal url
        switch (GetWizUrlType(strUrl)) {
        case WizUrl_Document:
            mainWindow->viewDocumentByWizKMURL(strUrl);
            break;
        case WizUrl_Attachment:
        {
            WizDocumentWebView *docWebView = qobject_cast<WizDocumentWebView *>(m_ownerView);
            if (docWebView) {
                if (!docWebView->isEditing()) {
                    QString strGUID = docWebView->view()->note().strKbGUID;
                    mainWindow->viewAttachmentByWizKMURL(strGUID, strUrl);
                }
            }
            break;
        }
        default:
            qDebug() << QString("%1 is a wiz internal url , but we can not identify it");
        }
    } else {
        // http or file url
        bool openInDesktop = mainWindow->userSettings().isEnableOpenLinkWithDesktopBrowser();
        switch (m_windowType) {
            // A web browser tab.
            case QWebEnginePage::WebBrowserTab:
            {
                if (openInDesktop) {
                    QDesktopServices::openUrl(url);
                } else {
                    mainWindow->mainTabView()->createTab(url);
                }
                break;
            }
            // A web browser tab without hiding the current visible WebEngineView. (Ctrl+ mouse left click)
            case QWebEnginePage::WebBrowserBackgroundTab: 
            {
                //WizWebEngineView * view = mainWindow->mainTabView()->createBackgroundTab();
                //view->load(url);
                QDesktopServices::openUrl(url);
 
                break;
            }
            // A window without decoration.
            case QWebEnginePage::WebDialog:
            // A complete web browser window.
            case QWebEnginePage::WebBrowserWindow: 
            {
                if (openInDesktop) {
                    QDesktopServices::openUrl(url);
                } else {
                    WizWebEngineView * view = mainWindow->mainTabView()->createWindow();
                    view->load(url);
                }
                break;
            }
        }
    }

    parent()->deleteLater();

    return false;
}

void WizNavigationForwarderPage::setWebWindowType(QWebEnginePage::WebWindowType type)
{
    m_windowType = type;
}


WebPageZoomWidget::WebPageZoomWidget(qreal factor, QWidget *parent)
    : ShadowWidget(parent)
    , m_resetBtn(new QPushButton)
    , m_scaleUpBtn(new QPushButton)
    , m_scaleDownBtn(new QPushButton)
    , m_factorLabel(new QLabel)
{
    m_scaleUpBtn->setObjectName("scaleUpBtn");
    m_scaleDownBtn->setObjectName("scaleDownBtn");
    m_resetBtn->setObjectName("resetBtn");
    m_factorLabel->setObjectName("factorLabel");

    auto layout = new QHBoxLayout;
    layout->addWidget(m_factorLabel);
    layout->addStretch();
    layout->addWidget(m_scaleDownBtn);
    layout->addWidget(m_scaleUpBtn);
    layout->addWidget(m_resetBtn);
    widget()->setLayout(layout);

    QString strTheme = Utils::WizStyleHelper::themeName();
    m_resetBtn->setText(tr("Reset"));
    m_scaleDownBtn->setIcon(WizLoadSkinIcon(strTheme, "scaleDown"));
    m_scaleUpBtn->setIcon(WizLoadSkinIcon(strTheme, "scaleUp"));

    setZoomFactor(factor);
    setTimeOut(2000);

    connect(m_scaleUpBtn, &QPushButton::clicked,
            this, &WebPageZoomWidget::scaleUpRequested);
    connect(m_scaleDownBtn, &QPushButton::clicked,
            this, &WebPageZoomWidget::scaleDownRequested);
    connect(m_resetBtn, &QPushButton::clicked,
            this, &WebPageZoomWidget::resetZoomFactorRequested);
}

void WebPageZoomWidget::closeEvent(QCloseEvent *event)
{
    Q_EMIT zoomFinished();
    ShadowWidget::closeEvent(event);
}

void WebPageZoomWidget::setZoomFactor(qreal factor)
{
    int fa = static_cast<int>( qRound(factor * 100) );
    m_factorLabel->setText(QString("%1%").arg(fa, 3, 10));
}

void WebPageZoomWidget::onZoomFactorChanged(qreal factor)
{
    setZoomFactor(factor);
    resetTimer();
}


