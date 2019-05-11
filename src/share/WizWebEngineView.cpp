#include <QWebEngineView>
#include <QWebSocketServer>
#include <QWebChannel>
#include <QDesktopWidget>
#include <QStyle>
#include <QMenu>
#include "WizWebEngineView.h"
#include "WizMisc.h"
#include "utils/WizPathResolve.h"

#include <QKeyEvent>
#include <QApplication>
#include <QDesktopServices>
#include <QClipboard>
#include <QFileDialog>

#ifdef Q_OS_MAC
#include "mac/WizMacHelper.h"
#include <QTimer>
#include <QMimeData>
#endif

#include "WizDevToolsDialog.h"
#include "WizDocumentView.h"

class WizInvisibleWebEngineView : public QWebEngineView
{
    class WizInvisibleWebEnginePage : public QWebEnginePage
    {
        WizWebEnginePage* m_ownerPage;
    public:
        explicit WizInvisibleWebEnginePage(WizWebEnginePage* ownerPage, QObject *parent = Q_NULLPTR)
            : QWebEnginePage(parent)
            , m_ownerPage(ownerPage)
        {

        }

        bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
        {
            emit m_ownerPage->openLinkInNewWindow(url);
            //
            parent()->deleteLater();
            //
            return false;
        }

    };

public:
    explicit WizInvisibleWebEngineView(WizWebEnginePage* ownerPage, QWidget* parent = Q_NULLPTR)
        : QWebEngineView(parent)
    {
        WizInvisibleWebEnginePage* page = new WizInvisibleWebEnginePage(ownerPage, this);
        setPage(page);
    }
    virtual ~WizInvisibleWebEngineView()
    {

    }

public:
    static QWebEnginePage* create(WizWebEnginePage* ownerPage)
    {
        WizInvisibleWebEngineView* web = new WizInvisibleWebEngineView(ownerPage, nullptr);
        //
        web->setVisible(false);
        //
        return web->page();
    }
};

WizWebEnginePage::WizWebEnginePage(const WizWebEngineInjectObjectCollection& objects, QObject* parent)
    : QWebEnginePage(parent)
    , m_continueNavigate(true)
{
    if (!objects.empty()) {
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
QWebEnginePage *WizWebEnginePage::createWindow(WebWindowType type)
{
    return WizInvisibleWebEngineView::create(this);
}

void WizWebEnginePage::triggerAction(WizWebEnginePage::WebAction action, bool checked /*= false*/)
{
    QWebEnginePage::triggerAction(action, checked);
    //
    if (action == Copy)
    {
#ifdef Q_OS_MAC
        //fix
        //从webengine复制的文字，粘贴到mac的备忘录的时候，中文会乱码。
        //webengine复制到剪贴板的纯文字有bug，编码有问题。因此延迟等到webengine处理完成后再重新粘贴纯文本
        //避免这个错误
        //
        //
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
}

/** WizWebEngineView 构造函数
 *
 *  创建WizWebEnginePage
 */
WizWebEngineView::WizWebEngineView(const WizWebEngineInjectObjectCollection& objects, QWidget* parent)
    : QWebEngineView(parent)
{
    WizWebEnginePage* p = new WizWebEnginePage(objects, this);
    setPage(p);
    //
    connect(p, SIGNAL(openLinkInNewWindow(QUrl)), this, SLOT(openLinkInDefaultBrowser(QUrl)));
    //
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(innerLoadFinished(bool)));
}

WizWebEngineView::~WizWebEngineView()
{
    
}

/**
 * @brief Create basic context menu for web view.
 * @return
 */
QMenu* WizWebEngineView::createStandardContextMenu()
{
    QMenu *menu = page()->createStandardContextMenu();
    const QList<QAction *> actions = menu->actions();
    // add Open DevTools action
    auto inspectElement = std::find(actions.cbegin(), actions.cend(), page()->action(QWebEnginePage::InspectElement));
    if (inspectElement == actions.cend()) {
        auto viewSource = std::find(actions.cbegin(), actions.cend(), page()->action(QWebEnginePage::ViewSource));
        if (viewSource == actions.cend())
            menu->addSeparator();

        QAction *action = new QAction(menu);
        action->setText(tr("Open DevTools"));
        connect(action, &QAction::triggered, this, &WizWebEngineView::openDevTools, Qt::UniqueConnection);

        QAction *before(inspectElement == actions.cend() ? nullptr : *inspectElement);
        menu->insertAction(before, action);
    } else {
        (*inspectElement)->setText(tr("Inspect element"));
        // refresh new page's InspectElement action
        //disconnect(*inspectElement, &QAction::triggered, this, &WizWebEngineView::openDevTools);
        connect(*inspectElement, &QAction::triggered, this, &WizWebEngineView::openDevTools, Qt::UniqueConnection);
    }
    return menu;
}

void WizWebEngineView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    // refresh new page's ViewSource action
    connect(pageAction(QWebEnginePage::ViewSource), &QAction::triggered, 
                    this, &WizWebEngineView::onViewSourceTriggered, Qt::UniqueConnection);
    // save page action
    connect(pageAction(QWebEnginePage::SavePage), &QAction::triggered, 
                    this, &WizWebEngineView::handleSavePageTriggered, Qt::UniqueConnection);
    //
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

/**
 * @brief Publish C++ objects to javascript clients.
 * 
 * @param name 
 * @param obj 
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
    if (!r_objs.contains(name))
        channel->registerObject(name, obj);
}

void WizWebEngineView::openDevTools()
{
    if (!m_devToolsWindow)
    {
        m_devToolsWindow = new WizDevToolsDialog(this);
        // 设置外观
        QString title = documentTitle();
        m_devToolsWindow->setWindowTitle("DevTools - " + title);
        //
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
    //
    m_devToolsWindow->show();
    m_devToolsWindow->raise();
}

void WizWebEngineView::onViewSourceTriggered()
{
    emit viewSourceRequested(page()->url(), page()->url().url());
}

void WizWebEngineView::handleSavePageTriggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Page"),
        QDir::home().absoluteFilePath(page()->title() + ".mhtml"),
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
 * @brief 重写基类setPage函数，重设m_page
 * @param page

void WizWebEngineView::setPage(WizWebEnginePage* page)
{
    m_page = page;
    QWebEngineView::setPage(page);
}
 */

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

bool WizWebEngineViewProgressKeyEvents(QKeyEvent* ev)
{
    if (ev->modifiers() && ev->key()) {
        if (QWebEngineView* web = getActiveWeb()) {
            if (ev->matches(QKeySequence::Copy))
            {
                web->page()->triggerAction(QWebEnginePage::Copy);
                return true;
            }
            else if (ev->matches(QKeySequence::Cut))
            {
                web->page()->triggerAction(QWebEnginePage::Cut);
                return true;
            }
            else if (ev->matches(QKeySequence::Paste))
            {
                web->page()->triggerAction(QWebEnginePage::Paste);
                return true;
            }
            else if (ev->matches(QKeySequence::Undo))
            {
                web->page()->triggerAction(QWebEnginePage::Undo);
                return true;
            }
            else if (ev->matches(QKeySequence::Redo))
            {
                web->page()->triggerAction(QWebEnginePage::Redo);
                return true;
            }
            else if (ev->matches(QKeySequence::SelectAll))
            {
                web->page()->triggerAction(QWebEnginePage::SelectAll);
                return true;
            }
            else if (ev->modifiers()&Qt::KeyboardModifier::ControlModifier && ev->key() == Qt::Key_Up)
            {
                //放大
                qreal factor = web->zoomFactor();
                factor += 0.1;
                factor = (factor > 5.0) ? 5.0 : factor;
                web->setZoomFactor(factor);
                return true;
            }
            else if (ev->modifiers()&Qt::KeyboardModifier::ControlModifier && ev->key() == Qt::Key_Down)
            {
                //缩小
                qreal factor = web->zoomFactor();
                factor -= 0.1;
                factor = (factor < 0.5) ? 0.5 : factor;
                web->setZoomFactor(factor);
                return true;
            }
        }
    }
    return false;
}

WizWebEngineViewContainerDialog::WizWebEngineViewContainerDialog(QWidget *parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{

}

void WizWebEngineViewContainerDialog::keyPressEvent(QKeyEvent* ev)
{
    if (WizWebEngineViewProgressKeyEvents(ev))
        return;
    //
    QDialog::keyPressEvent(ev);
}

void WizWebEngineView::wheelEvent(QWheelEvent *event)
{
    qreal factor = 0;

    if (event->modifiers()==Qt::ControlModifier) {
        factor = zoomFactor();
        if (event->delta() > 0) {
            //放大
            factor += 0.1;
            factor = (factor > 5.0)?5.0:factor;
        } else {
            //缩小
            factor -= 0.1;
            factor = (factor < 0.5)?0.5:factor;
        }
        //setZoomFactor(factor);
    } else {
        event->ignore();
    }
}
