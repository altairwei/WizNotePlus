#ifndef WIZMAINTABBROWSER_H
#define WIZMAINTABBROWSER_H

#include <QTabWidget>
#include <QMouseEvent>
#include <QScopedPointer>
#include <QWebEngineFullScreenRequest>

#include "share/WizWebEngineView.h"
#include "gui/documentviewer/WizDocumentWebView.h"
#include "FullScreenWindow.h"
#include "AbstractTabPage.h"

class QUrl;

class WizExplorerApp;
class WizDatabaseManager;
class WizDocumentView;
class WizDocumentWebView;
class WizWebsiteView;


class WizMainTabBrowser : public QTabWidget
{
    Q_OBJECT

public:

    WizMainTabBrowser(WizExplorerApp& app, QWidget *parent = nullptr);

    //WizDocumentView* currentDocView();
    WizWebEngineView* currentWebView() const;
    WizWebEngineView* getWebView(int index) const;
    AbstractTabPage *tabPage(int index) const;

    QMap<QString, QVariant> tabStatus(int index) const;
    bool isTabLocked(int index) const;

private:
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;
    QString m_strTheme;
    QScopedPointer<FullScreenWindow> m_fullScreenWindow;
    QList<AbstractTabPage *> m_scheduleForClose;

signals:
    void titleChanged(const QString &title);

public slots:
    void handleCurrentChanged(int index);
    void handleContextMenuRequested(const QPoint &pos);
    void handleCloseButtonClicked();
    WizWebEngineView *createTab();
    WizWebEngineView *createBackgroundTab();
    WizWebEngineView *createWindow();
    int createTab(const QUrl &url);
    int createTab(AbstractTabPage *tabPage);
    void destroyTab(int index);
    void closeTab(int index, bool force = false);
    void closeCurrentTab();
    void closeOtherTabs(int index);
    void closeAllTabs();
    void closeLeftTabs(int index);
    void closeRightTabs(int index);
    void lockTab(int index);
    void unlockTab(int index);

    void triggeredFullScreen();
    void handleExitFullScreen();
    void fullScreenRequested(QWebEngineFullScreenRequest request);
    void handleTabCloseRequested(int index);

private:
    void setupTab(int index);
    void setupView(WizWebEngineView* view);
    void setupTabPage(AbstractTabPage *tabPage);
    void switchTabStatus(int index, bool lock);
    void doCloseSchedule();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent* ev) override;

public:
    bool eventFilter(QObject* watched, QEvent* event) override;
};

#endif // WIZTABWIDGET_H
