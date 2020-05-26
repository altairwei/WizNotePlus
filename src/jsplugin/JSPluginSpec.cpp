#include "JSPluginSpec.h"

#include <algorithm>

#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QVector>


JSPluginSpec::JSPluginSpec(const QString &manifestFileName, QObject* parent)
    : QObject(parent)
{
    QFileInfo manifestFile(manifestFileName);
    m_dir = manifestFile.absoluteDir();
    m_path = m_dir.absolutePath() + "/";

    m_settings = new QSettings(manifestFile.absoluteFilePath(), QSettings::IniFormat);
    m_settings->setIniCodec("UTF-8");

    // Parse 'Common' setiong
    m_name = m_settings->value("Common/AppName", "").toString();
    m_type = m_settings->value("Common/AppType", "").toString();
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
    const QString appDirName = m_dir.dirName();

    if (m_name.isEmpty() || m_guid.isEmpty()) {
        qWarning() << QString("%1: AppName or AppGUID is empty.").arg(appDirName);
        return false;
    }

    if (m_moduleCount != m_realModuleCount) {
        qWarning() << QString("%1: ModuleCount isn't equal to real count.").arg(appDirName);
        return false;
    }

    return true;
}


JSPluginModuleSpec::JSPluginModuleSpec(QString& section, QSettings *setting, QObject* parent)
    : QObject(parent)
    , m_section(section)
{
    m_parentSpec = qobject_cast<JSPluginSpec*>(parent);
    m_path = m_parentSpec->path() + "/";
    m_dir = m_parentSpec->m_dir;

    // Module common information
    m_caption = setting->value(section + "/Caption", "").toString();
    m_guid = setting->value(section + "/GUID", "").toString();
    m_moduleType = setting->value(section + "/ModuleType", "").toString();
    m_iconFileName = m_path + setting->value(section + "/IconFileName", "").toString();

    // Action type module
    m_slotType = setting->value(section + "/SlotType", "").toString();
    m_buttonLocation = setting->value(section + "/ButtonLocation", "").toString();
    m_menuLocation = setting->value(section + "/MenuLocation", "").toString();
    m_htmlFileName = m_path + setting->value(section + "/HtmlFileName", "").toString();
    m_scriptFileName = m_path + setting->value(section + "/ScriptFileName", "").toString();
    m_width = setting->value(section + "/Width", 800).toInt();
    m_height = setting->value(section + "/Height", 500).toInt();

    // Editor type module
    m_scriptFiles = setting->value(section + "/ScriptFiles", "").toString().split(',');
    m_styleFiles = setting->value(section + "/StyleFiles", "").toString().split(',');
    m_supportedFormats = setting->value(section + "/SupportedFormats", "").toString().split(',');

}

bool JSPluginModuleSpec::validateActionTypeModule()
{
    if (m_moduleType != "Action")
        return false;

    const QString appDirName = m_dir.dirName();
    QVector<QString> slotTypeOptions = {
        "ExecuteScript", "HtmlDialog", "PopupDialog", "MainTabView"
    };
    if (!slotTypeOptions.contains(m_slotType)) {
        qWarning() << QString("%1: Unknown SlotType '%2'")
            .arg(appDirName)
            .arg(m_slotType);
        return false;
    }

    QVector<QString> buttonLocationOptions = {
        "Main", "Document"
    };
    if (!buttonLocationOptions.contains(m_buttonLocation)) {
        qWarning() << QString("%1: Unknown ButtonLocation '%2'")
            .arg(appDirName)
            .arg(m_buttonLocation);
        return false;
    }

    if (m_slotType == "ExecuteScript") {
        if (m_scriptFileName.isEmpty()) {
            qWarning() << QString("%1: ExecuteScript type module should specify ScriptFileName.")
                .arg(appDirName);
            return false;
        }

        if (!m_dir.exists(m_scriptFileName)) {
            qWarning() << QString("%1: The file of ScriptFileName does not exist.")
                .arg(appDirName);
            return false;
        }
    }

    if (slotTypeOptions.mid(0, -1).contains(m_slotType)) {
        if (m_htmlFileName.isEmpty()) {
            qWarning() << QString("%1: Dialog type module should specify HtmlFileName.")
                .arg(appDirName);
            return false;
        }
        
        if (!m_dir.exists(m_htmlFileName)) {
            qWarning() << QString("%1: The file of HtmlFileName does not exist.")
                .arg(appDirName);
            return false;
        }
    }

    return true;
}

bool JSPluginModuleSpec::validate() {
    if (m_moduleType == "Action") {
        return validateActionTypeModule();
    }

    const QString appDirName = m_dir.dirName();
    qWarning() << QString("%1: Unknown module type '%2'")
        .arg(appDirName)
        .arg(m_moduleType);
    return false;
}