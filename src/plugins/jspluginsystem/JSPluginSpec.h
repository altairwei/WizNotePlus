#ifndef JSPLUGINSPEC_H
#define JSPLUGINSPEC_H

#include "share/WizSettings.h"

#include <QObject>

class JSPluginSpec : public QObject
{
    Q_OBJECT

public:
    JSPluginSpec(QString path, QObject* parent);
    //
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString path READ path)
    Q_PROPERTY(QString guid READ guid)
    Q_PROPERTY(QString strings READ strings)

public:
    WizSettings *settings() { return m_settings; }
    QString path() const { return m_path; }
    QString guid() const { return m_guid; }
    QString type() const { return m_type; }
    QString name() const { return m_name; }
    QString strings() const { return m_strings; }
    QVector<JSPluginModuleSpec *> modules() { return m_modules; }
    void initStrings();
    void emitDocumentChanged();
    void emitShowEvent();

Q_SIGNALS:
    void documentChanged();
    void willShow();

private:
    WizSettings *m_settings;
    QString m_path;
    QString m_guid;
    QString m_type;
    QString m_name;
    QString m_strings;
    int m_moduleCount;
    QVector<JSPluginModuleSpec *> m_modules;
    //
    friend class JSPluginSelectorWindow;
    friend class JSPluginModuleSpec;
};

class JSPluginModuleSpec : public QObject
{
    Q_OBJECT

public:
    JSPluginModuleSpec(QString& section, WizSettings& setting, QObject* parent);
    //
    Q_PROPERTY(QString section READ section)
    Q_PROPERTY(QString caption READ caption)
    Q_PROPERTY(QString guid READ guid)
    Q_PROPERTY(QString moduleType READ moduleType)
    Q_PROPERTY(QString iconFileName READ iconFileName)
    Q_PROPERTY(QString htmlFileName READ htmlFileName)
    Q_PROPERTY(QString scriptFileName READ scriptFileName)

public:
    JSPluginSpec* parentPlugin() { return m_parentPlugin; }
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
    void emitDocumentChanged();
    void emitShowEvent();
    
Q_SIGNALS:
    void documentChanged();
    void willShow();

private:
    // base info
    
    JSPluginSpec* m_parentPlugin;
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

};

#endif // JSPLUGINSPEC_H