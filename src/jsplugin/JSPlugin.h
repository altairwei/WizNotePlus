#ifndef JSPLUGIN_JSPLUGIN_H
#define JSPLUGIN_JSPLUGIN_H

#include <QObject>
#include <QVector>

#include "JSPluginSpec.h"

class JSPluginModule;

class JSPlugin : public QObject
{
    Q_OBJECT

public:
    JSPlugin(QString &pluginFolder, QObject *parent);

    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString PluginPath READ path)
    Q_PROPERTY(QString guid READ guid)
    Q_PROPERTY(QString strings READ strings)

    JSPluginSpec *spec() { return m_data; }
    QVector<JSPluginModule *> modules() { return m_modules; }
    JSPluginModule *module(int i) { return m_modules[i]; }

    QString name() { return m_data->name(); }
    QString path() { return m_data->path(); }
    QString guid() { return m_data->guid(); }
    QString strings() { return m_strings; }
 
    /** Validate the plugin spec. */
    bool isAvailable();

    /** Load language localisation of the plugin. */
    void initStrings();

    void emitDocumentChanged();
    void emitShowEvent();

private:
    JSPluginSpec *m_data;
    QString m_strings;
    QVector<JSPluginModule *> m_modules;
};


class JSPluginModule : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(QObject* Spec READ spec)

    JSPluginModuleSpec *spec() { return m_data; }
    JSPlugin* parentPlugin() { return m_parentPlugin; }

    /** Validate the module spec. */
    bool isAvailable();

    void emitDocumentChanged();
    void emitShowEvent();

private:
    JSPluginModule(JSPluginModuleSpec *moduleSpec, QObject *parent);

Q_SIGNALS:
    void documentChanged();
    void documentHtmlChanged();
    void willShow();

private:
    JSPluginModuleSpec *m_data;
    JSPlugin* m_parentPlugin;

public:
    friend class JSPlugin;
};

#endif // JSPLUGIN_JSPLUGIN_H
