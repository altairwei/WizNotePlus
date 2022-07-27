#include "JSPluginSpec.h"

#include <algorithm>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QVector>
#include <QVersionNumber>

#include "WizDef.h"

#define MANIFESTVERSION 2


JSPluginSpec::JSPluginSpec(const QString &manifestFileName, QObject* parent)
    : QObject(parent)
{
    QFileInfo manifestFile(manifestFileName);
    m_dir = manifestFile.absoluteDir();
    m_path = m_dir.absolutePath() + "/";

    m_settings = new QSettings(manifestFile.absoluteFilePath(), QSettings::IniFormat, this);
    m_settings->setIniCodec("UTF-8");

    // Parse 'Common' setiong
    m_name = m_settings->value("Common/AppName", "").toString();
    m_type = m_settings->value("Common/AppType", "").toString();
    m_guid = m_settings->value("Common/AppGUID", "").toString();
    m_moduleCount = m_settings->value("Common/ModuleCount", 0).toInt();
    m_manifestVersion = m_settings->value("Common/ManifestVersion", 0).toInt();
    m_apiMinimumRequired = m_settings->value("Common/ApiMinimumRequired", "").toString();

    // Parse Modules
    m_realModuleCount = 0;
    QStringList groups = m_settings->childGroups();
    for (QString& pluginIndex : groups) {
        if (!pluginIndex.contains("Module_"))
            continue;
        JSPluginModuleSpec *module = new JSPluginModuleSpec(pluginIndex, m_settings, this);
        m_modules.append(module);
        m_realModuleCount++;
    }
    if (m_realModuleCount != m_moduleCount)
        warn(QString("ModuleCount of '%1' not correct!").arg(m_name));
}

void JSPluginSpec::warn(const QString &msg)
{
    const QString appDirName = m_dir.dirName();
    qWarning() << QString("%1: %2")
        .arg(appDirName)
        .arg(msg);
}

bool JSPluginSpec::validate() {
    if (m_manifestVersion != MANIFESTVERSION) {
        warn(QString("Plugin is too old! ManifestVersion is not equal to %1").arg(MANIFESTVERSION));
        return false;
    }

    if (m_name.isEmpty() || m_guid.isEmpty()) {
        warn("AppName or AppGUID is empty.");
        return false;
    }

    if (m_moduleCount != m_realModuleCount) {
        warn("ModuleCount isn't equal to real count.");
        return false;
    }

    if (!m_apiMinimumRequired.isEmpty()) {
        QVersionNumber current = QVersionNumber::fromString(WIZ_CLIENT_VERSION);
        QVersionNumber required = QVersionNumber::fromString(m_apiMinimumRequired);
        if (current < required) {
            warn(QString(
                "Minimum WizNotePlus version required is '%1', but current version is '%2'")
                    .arg(required.toString(), current.toString()));
            return false;
        }
    }

    return true;
}


JSPluginModuleSpec::JSPluginModuleSpec(QString& section, QSettings *setting, QObject* parent)
    : QObject(parent)
    , m_section(section)
{
    m_parentSpec = qobject_cast<JSPluginSpec*>(parent);
    m_path = m_parentSpec->path();
    m_dir = m_parentSpec->m_dir;

    // Module common information
    m_caption = setting->value(section + "/Caption", "").toString();
    m_guid = setting->value(section + "/GUID", "").toString();
    m_moduleType = setting->value(section + "/ModuleType", "").toString();
    m_iconFileName = getFilePath(setting, section + "/IconFileName");

    // Action type module
    m_slotType = setting->value(section + "/SlotType", "").toString();
    m_buttonLocation = setting->value(section + "/ButtonLocation", "").toString();
    m_menuLocation = setting->value(section + "/MenuLocation", "").toString();
    m_htmlFileName = getFilePath(setting, section + "/HtmlFileName");
    m_scriptFileName = getFilePath(setting, section + "/ScriptFileName");
    m_width = setting->value(section + "/Width", 800).toInt();
    m_height = setting->value(section + "/Height", 500).toInt();

    // Editor type module
    m_scriptFiles = getFileList(setting, section + "/ScriptFiles");
    m_styleFiles = getFileList(setting, section + "/StyleFiles");
    m_supportedFormats = setting->value(section + "/SupportedFormats").toStringList();

    // Menu type module
    auto indexes = setting->value(section + "/Actions").toStringList();
    indexes.removeDuplicates();
    foreach (const QString &ind, indexes) {
        m_actionIndexes.append(ind.toInt());
    }
    m_sidebarLocation = setting->value(section + "/SidebarLocation", "Right").toString();
}

QStringList JSPluginModuleSpec::getFileList(QSettings *setting, const QString &key)
{
    // Value with comma will be treated as string list.
    QStringList fileList = setting->value(key).toStringList();
    if (fileList.isEmpty()) {
        return QStringList();
    } else {
        for (QString &filePath : fileList) {
            filePath.prepend(m_path);
        }
        return fileList;
    }
}

QString JSPluginModuleSpec::getFilePath(QSettings *setting, const QString &key)
{
    QString value = setting->value(key, "").toString();
    if (value.isEmpty()) {
        return QString();
    } else {
        return m_path + value;
    }
}

void JSPluginModuleSpec::warn(const QString &msg)
{
    const QString appDirName = m_dir.dirName();
    qWarning() << QString("%1: %2")
        .arg(appDirName)
        .arg(msg);
}

bool JSPluginModuleSpec::validateActionTypeModule()
{
    if (m_moduleType != "Action")
        return false;

    QVector<QString> slotTypeOptions = {
        "ExecuteScript", "HtmlDialog",
        "SelectorWindow", "MainTabView",
        "DocumentSidebar"
    };
    if (!slotTypeOptions.contains(m_slotType)) {
        warn(QString("Unknown SlotType '%1'").arg(m_slotType));
        return false;
    }

    QVector<QString> buttonLocationOptions = {
        "Main", "Document", "Menu"
    };
    if (!buttonLocationOptions.contains(m_buttonLocation)) {
        warn(QString("Unknown ButtonLocation '%1'").arg(m_buttonLocation));
        return false;
    }

    if (m_slotType == "ExecuteScript") {
        if (m_scriptFileName.isEmpty()) {
            warn("ExecuteScript type module should specify ScriptFileName.");
            return false;
        }

        if (!m_dir.exists(m_scriptFileName)) {
            warn("The file of ScriptFileName does not exist.");
            return false;
        }
    }

    if (slotTypeOptions.mid(1, -1).contains(m_slotType)) {
        if (m_htmlFileName.isEmpty()) {
            warn("Dialog type module should specify HtmlFileName.");
            return false;
        }
        
        if (!m_dir.exists(m_htmlFileName)) {
            warn("The file of HtmlFileName does not exist.");
            return false;
        }
    }

    QStringList sidebars = {"Left", "Right"};
    if (!sidebars.contains(m_sidebarLocation)) {
        warn("SidebarLocation must be: 'Right' or 'Left'.");
        return false;
    }

    return true;
}

bool JSPluginModuleSpec::validateEditorTypeModule()
{
    if (m_moduleType != "Editor")
        return false;

    bool isAllScriptValid = std::all_of(m_scriptFiles.begin(), m_scriptFiles.end(),
        [this](const QString &scriptFile) {
            QFileInfo script(scriptFile);
            if (!script.exists()) {
                warn(QString("Script file '%1' does not exist.").arg(scriptFile));
                return false;
            } else {
                return true;
            }
        });

    bool isAllStyleValid = std::all_of(m_styleFiles.begin(), m_styleFiles.end(),
        [this](const QString &styleFile){
            QFileInfo style(styleFile);
            if (!style.exists()) {
                warn(QString("Style file '%1' does not exist.").arg(styleFile));
                return false;
            } else {
                return false;
            }
        });

    return isAllScriptValid && isAllScriptValid;
}

bool JSPluginModuleSpec::validateMenuTypeModule()
{
    QVector<QString> buttonLocationOptions = {
        "Main", "Document"
    };
    if (!buttonLocationOptions.contains(m_buttonLocation)) {
        warn(QString("Unknown ButtonLocation '%1'").arg(m_buttonLocation));
        return false;
    }

    if (m_actionIndexes.isEmpty()) {
        warn("Empty menu actions");
        return false;
    }

    JSPluginSpec *spec = parentSpec();
    bool isAllActionValid = std::all_of(m_actionIndexes.begin(), m_actionIndexes.end(),
        [spec, this](const int &index) {
            if (index > spec->moduleCount()) {
                warn("menu action index out of range");
                return false;
            }
            JSPluginModuleSpec *ms = spec->module(index);
            if (ms->moduleType() != "Action") {
                warn("menu module must be action type");
                return false;
            }
            if (ms->buttonLocation() != "Menu") {
                warn("menu module must be located at Menu");
                return false;
            }
            return true;
        });

    return isAllActionValid;
}

bool JSPluginModuleSpec::validate()
{
    if (m_caption.isEmpty()) {
        warn("Empty caption.");
        return false;
    }

    if (m_guid.isEmpty()) {
        warn("Empty GUID.");
        return false;
    }

    if (m_moduleType.isEmpty()) {
        warn("Empty module type.");
        return false;
    }

    if (m_moduleType == "Action") {
        return validateActionTypeModule();
    } else if (m_moduleType == "Editor") {
        return validateEditorTypeModule();
    } else if (m_moduleType == "Menu") {
        return validateMenuTypeModule();
    }

    warn(QString("Unknown module type '%2'").arg(m_moduleType));
    return false;
}
