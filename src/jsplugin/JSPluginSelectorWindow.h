#ifndef JSPLUGINSELECTORWINDOW_H
#define JSPLUGINSELECTORWINDOW_H

#include "share/WizPopupWidget.h"

class WizExplorerApp;
class WizWebEngineView;
class JSPluginModule;

class JSPluginSelectorWindow : public WizPopupWidget
{

    Q_OBJECT

public:
    JSPluginSelectorWindow(WizExplorerApp& app, JSPluginModule* module, QWidget* parent);

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

#endif // JSPLUGINSELECTORWINDOW_H