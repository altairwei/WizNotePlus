#include "WizWebsiteView.h"

#include <QWidget>
#include <QVBoxLayout>

#include "WizMainWindow.h"
#include "share/WizWebEngineView.h"

WizWebsiteView::WizWebsiteView(WizExplorerApp& app, QWidget* parent)
    : QWidget(parent)
    , m_sizeHint(QSize(200, 1))
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_userSettings(app.userSettings())
{
    // 创建布局
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    this->setLayout(layout);
    // 创建网页视图
    m_webView = new WizWebEngineView(this);
    WizWebEnginePage* webPage = new WizWebEnginePage(m_webView);
    m_webView->setPage(webPage);
    //
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    m_webView->addToJavaScriptWindowObject("WizExplorerApp", mainWindow->componentInterface());
    //
    layout->addWidget(m_webView);
    m_webView->show();

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
