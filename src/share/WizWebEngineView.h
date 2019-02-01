#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "WebSocketClientWrapper.h"
#include "WebSocketTransport.h"
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QDialog>

class QWebChannel;
class QMenu;

class WizWebEngineView;
class WizDevToolsDialog;

class WizWebEnginePage: public QWebEnginePage
{
    Q_OBJECT
public:
    explicit WizWebEnginePage(QObject* parent = nullptr);
    //
    void stopCurrentNavigation() { m_continueNavigate = false; }
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
    WizWebEngineView(QWidget* parent);
    virtual ~WizWebEngineView();
public:
    WizWebEnginePage* getPage();
    void addToJavaScriptWindowObject(QString name, QObject* obj);
    void closeAll();
    QMenu* createStandardContextMenu();
    QString documentTitle();

public Q_SLOTS:
    void innerLoadFinished(bool);
    void openLinkInDefaultBrowser(QUrl url);
    void openDevTools();
    void onViewSourceTriggered();
    void handleSavePageTriggered();
    
Q_SIGNALS:
    void loadFinishedEx(bool);
    void viewSourceRequested(QUrl url, QString title);
private:
    QWebSocketServer* m_server;
    WebSocketClientWrapper* m_clientWrapper;
    QWebChannel* m_channel;
    QString m_objectNames;
    WizDevToolsDialog* m_devToolsWindow = nullptr;
    //WizWebEnginePage* m_page;
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
