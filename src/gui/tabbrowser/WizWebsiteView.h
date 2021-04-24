#ifndef GUI_TABBROWSER_WIZWEBSITEVIEW_H
#define GUI_TABBROWSER_WIZWEBSITEVIEW_H

#include <QWidget>

#include "AbstractTabPage.h"

class WizExplorerApp;
class WizDatabaseManager;
class WizUserSettings;
class WizWebEngineView;
class WizObjectDownloaderHost;

class WizWebsiteView  : public AbstractTabPage
{
    Q_OBJECT

public:
    WizWebsiteView(WizWebEngineView *webView, WizExplorerApp& app, QWidget* parent = nullptr);
    WizWebsiteView(WizExplorerApp& app, QWidget* parent = nullptr) : WizWebsiteView(nullptr, app, parent) {};
    ~WizWebsiteView();

    virtual QSize sizeHint() const;
    void setSizeHint(QSize size);

    WizWebEngineView* getWebView() const { return m_webView; }
    WizWebEngineView* webView() const { return m_webView; }
    void viewHtml(const QUrl &url);

    QString Title() override;
    void RequestClose(bool force = false) override;

public slots:
    void handleWindowCloseRequested();

private:
    QSize m_sizeHint;
    WizWebEngineView* m_webView;

protected:
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;
    WizUserSettings& m_userSettings;
    WizObjectDownloaderHost* m_downloaderHost;
};

#endif // GUI_TABBROWSER_WIZWEBSITEVIEW_H
