#ifndef WIZMAINTABBROWSER_H
#define WIZMAINTABBROWSER_H

#include <QTabWidget>
#include <QAbstractButton>
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
class QStyleOptionButton;
class QStyleOptionTabBarBase;

class TabButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit TabButton(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override
        { return sizeHint(); }
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void drawTabBtn(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget = nullptr) const;
};

class WizMainTabBrowser : public QTabWidget
{
    Q_OBJECT

public:

    typedef QMap<QString, QVariant> TabStatusData;

    WizMainTabBrowser(WizExplorerApp& app, QWidget *parent = nullptr);

    //WizDocumentView* currentDocView();
    WizWebEngineView* currentWebView() const;
    WizWebEngineView* getWebView(int index) const;
    AbstractTabPage *tabPage(int index) const;

private:
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;
    QString m_strTheme;
    QScopedPointer<FullScreenWindow> m_fullScreenWindow;

signals:
    void titleChanged(const QString &title);

public slots:
    void handleCurrentChanged(int index);
    void handleContextMenuRequested(const QPoint &pos);
    WizWebEngineView *createTab();
    WizWebEngineView *createBackgroundTab();
    WizWebEngineView *createWindow();
    int createTab(const QUrl &url);
    int createTab(AbstractTabPage *tabPage);
    void closeTab(int index);
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
    void setupView(WizWebEngineView* view);
    void setupTabPage(AbstractTabPage *tabPage);
};

#endif // WIZTABWIDGET_H
