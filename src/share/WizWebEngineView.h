#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWebEngineView>
#include <QWebEnginePage>
#include <QDialog>
#include <QHash>
#include <QWebEngineProfile>

class QWebChannel;
class QMenu;

class WizWebEngineView;
class WizDevToolsDialog;

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
    //
    void stopCurrentNavigation() { m_continueNavigate = false; }

protected:
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID);
    virtual bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame);
    virtual void triggerAction(WebAction action, bool checked = false);

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
    WizWebEngineView(const WizWebEngineInjectObjectCollection& objects, QWidget* parent);
    // Do not use {{}} to initialize InjectObjectCollection.
    WizWebEngineView(QWidget* parent): WizWebEngineView({}, parent) { }
    virtual ~WizWebEngineView();

public:
    WizWebEnginePage* getPage();
    QMenu* createStandardContextMenu();
    QString documentTitle();

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

    double scaleUp();
    double scaleDown();

public Q_SLOTS:
    void innerLoadFinished(bool);
    void openLinkInDefaultBrowser(QUrl url);
#if QT_VERSION >= 0x051100
    void openDevTools();
    void handleOpenDevToolsTriggered();
#endif
    void handleSavePageTriggered();
    
Q_SIGNALS:
    void loadFinishedEx(bool);

private:
    WizDevToolsDialog* m_devToolsWindow = nullptr;

protected:
    QWebEngineView *createWindow(QWebEnginePage::WebWindowType type) override;
    void contextMenuEvent(QContextMenuEvent *event);
    void childEvent(QChildEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *ev) override;

};

class WizWebEngineViewContainerDialog: public QDialog
{
public:
    WizWebEngineViewContainerDialog(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
};

QWebEngineProfile* createWebEngineProfile(const WizWebEngineInjectObjectCollection& objects, QObject* parent);

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

#endif // MAINWINDOW_H
