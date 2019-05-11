#ifndef JSPLUGINMANAGER_H
#define JSPLUGINMANAGER_H

#include "share/WizSettings.h"

#include <QAction>

class WizExplorerApp;
class JSPluginSelectorWindow;
class JSPluginSpec;
class WizWebsiteView;
class JSPluginModuleSpec;
class JSPluginSpec;
class JSPluginHtmlDialog;

class JSPluginManager : public QObject
{
    Q_OBJECT

public:
    JSPluginManager(QStringList &pluginScanPathList, WizExplorerApp& app, QObject *parent = nullptr);
    ~JSPluginManager();
    //
    QList<JSPluginModuleSpec *> modulesByButtonLocation(QString buttonLocation) const;
    QList<JSPluginModuleSpec *> modulesByKeyValue(QString key, QString value) const;
    JSPluginModuleSpec *moduleByGUID(QString guid) const;
    //
    static QAction *createPluginAction(QWidget *parent, JSPluginModuleSpec *moduleData);

    JSPluginHtmlDialog *initPluginHtmlDialog(JSPluginModuleSpec *moduleData);
    void showPluginHtmlDialog(JSPluginModuleSpec *moduleData);

    JSPluginSelectorWindow *initPluginSelectorWindow(JSPluginModuleSpec *moduleData);
    void showPluginSelectorWindow(JSPluginModuleSpec *moduleData, QPoint &pt);

    WizWebsiteView *initPluginMainTabView(JSPluginModuleSpec *moduleData);
    void showPluginMainTabView(JSPluginModuleSpec *moduleData);

private:
    void loadPluginData(QString &pluginScanPath);

public slots:
    void handlePluginActionTriggered();

private:
    WizExplorerApp &m_app;
    QList<JSPluginSpec *> m_pluginDataCollection;
    QHash<QString, JSPluginHtmlDialog *> m_pluginHtmlDialogCollection;
    QHash<QString, JSPluginSelectorWindow *> m_pluginPopupDialogCollection;
    QHash<QString, QPointer<WizWebsiteView > > m_pluginMainTabViewCollection;

};



#endif // JSPLUGINMANAGER_H
