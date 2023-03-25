#ifndef JSPLUGINSPEC_H
#define JSPLUGINSPEC_H

#include <QObject>
#include <QSettings>
#include <QDir>
#include <QVector>
#include <QStringList>

class JSPluginModuleSpec;

class JSPluginSpec : public QObject
{
    Q_OBJECT

public:
    JSPluginSpec(const QString &manifestFileName, QObject* parent);
    ~JSPluginSpec() = default;

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
    JSPluginModuleSpec *module(int index) { return m_modules[index]; }
    int moduleCount() { return m_moduleCount; }
    int manifestVersion() { return m_manifestVersion; }

private:
    void warn(const QString &msg);

private:
    QSettings *m_settings;
    QDir m_dir;
    QString m_path;
    QString m_guid;
    QString m_type;
    QString m_name;
    int m_moduleCount;
    int m_realModuleCount;
    int m_manifestVersion;
    QString m_apiMinimumRequired;
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
    ~JSPluginModuleSpec() = default;

    bool validateActionTypeModule();
    bool validateEditorTypeModule();
    bool validateMenuTypeModule();

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
    QStringList scriptFiles() { return m_scriptFiles; }
    QStringList styleFiles() { return m_styleFiles; }
    QStringList supportedFormats() { return m_supportedFormats; }
    QList<int> actionIndexes() { return m_actionIndexes; }
    int width() { return m_width; }
    int height() { return m_height; }
    QString sidebarLocation() { return m_sidebarLocation; }

private:
    QString getFilePath(QSettings *setting, const QString &key);
    QStringList getFileList(QSettings *setting, const QString &key);
    void warn(const QString &msg);

private:
    // base info

    JSPluginSpec *m_parentSpec;
    QString m_path;
    QDir m_dir;
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

    // ModuleType=Editor

    QStringList m_scriptFiles;
    QStringList m_styleFiles;
    QStringList m_supportedFormats;

    // ModuleType=Menu
    QList<int> m_actionIndexes;
    QString m_sidebarLocation;

public:
    friend class JSPluginSpec;

};

#endif // JSPLUGINSPEC_H
