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

#ifdef Q_OS_MAC
#include "mac/WizMacHelper.h"
#include <QTimer>
#include <QMimeData>
#endif

#include "WizMisc.h"
#include "utils/WizPathResolve.h"
#include "WizMainWindow.h"
#include "gui/tabbrowser/WizMainTabBrowser.h"
#include "WizDevToolsDialog.h"
#include "gui/documentviewer/WizDocumentView.h"
#include "share/WizSettings.h"


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


QWebEngineProfile* createWebEngineProfile(const WizWebEngineInjectObjectCollection& objects, QObject* parent)
{
    if (objects.empty())
        return nullptr;
    // Create a new profile
    QWebEngineProfile *profile = new QWebEngineProfile("WizNoteWebEngineProfile", parent);
    // Read qtwebchannel library
    QString jsWebChannelFileName = Utils::WizPathResolve::resourcesPath() + "files/webengine/wizwebchannel.js";
    QString jsWebChannel;
    WizLoadUnicodeTextFromFile(jsWebChannelFileName, jsWebChannel);
    // Read javascript initialization script
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
    // 
    CString objectNames;
    WizStringArrayToText(names, objectNames, ", ");
    jsInit.replace("__objectNames__", objectNames);
    // Combine scripts
    QString jsAll = jsWebChannel + "\n" + jsInit;
    //
    {
        QWebEngineScript script;
        script.setSourceCode(jsAll);
        script.setName("webchannel.js");
        script.setWorldId(QWebEngineScript::MainWorld);
        script.setInjectionPoint(QWebEngineScript::DocumentCreation);
        script.setRunsOnSubFrames(false); // if set True, it will cause some error in javascript.
        profile->scripts()->insert(script);
    }
    //
    return profile;
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


WizWebEngineView::WizWebEngineView(const WizWebEngineInjectObjectCollection& objects, QWidget* parent)
    : QWebEngineView(parent)
{
    // Initialize m_viewActions to nullptr
    memset(m_viewActions, 0, sizeof(m_viewActions));

    setupPage(new WizWebEnginePage(objects, this));
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

    return zoomFactor();
}

double WizWebEngineView::scaleDown()
{
    // zoom out
    qreal factor = zoomFactor();
    factor -= 0.05;
    factor = (factor < 0.5) ? 0.5 : factor;
    setZoomFactor(factor);

    return zoomFactor();
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

    // save page action
    connect(pageAction(QWebEnginePage::SavePage), &QAction::triggered, 
            this, &WizWebEngineView::handleSavePageTriggered);
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
                qApp->desktop()->availableGeometry()
            )
        );
    }

    page()->triggerAction(QWebEnginePage::InspectElement);

    m_devToolsWindow->show();
    m_devToolsWindow->raise();
}
#endif

void WizWebEngineView::handleSavePageTriggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Page"),
        QDir::home().absoluteFilePath(documentTitle() + ".mhtml"),
        tr("MIME HTML (*.mht *.mhtml)"));
    if (!fileName.isEmpty())
        page()->save(fileName);
}

/**
 * @brief WizWebEngineView::getPage
 * @return
 */
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
    if (ev->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(ev);
        if (keyEvent->modifiers() && keyEvent->key()) {
            if (keyEvent->modifiers() & Qt::ControlModifier
                    && keyEvent->key() == Qt::Key_Up) {
                scaleUp();
            } else if (keyEvent->modifiers() & Qt::ControlModifier
                            && keyEvent->key() == Qt::Key_Down) {
                scaleDown();
            }
        }
    } else if (ev->type() == QEvent::Wheel) {
        QWheelEvent *whellEvent = static_cast<QWheelEvent *>(ev);
        if (whellEvent->modifiers() == Qt::ControlModifier) {
            if (whellEvent->delta() > 0) {
                scaleUp();
            } else {
                scaleDown();
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