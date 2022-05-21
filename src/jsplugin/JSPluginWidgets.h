#ifndef JSPLUGINHTMLDIALOG_H
#define JSPLUGINHTMLDIALOG_H

#include <QWidget>
#include "share/WizPopupWidget.h"

class WizExplorerApp;
class WizWebEngineView;
class JSPluginModule;

class JSPluginWebsiteView : public QWidget
{
    Q_OBJECT

public:
    JSPluginWebsiteView(WizExplorerApp& app,
                        JSPluginModule* module,
                        QWidget* parent = nullptr);

protected:
    WizWebEngineView* m_web;
    JSPluginModule* m_module;
};

class JSPluginHtmlDialog : public JSPluginWebsiteView
{
    Q_OBJECT

public:
    JSPluginHtmlDialog(WizExplorerApp& app,
                       JSPluginModule* module,
                       QWidget* parent = nullptr);
    QSize dialogSize() const;

private:
    int m_dialogWidth;
    int m_dialogHeight;

private:
    virtual QSize sizeHint() const;

};

class JSPluginSelectorWindow : public WizPopupWidget
{

    Q_OBJECT

public:
    JSPluginSelectorWindow(WizExplorerApp& app,
                           JSPluginModule* module,
                           QWidget* parent = nullptr);

public:
    WizWebEngineView* web() const {return m_web; }

private:
    WizWebEngineView* m_web;
    JSPluginModule* m_module;
    int m_windowWidth;
    int m_windowHeight;

private:
    virtual QSize sizeHint() const;

};

class JSPluginDocSidebar : public JSPluginWebsiteView
{
    Q_OBJECT

public:
    explicit JSPluginDocSidebar(WizExplorerApp& app,
                                JSPluginModule* module,
                                QWidget *parent = nullptr);

    virtual QSize sizeHint() const;
    JSPluginModule* module() { return m_module; }

Q_SIGNALS:
    void documentHtmlChanged();
    void editorLoadFinished();

private:
    int m_sidebarWidth;

};

#endif // JSPLUGINHTMLDIALOG_H
