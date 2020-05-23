#ifndef JSPLUGINSPEC_H
#define JSPLUGINSPEC_H

#include <QObject>
#include <QSettings>
#include <QDir>
#include <QVector>

class JSPluginModuleSpec;

class JSPluginSpec : public QObject
{
    Q_OBJECT

public:
    JSPluginSpec(QString path, QObject* parent);
    //
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString path READ path)
    Q_PROPERTY(QString PluginPath READ path)
    Q_PROPERTY(QString guid READ guid)

    /** Validate the plugin specification. */
    bool validate();

public:
    QSettings *settings() { return m_settings; }
    QString path() const { return m_path; }
    QString guid() const { return m_guid; }
    QString type() const { return m_type; }
    QString name() const { return m_name; }
    QVector<JSPluginModuleSpec *> modules() { return m_modules; }

private:
    QSettings *m_settings;
    QDir m_dir;
    QString m_path;
    QString m_guid;
    QString m_type;
    QString m_name;
    int m_moduleCount;
    int m_realModuleCount;
    QVector<JSPluginModuleSpec *> m_modules;

public:
    friend class JSPluginSelectorWindow;
    friend class JSPluginModuleSpec;
};

class JSPluginModuleSpec : public QObject
{
    Q_OBJECT

public:
    Q_PROPERTY(QString section READ section)
    Q_PROPERTY(QString caption READ caption)
    Q_PROPERTY(QString guid READ guid)
    Q_PROPERTY(QString moduleType READ moduleType)
    Q_PROPERTY(QString iconFileName READ iconFileName)
    Q_PROPERTY(QString htmlFileName READ htmlFileName)
    Q_PROPERTY(QString scriptFileName READ scriptFileName)

    /** Validate the plugin module specification. */
    bool validate();

private:
    JSPluginModuleSpec(QString& section, QSettings *setting, QObject* parent);

public:
    JSPluginSpec *parentSpec() { return m_parentSpec; }
    QString section() { return m_section; }
    QString caption() { return m_caption; }
    QString guid() { return m_guid; }
    QString slotType() { return m_slotType; }
    QString moduleType() { return m_moduleType; }
    QString buttonLocation() { return m_buttonLocation; }
    QString menuLocation() { return m_menuLocation; }
    QString iconFileName() { return m_iconFileName; }
    QString htmlFileName() { return m_htmlFileName; }
    QString scriptFileName() { return m_scriptFileName; }
    int width() { return m_width; }
    int height() { return m_height; }

private:
    // base info

    JSPluginSpec *m_parentSpec;
    QString m_path;
    QString m_section;
    QString m_caption;
    QString m_guid;
    QString m_moduleType;

    // ModuleType=Action

    QString m_slotType;
    QString m_buttonLocation;
    QString m_menuLocation;
    QString m_iconFileName;
    QString m_htmlFileName;
    QString m_scriptFileName;
    int m_width;
    int m_height;

public:
    friend class JSPluginSpec;

};

#endif // JSPLUGINSPEC_H