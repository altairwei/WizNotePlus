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

/**
 * @brief The manager for JavaScript plugins.
 * 
 *      This singleton is design according to 
 *      https://stackoverflow.com/questions/1008019/c-singleton-design-pattern
 *      It is lazy-evaluated, correctly-destroyed, and thread-safe.
 * 
 */
class JSPluginManager : public QObject
{
    Q_OBJECT

public:
    static JSPluginManager& instance() {
        static JSPluginManager instance;
        return instance;
    }
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
    JSPluginManager();
    ~JSPluginManager();

    void loadPluginData(QString &pluginScanPath);

public:
    JSPluginManager(JSPluginManager const &) = delete;
    void operator=(JSPluginManager const &)  = delete;

public slots:
    void handlePluginActionTriggered();
    void notifyDocumentChanged();

private:
    WizExplorerApp &m_app;
    QList<JSPluginSpec *> m_pluginDataCollection;
    QHash<QString, JSPluginHtmlDialog *> m_pluginHtmlDialogCollection;
    QHash<QString, JSPluginSelectorWindow *> m_pluginPopupDialogCollection;
    QHash<QString, QPointer<WizWebsiteView > > m_pluginMainTabViewCollection;

};



#endif // JSPLUGINMANAGER_H
