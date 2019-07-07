#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWebEngineView>
#include <QWebEnginePage>
#include <QDialog>
#include <QHash>

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
    explicit WizWebEnginePage(QObject* parent = nullptr): WizWebEnginePage({{}}, parent) { }
    WizWebEnginePage(const WizWebEngineInjectObjectCollection& objects, QObject* parent = nullptr);
    //
    void stopCurrentNavigation() { m_continueNavigate = false; }
    void addObjectToJavaScriptClient(QString name, QObject* obj);

protected:
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID);
    virtual bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame);
    virtual QWebEnginePage *createWindow(WebWindowType type);
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
    WizWebEngineView(QWidget* parent): WizWebEngineView({{}}, parent) { }
    WizWebEngineView(const WizWebEngineInjectObjectCollection& objects, QWidget* parent);
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

public Q_SLOTS:
    void innerLoadFinished(bool);
    void openLinkInDefaultBrowser(QUrl url);
    void openDevTools();
    void onViewSourceTriggered();
    void handleSavePageTriggered();
    void handleOpenDevToolsTriggered();
    
Q_SIGNALS:
    void loadFinishedEx(bool);
    void viewSourceRequested(QUrl url, QString title);
    
private:
    WizDevToolsDialog* m_devToolsWindow = nullptr;

protected:
    void wheelEvent(QWheelEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
};

bool WizWebEngineViewProgressKeyEvents(QKeyEvent* ev);

class WizWebEngineViewContainerDialog: public QDialog
{
public:
    WizWebEngineViewContainerDialog(QWidget *parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
protected:
    virtual void keyPressEvent(QKeyEvent* ev);
};

#endif // MAINWINDOW_H
