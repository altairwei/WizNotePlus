#ifndef JSPLUGINMANAGER_H
#define JSPLUGINMANAGER_H

#include <QAction>

#include "share/WizObject.h"

class WizExplorerApp;
class WizMainWindow;
class JSPluginSelectorWindow;
class JSPluginSpec;
class WizWebsiteView;
class JSPlugin;
class JSPluginModule;
class JSPluginModuleSpec;
class JSPluginSpec;
class JSPluginHtmlDialog;
class JSPluginDocSidebar;

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

    void setMainWindow(WizMainWindow *mv) { m_mainWindow = mv; }

    QList<JSPluginModule *> modulesByButtonLocation(QString buttonLocation) const;
    QList<JSPluginModule *> modulesByModuleType(QString type) const;
    QList<JSPluginModule *> modulesByKeyValue(QString key, QString value) const;
    JSPluginModule *moduleByGUID(QString guid) const;

    static QAction *createPluginAction(QWidget *parent, JSPluginModule *moduleData);
    static void initPluginAction(QAction *ac, JSPluginModule *moduleData);

    JSPluginHtmlDialog *initPluginHtmlDialog(JSPluginModule *moduleData);
    void showPluginHtmlDialog(JSPluginModule *moduleData);

    void showPluginSelectorWindow(JSPluginModule *moduleData, const QPoint &pt);

    WizWebsiteView *initPluginMainTabView(JSPluginModule *moduleData);
    void showPluginMainTabView(JSPluginModule *moduleData);

    void executeModuleScript(JSPluginModule *moduleData);

private:
    JSPluginManager();
    ~JSPluginManager();

    void loadPluginData(QString &pluginScanPath);

public:
    JSPluginManager(JSPluginManager const &) = delete;
    void operator=(JSPluginManager const &)  = delete;

public slots:
    void handlePluginActionTriggered();
    void handleDocumentChanged();
    void handlePluginEditorRequest(const WIZDOCUMENTDATA &doc, const QString &guid);
    void handlePluginPopupRequest(QAction *ac, const QPoint &pos);

private:
    WizMainWindow *m_mainWindow;
    QList<JSPlugin *> m_pluginDataCollection;
    QHash<QString, JSPluginHtmlDialog *> m_pluginHtmlDialogCollection;
    QHash<QString, QPointer<WizWebsiteView > > m_pluginMainTabViewCollection;
};



#endif // JSPLUGINMANAGER_H
