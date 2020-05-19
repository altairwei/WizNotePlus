#ifndef JSPLUGINHTMLDIALOG_H
#define JSPLUGINHTMLDIALOG_H

#include <QWidget>

class WizExplorerApp;
class WizWebEngineView;
class JSPluginModuleSpec;

class JSPluginHtmlDialog : public QWidget
{
    Q_OBJECT

public:
    JSPluginHtmlDialog(WizExplorerApp& app, JSPluginModuleSpec* data, QWidget* parent);
    QSize dialogSize() const;

private:
    WizWebEngineView* m_web;
    JSPluginModuleSpec* m_data;
    int m_dialogWidth;
    int m_dialogHeight;
    //
private:
    virtual QSize sizeHint() const;
    //
    friend class JSPluginSpec;
    friend class JSPluginModuleSpec;
};

#endif // JSPLUGINHTMLDIALOG_H