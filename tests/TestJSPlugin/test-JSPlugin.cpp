#include "test-JSPlugin.h"

#include <QDebug>

#include "src/jsplugin/JSPluginSpec.h"


QTEST_MAIN(TestJSPlugin)


void TestJSPlugin::check_JSPluginSpec()
{
    QFETCH(QString, iniFileName);
    QFETCH(QString, AppName);
    QFETCH(QString, AppType);
    QFETCH(QString, AppGUID);
    QFETCH(int, ModuleCount);
    QFETCH(int, ManifestVersion);
    QFETCH(bool, isValid);

    JSPluginSpec plugin(iniFileName, this);

    QCOMPARE(plugin.name(), AppName);
    QCOMPARE(plugin.type(), AppType);
    QCOMPARE(plugin.guid(), AppGUID);
    QCOMPARE(plugin.moduleCount(), ModuleCount);
    QCOMPARE(plugin.manifestVersion(), ManifestVersion);
    QCOMPARE(plugin.validate(), isValid);
}


void TestJSPlugin::check_JSPluginSpec_data()
{
    QTest::addColumn<QString>("iniFileName");
    QTest::addColumn<QString>("AppName");
    QTest::addColumn<QString>("AppType");
    QTest::addColumn<QString>("AppGUID");
    QTest::addColumn<int>("ModuleCount");
    QTest::addColumn<int>("ManifestVersion");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("Parse common plugin information") << QFINDTESTDATA("manifest_files/plugin_eg_01.ini")
        << "HelloWorldApp" << "Tools" << "{edb64fbd-2255-408f-b690-f61e56cb9606}" << 4 << 2 << true;

    QTest::newRow("Parse manifest without any module") << QFINDTESTDATA("manifest_files/plugin_eg_02.ini")
        << "HelloWorldApp" << "Tools" << "{edb64fbd-2255-408f-b690-f61e56cb9606}" << 0 << 2 << true;

    QTest::newRow("Manifest file without GUID") << QFINDTESTDATA("manifest_files/plugin_eg_03.ini")
        << "HelloWorldApp" << "Tools" << "" << 2 << 1 << false;
}


void TestJSPlugin::check_JSPluginModuleSpec()
{
    QFETCH(QString, iniFileName);
    QFETCH(int, indexOfModule);

    QFETCH(QString, ModuleType);
    QFETCH(QString, Caption);
    QFETCH(QString, GUID);
    QFETCH(QString, ButtonLocation);
    QFETCH(QString, SlotType);
    QFETCH(QString, HtmlFileName);
    QFETCH(QString, ScriptFileName);
    QFETCH(QStringList, ScriptFiles);
    QFETCH(QStringList, StyleFiles);
    QFETCH(QStringList, SupportedFormats);
    QFETCH(bool, isValid);

    JSPluginSpec plugin(iniFileName, this);
    JSPluginModuleSpec *module = plugin.module(indexOfModule);

    QCOMPARE(module->moduleType(), ModuleType);
    QCOMPARE(module->caption(), Caption);
    QCOMPARE(module->guid(), GUID);
    QCOMPARE(module->buttonLocation(), ButtonLocation);
    QCOMPARE(module->slotType(), SlotType);
    QCOMPARE(module->htmlFileName(), HtmlFileName);
    QCOMPARE(module->scriptFileName(), ScriptFileName);
    QCOMPARE(module->scriptFiles(), ScriptFiles);
    QCOMPARE(module->styleFiles(), StyleFiles);
    QCOMPARE(module->supportedFormats(), SupportedFormats);

    QCOMPARE(module->validate(), isValid);
}


void TestJSPlugin::check_JSPluginModuleSpec_data()
{
    QTest::addColumn<QString>("iniFileName");
    QTest::addColumn<int>("indexOfModule");

    QTest::addColumn<QString>("ModuleType");
    QTest::addColumn<QString>("Caption");
    QTest::addColumn<QString>("GUID");

    QTest::addColumn<QString>("ButtonLocation");
    QTest::addColumn<QString>("SlotType");
    QTest::addColumn<QString>("HtmlFileName");
    QTest::addColumn<QString>("ScriptFileName");

    QTest::addColumn<QStringList>("ScriptFiles");
    QTest::addColumn<QStringList>("StyleFiles");
    QTest::addColumn<QStringList>("SupportedFormats");

    QTest::addColumn<bool>("isValid");

    // Legal formats
    QTest::newRow("ModuleType = Action, SlotType = MainTabView")
        << QFINDTESTDATA("manifest_files/plugin_eg_01.ini") << 0
        << "Action" << "Helle World (MainTabView)" << "{5727DF40-7BA3-4E78-88EE-DCCDB107B811}"
        << "Main" << "MainTabView" << QFINDTESTDATA("manifest_files/plugin_eg_02.ini") << ""
        << QStringList() << QStringList() << QStringList()
        << true;

    QTest::newRow("ModuleType = Action, SlotType = ExecuteScript")
        << QFINDTESTDATA("manifest_files/plugin_eg_01.ini") << 1
        << "Action" << "Helle World (ExecuteScript)" << "{71583fff-92a5-4308-a21b-731bd52c6474}"
        << "Document" << "ExecuteScript" << "" << QFINDTESTDATA("manifest_files/plugin_eg_02.ini") 
        << QStringList() << QStringList() << QStringList()
        << true;

    QTest::newRow("ModuleType = Action, SlotType = HtmlDialog")
        << QFINDTESTDATA("manifest_files/plugin_eg_01.ini") << 2
        << "Action" << "Helle World (HtmlDialog)" << "{a92d2f09-96e9-433e-ac74-ba4befdaa80d}"
        << "Main" << "HtmlDialog" << QFINDTESTDATA("manifest_files/plugin_eg_02.ini") << ""
        << QStringList() << QStringList() << QStringList()
        << true;

    QTest::newRow("ModuleType = Editor")
        << QFINDTESTDATA("manifest_files/plugin_eg_01.ini") << 3
        << "Editor" << "Helle World (Editor)" << "{4014eae9-534e-4369-bb79-9fbb907d98a8}"
        << "" << "" << "" << ""
        << QStringList({QFINDTESTDATA("manifest_files/plugin_eg_01.ini"), QFINDTESTDATA("manifest_files/plugin_eg_02.ini")}) 
        << QStringList({QFINDTESTDATA("manifest_files/plugin_eg_02.ini"), QFINDTESTDATA("manifest_files/plugin_eg_03.ini")})
        << QStringList({"Markdown", "HTML"})
        << true;

    // Illegal format
    QTest::newRow("ModuleType = Action, SlotType = SelectorWindow, illegal")
        << QFINDTESTDATA("manifest_files/plugin_eg_03.ini") << 0
        << "Action" << "Hello World (SelectorWindow)" << "{a45d3846-4d3d-4ceb-849c-23a5980c0318}"
        << "Document" << "SelectorWindow" << "" << ""
        << QStringList() << QStringList() << QStringList()
        << false;

    QTest::newRow("ModuleType = Editor, illegal")
        << QFINDTESTDATA("manifest_files/plugin_eg_03.ini") << 1
        << "Editor" << "Helle World (Editor)" << "{706cb70c-89e4-4294-af8a-a5c136edfece}"
        << "" << "" << "" << ""
        << QStringList({QFINDTESTDATA("manifest_files") + "/index.js"})
        << QStringList({QFINDTESTDATA("manifest_files") + "/index.css", QFINDTESTDATA("manifest_files") + "/hello.css"})
        << QStringList({"Markdown", "HTML"})
        << false;
}