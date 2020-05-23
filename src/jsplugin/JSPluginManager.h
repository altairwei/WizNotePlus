#ifndef JSPLUGINMANAGER_H
#define JSPLUGINMANAGER_H

#include <QAction>

class WizExplorerApp;
class JSPluginSelectorWindow;
class JSPluginSpec;
class WizWebsiteView;
class JSPlugin;
class JSPluginModule;
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
    QList<JSPluginModule *> modulesByButtonLocation(QString buttonLocation) const;
    QList<JSPluginModule *> modulesByKeyValue(QString key, QString value) const;
    JSPluginModule *moduleByGUID(QString guid) const;
    //
    static QAction *createPluginAction(QWidget *parent, JSPluginModule *moduleData);

    JSPluginHtmlDialog *initPluginHtmlDialog(JSPluginModule *moduleData);
    void showPluginHtmlDialog(JSPluginModule *moduleData);

    JSPluginSelectorWindow *initPluginSelectorWindow(JSPluginModule *moduleData);
    void showPluginSelectorWindow(JSPluginModule *moduleData, QPoint &pt);

    WizWebsiteView *initPluginMainTabView(JSPluginModule *moduleData);
    void showPluginMainTabView(JSPluginModule *moduleData);

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
    QList<JSPlugin *> m_pluginDataCollection;
    QHash<QString, JSPluginHtmlDialog *> m_pluginHtmlDialogCollection;
    QHash<QString, JSPluginSelectorWindow *> m_pluginPopupDialogCollection;
    QHash<QString, QPointer<WizWebsiteView > > m_pluginMainTabViewCollection;

};



#endif // JSPLUGINMANAGER_H
