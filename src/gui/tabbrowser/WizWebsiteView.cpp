#include "WizWebsiteView.h"

#include <QWidget>
#include <QVBoxLayout>

#include "WizMainWindow.h"
#include "share/WizWebEngineView.h"

WizWebsiteView::WizWebsiteView(WizWebEngineView *webView, WizExplorerApp& app, QWidget* parent)
    : AbstractTabPage(parent)
    , m_webView(webView)
    , m_sizeHint(QSize(200, 1))
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
    if (!m_webView) {
        m_webView = new WizWebEngineView;
    }
    // set web view
    layout->addWidget(m_webView);
    m_webView->show();

    connect(m_webView->page(), &QWebEnginePage::windowCloseRequested,
        this, &WizWebsiteView::handleWindowCloseRequested);
    connect(m_webView->page(), &QWebEnginePage::titleChanged,
        this, &WizWebsiteView::titleChanged);
}

WizWebsiteView::~WizWebsiteView()
{

}

QSize WizWebsiteView::sizeHint() const
{
    return m_sizeHint;
}

void WizWebsiteView::setSizeHint(QSize size)
{
    m_sizeHint = size;
}

void WizWebsiteView::viewHtml(const QUrl &url)
{
    m_webView->setUrl(url);
}

QString WizWebsiteView::Title()
{
    return m_webView->title();
}

void WizWebsiteView::RequestClose(bool force /*= false*/)
{
    if (force) {
        handleWindowCloseRequested();
    } else {
        m_webView->triggerPageAction(QWebEnginePage::RequestClose);
    }
}

void WizWebsiteView::handleWindowCloseRequested()
{
    emit pageCloseRequested();
}
