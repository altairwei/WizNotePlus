#ifndef WIZWEBSITEVIEW_H
#define WIZWEBSITEVIEW_H

#include <QWidget>

class WizExplorerApp;
class WizDatabaseManager;
class WizUserSettings;
class WizWebEngineView;
class WizObjectDownloaderHost;

class WizWebsiteView  : public QWidget
{
    Q_OBJECT

public:
    WizWebsiteView(WizWebEngineView *webView, WizExplorerApp& app, QWidget* parent = nullptr);
    WizWebsiteView(WizExplorerApp& app, QWidget* parent = nullptr) : WizWebsiteView(nullptr, app, parent) {};
    ~WizWebsiteView();

    virtual QSize sizeHint() const;
    void setSizeHint(QSize size);

    WizWebEngineView* getWebView() const { return m_webView; }
    void viewHtml(const QUrl &url);

private:
    QSize m_sizeHint;
    WizWebEngineView* m_webView;

protected:
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;
    WizUserSettings& m_userSettings;
    WizObjectDownloaderHost* m_downloaderHost;
};

#endif // WIZWEBSITEVIEW_H
