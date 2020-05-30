#include "JSPlugin.h"

#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

#include "share/WizSettings.h"


JSPlugin::JSPlugin(QString &pluginFolder, QObject *parent)
    : QObject(parent)
{
    m_data = new JSPluginSpec(QDir(pluginFolder).absoluteFilePath("manifest.ini"), this);
    for (const auto &moduleSpec : m_data->modules()) {
        m_modules.append(new JSPluginModule(moduleSpec, this));
    }
}


bool JSPlugin::isAvailable()
{
    //TODO: check common information.
    return m_data->validate()
        && std::all_of(m_modules.begin(), m_modules.end(), [=](JSPluginModule *module) {
        return module->isAvailable();
    });
}


void JSPlugin::emitDocumentChanged()
{
    for (auto module : m_modules) {
        module->emitDocumentChanged();
    }
}


void JSPlugin::emitShowEvent()
{
    for (auto module : m_modules) {
        module->emitShowEvent();
    }
}


struct LOCAL_LCID_DATA
{
    const char* name;
    int lcid;
};

static const LOCAL_LCID_DATA g_lcid[] =
{
#include "local_lcid_data.inc"
    {nullptr, 0},
};

static int localNameToLcid(QString name) {
    for (const LOCAL_LCID_DATA* pData = g_lcid; ;pData++) {
        if (!pData->name)
            break;
        //
        if (0 == name.compare(pData->name, Qt::CaseInsensitive)) {
            return pData->lcid;
        }
    }
    return 0;
}

static void splitStringKey(QString key, QString& name, int& lcid)
{
    int index = key.lastIndexOf("_");
    if (index != -1) {
        QString testName = key.left(index);
        QString lcidStr = key.mid(index + 1);
        int testLcid = lcidStr.toInt();
        if (WizIntToStr(testLcid) == lcidStr) {  //is a number
            name = testName;
            lcid = testLcid;
            return;
        }
    }
    //
    name = key;
    lcid = 0;
    return;
}


void JSPlugin::initStrings()
{
    QString path = m_data->path();
    WizPathAddBackslash(path);
    QString fileName = path + "manifest.ini";

    WizSettings plugin(fileName);
    CWizStdStringArray keys;
    plugin.getKeys("Strings", keys);

    QJsonObject object;
    QString localeName;
    if (WizUserSettings* settings = WizUserSettings::currentSettings()) {
        localeName = settings->locale();
    }
    int lcid = localNameToLcid(localeName);
    QString suffix = lcid ? QString("_") + WizIntToStr(lcid) : QString("");
    for (auto key : keys) {
        QString name;
        int l = 0;
        splitStringKey(key, name, l);
        if (l == 0 || l == lcid) {
            QString value = plugin.getString("Strings", key);
            object[name] = value;
        }
    }
    QJsonDocument doc(object);
    m_strings = QString(doc.toJson());
}


JSPluginModule::JSPluginModule(JSPluginModuleSpec *moduleSpec, QObject *parent)
    : QObject(parent)
{
    m_parentPlugin = qobject_cast<JSPlugin*>(parent);
    if (!m_parentPlugin) {
        qWarning() << "Error";
    }
    m_data = moduleSpec;
}


void JSPluginModule::emitDocumentChanged()
{
    emit documentChanged();
}


void JSPluginModule::emitShowEvent()
{
    emit willShow();
}


bool JSPluginModule::isAvailable()
{
    // Check files exists
    return m_data->validate();
}