#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWebEngineView>
#include <QWebEnginePage>
#include <QDialog>
#include <QHash>
#include <QWebEngineProfile>
#include <QPointer>

#include "widgets/ShadowWidget.h"

class QWebChannel;
class QMenu;
class QLabel;

class WizWebEngineView;
class WizDevToolsDialog;
class WebPageZoomWidget;

typedef QHash<QString, QObject *> WizWebEngineInjectObjectCollection;

class WizWebEngineAsyncMethodResultObject: public QObject
{
    Q_OBJECT
public:
    WizWebEngineAsyncMethodResultObject(QObject* parent);
    virtual ~WizWebEngineAsyncMethodResultObject();
    Q_PROPERTY(QVariant result READ result NOTIFY resultAcquired)
    Q_PROPERTY(QVariant acquired READ acquired)
public:
    void setResult(const QVariant& result);
private:
    QVariant m_result;
    bool m_acquired;
    QVariant result() const { return m_result; }
    bool acquired() const { return m_acquired; }
Q_SIGNALS:
    void resultAcquired(const QVariant& ret);
};

class WizWebEnginePage: public QWebEnginePage
{
    Q_OBJECT

public:
    // Do not use {{}} to initialize InjectObjectCollection.
    explicit WizWebEnginePage(QObject* parent = nullptr): WizWebEnginePage({}, QWebEngineProfile::defaultProfile(), parent) { }
    WizWebEnginePage(QWebEngineProfile *profile, 
        QObject* parent = nullptr): WizWebEnginePage({}, profile, parent) { }
    WizWebEnginePage(const WizWebEngineInjectObjectCollection& objects, 
        QObject* parent = nullptr): WizWebEnginePage(objects, QWebEngineProfile::defaultProfile(), parent) { }
    WizWebEnginePage(const WizWebEngineInjectObjectCollection& objects, 
        QWebEngineProfile *profile, QObject* parent = nullptr);

    void stopCurrentNavigation() { m_continueNavigate = false; }
    static void processCopiedData();

protected:
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID);
    virtual bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame);
    virtual void triggerAction(WebAction action, bool checked = false);
    virtual void javaScriptAlert(const QUrl &securityOrigin, const QString& msg);
    virtual bool javaScriptConfirm(const QUrl &securityOrigin, const QString& msg);
    virtual bool javaScriptPrompt(const QUrl &securityOrigin, const QString& msg, const QString& defaultValue, QString* result);

Q_SIGNALS:
    void linkClicked(QUrl url, QWebEnginePage::NavigationType type, bool isMainFrame, WizWebEnginePage* page);
    void openLinkInNewWindow(QUrl url);

private:
    bool m_continueNavigate;
};


class WizWebEngineView : public QWebEngineView
{
    Q_OBJECT

public:
    WizWebEngineView(const WizWebEngineInjectObjectCollection& objects, QWidget* parent = nullptr);
    // Do not use {{}} to initialize InjectObjectCollection.
    WizWebEngineView(QWidget* parent = nullptr): WizWebEngineView({}, parent) { }
    virtual ~WizWebEngineView();

public:
    enum ViewAction {
        OpenDevTools,
        OpenTempFileLocation,

        ViewActionCount
    };

    QAction *viewAction(ViewAction action) const;
    WizWebEnginePage* getPage();
    QMenu* createStandardContextMenu();
    virtual QString documentTitle();

    void addObjectToJavaScriptClient(QString name, QObject* obj);

    Q_INVOKABLE QVariant ExecuteScript(QString script);
    Q_INVOKABLE QVariant ExecuteScriptFile(QString fileName);
    Q_INVOKABLE QVariant ExecuteFunction0(QString function);
    Q_INVOKABLE QVariant ExecuteFunction1(QString function, const QVariant& arg1);
    Q_INVOKABLE QVariant ExecuteFunction2(QString function, const QVariant& arg1, const QVariant& arg2);
    Q_INVOKABLE QVariant ExecuteFunction3(QString function, const QVariant& arg1, const QVariant& arg2, const QVariant& arg3);
    Q_INVOKABLE QVariant ExecuteFunction4(QString function, const QVariant& arg1, const QVariant& arg2, const QVariant& arg3, const QVariant& arg4);

    Q_INVOKABLE void SetZoom(int percent);
    Q_INVOKABLE int GetZoom();
    void displayZoomWidget();

public Q_SLOTS:
    void innerLoadFinished(bool);
    void openLinkInDefaultBrowser(QUrl url);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    void openDevTools();
#endif
    double scaleUp();
    double scaleDown();
    void handleShowZoomWidgetRequest(bool show, const QRect &btnLocation);

private:
    void createZoomWidget();

Q_SIGNALS:
    void loadFinishedEx(bool);
    void zoomFactorChanged(qreal factor);
    void zoomWidgetFinished();

private:
    WizDevToolsDialog* m_devToolsWindow = nullptr;
    mutable QAction *m_viewActions[ViewActionCount];
    QPointer<WebPageZoomWidget> m_zoomWgt;

protected:
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override;
    virtual void setupPage(WizWebEnginePage *page);
    virtual void setupWebActions();
    void contextMenuEvent(QContextMenuEvent *event) override;
    void childEvent(QChildEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *ev) override;
    void hideEvent(QHideEvent *event) override;

};

class WizWebEngineViewContainerDialog: public QDialog
{
public:
    WizWebEngineViewContainerDialog(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
};

QWebEngineProfile* createWebEngineProfile(const WizWebEngineInjectObjectCollection& objects, QObject* parent);
void insertStyleSheet(QWebEngineProfile *profile, const QString &name, const QString &source);
void insertScrollbarStyleSheet(QWebEngineProfile *profile);

class WizNavigationForwarderPage : public QWebEnginePage
{
    Q_OBJECT

public:
    WizNavigationForwarderPage(QWebEngineView *ownerView, QObject *parent = nullptr);

    void setWebWindowType(QWebEnginePage::WebWindowType type);

protected:
    virtual bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame);

private:
    QWebEngineView *m_ownerView;
    QWebEnginePage::WebWindowType m_windowType;
};


class WizNavigationForwarderView : public QWebEngineView
{
    Q_OBJECT

public:
    WizNavigationForwarderView(QWebEngineView *ownerView, QWidget* parent = nullptr);

    QWebEngineView *forward(QWebEnginePage::WebWindowType type);

private:
    WizNavigationForwarderPage *m_page;
};

class WebPageZoomWidget : public ShadowWidget
{
    Q_OBJECT

public:
    explicit WebPageZoomWidget(QWidget *parent = nullptr)
        : WebPageZoomWidget(1, parent) { }
    WebPageZoomWidget(qreal factor, QWidget *parent = nullptr);

    void setZoomFactor(qreal factor);

Q_SIGNALS:
    void zoomFinished();
    void scaleUpRequested();
    void scaleDownRequested();
    void resetZoomFactorRequested();

protected:
    void closeEvent(QCloseEvent *event) override;

public Q_SLOTS:
    void onZoomFactorChanged(qreal factor);

private:
    QPushButton *m_resetBtn;
    QPushButton *m_scaleUpBtn;
    QPushButton *m_scaleDownBtn;
    QLabel *m_factorLabel;
};

#endif // MAINWINDOW_H
