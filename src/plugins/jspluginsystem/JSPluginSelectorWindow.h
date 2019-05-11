#ifndef JSPLUGINSELECTORWINDOW_H
#define JSPLUGINSELECTORWINDOW_H

#include "share/WizPopupWidget.h"

class WizExplorerApp;
class WizWebEngineView;
class JSPluginModuleSpec;
class JSPluginSpec;

class JSPluginSelectorWindow : public WizPopupWidget
{

public:
    JSPluginSelectorWindow(WizExplorerApp& app, JSPluginModuleSpec* data, QWidget* parent);

public:
    WizWebEngineView* web() const {return m_web; }

private:
    WizWebEngineView* m_web;
    JSPluginModuleSpec* m_data;
    int m_windowWidth;
    int m_windowHeight;

private:
    virtual QSize sizeHint() const;
    //
    friend class JSPluginSpec;
    friend class JSPluginModuleSpec;
};

#endif // JSPLUGINSELECTORWINDOW_H