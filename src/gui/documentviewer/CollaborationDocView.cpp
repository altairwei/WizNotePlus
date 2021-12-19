#include "CollaborationDocView.h"

#include <QWebEnginePage>
#include <QVBoxLayout>

#include "share/WizWebEngineView.h"
#include "share/WizThreads.h"
#include "sync/WizToken.h"
#include "WizDef.h"
#include "WizMainWindow.h"

CollaborationDocView::CollaborationDocView(const WIZDOCUMENTDATAEX &doc, WizExplorerApp &app, QWidget *parent)
    : AbstractTabPage(parent)
    , m_doc(doc)
    , m_webView(nullptr)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_userSettings(app.userSettings())
{
    // set layout
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    this->setLayout(layout);

    // create default web view
    m_webView = new WizWebEngineView(this);
    WizWebEngineInjectObjectCollection objects = {{"wizQt", this}};
    auto profile = createWebEngineProfile(objects, m_webView);
    auto webPage = new WizWebEnginePage(objects, profile, m_webView);
    m_webView->setPage(webPage);

    QUrl liveEditorUrl("https://www.wiz.cn/note-plus/note/" + doc.strKbGUID + "/" + doc.strGUID);
    liveEditorUrl.setQuery("l=zh-cn&clientType=macos&clientVersion=4.13.32.0&p=wiz&theme=light&disableVideo=1&top=0");
    m_webView->load(liveEditorUrl);
    layout->addWidget(m_webView);
    m_webView->show();

    connect(m_webView->page(), &QWebEnginePage::windowCloseRequested,
        this, &CollaborationDocView::handleWindowCloseRequested);
    connect(m_webView->page(), &QWebEnginePage::titleChanged,
        this, &CollaborationDocView::titleChanged);
}

CollaborationDocView::~CollaborationDocView()
{
    if (m_webView) {
        // We need to delete WizWebEngineView first to avoid dispatching
        // pageCloseRequested() signal via QWebChannel.
        delete m_webView;
        m_webView = nullptr;
    }
}

QString CollaborationDocView::Title()
{
    return m_doc.strTitle;
}

void CollaborationDocView::RequestClose(bool force /*= false*/)
{
    if (force) {
        handleWindowCloseRequested();
    } else {
        m_webView->triggerPageAction(QWebEnginePage::RequestClose);
    }
}

void CollaborationDocView::handleWindowCloseRequested()
{
    emit pageCloseRequested();
}

void CollaborationDocView::GetToken(const QString &func)
{
    QString functionName(func);
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=] {
        QString strToken = WizToken::token();
        if (strToken.isEmpty()) return;
        WizExecuteOnThread(WIZ_THREAD_MAIN, [=] {
            QString strExec = functionName + QString("('%1')").arg(strToken);
            m_webView->page()->runJavaScript(strExec);
        });
    });
}
