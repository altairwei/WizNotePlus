#ifndef JSPLUGINHTMLDIALOG_H
#define JSPLUGINHTMLDIALOG_H

#include <QWidget>

class WizExplorerApp;
class WizWebEngineView;
class JSPluginModule;

class JSPluginHtmlDialog : public QWidget
{
    Q_OBJECT

public:
    JSPluginHtmlDialog(WizExplorerApp& app, JSPluginModule* module, QWidget* parent);
    QSize dialogSize() const;

private:
    WizWebEngineView* m_web;
    JSPluginModule* m_module;
    int m_dialogWidth;
    int m_dialogHeight;

private:
    virtual QSize sizeHint() const;

};

#endif // JSPLUGINHTMLDIALOG_H