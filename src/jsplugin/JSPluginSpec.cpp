#include "JSPluginSpec.h"

#include <algorithm>

#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QDir>


JSPluginSpec::JSPluginSpec(QString path, QObject* parent)
    : QObject(parent)
{
    m_dir = QDir(path);
    m_path = m_dir.absolutePath() + "/";
    QString fileName = m_dir.absoluteFilePath("manifest.ini");

    m_settings = new QSettings(fileName);
    m_settings->setIniCodec("UTF-8");

    // Parse 'Common' setiong
    m_name = m_settings->value("Common/AppName", "").toString();
    m_type = m_settings->value("Common/Type", "").toString();
    m_guid = m_settings->value("Common/AppGUID", "").toString();
    m_moduleCount = m_settings->value("Common/ModuleCount", 0).toInt();

    // Parse Modules
    m_realModuleCount = 0;
    QStringList groups = m_settings->childGroups();
    for (QString& pluginIndex : groups) {
        if (!pluginIndex.contains("Module_"))
            continue;
        m_realModuleCount++;
    }
    if (m_realModuleCount != m_moduleCount)
        qWarning() << QString("App %s's ModuleCount not correct!").arg(m_name);
}


bool JSPluginSpec::validate() {
    //TODO: check common information.
    return true;
}


JSPluginModuleSpec::JSPluginModuleSpec(QString& section, QSettings *setting, QObject* parent)
    : QObject(parent)
    , m_section(section)
{
    m_parentSpec = qobject_cast<JSPluginSpec*>(parent);
    m_path = m_parentSpec->path();
    m_caption = setting->value(section + "/Caption", "").toString();
    m_guid = setting->value(section + "/GUID", "").toString();
    m_moduleType = setting->value(section + "/ModuleType", "").toString();

    m_slotType = setting->value(section + "/SlotType", "").toString();
    m_buttonLocation = setting->value(section + "/ButtonLocation", "").toString();
    m_menuLocation = setting->value(section + "/MenuLocation", "").toString();
    m_iconFileName = m_path + setting->value(section + "/IconFileName", "").toString();
    m_htmlFileName = m_path + setting->value(section + "/HtmlFileName").toString();
    m_scriptFileName = m_path + setting->value(section + "/ScriptFileName").toString();
    m_width = setting->value(section + "/Width", 800).toInt();
    m_height = setting->value(section + "/Height", 500).toInt();
}


bool JSPluginModuleSpec::validate() {
    //TODO: wait to impl
    return true;
}