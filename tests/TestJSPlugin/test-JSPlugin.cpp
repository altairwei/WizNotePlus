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
    QFETCH(bool, isValid);

    JSPluginSpec plugin(iniFileName, this);

    QCOMPARE(plugin.name(), AppName);
    QCOMPARE(plugin.type(), AppType);
    QCOMPARE(plugin.guid(), AppGUID);
    QCOMPARE(plugin.moduleCount(), ModuleCount);
    QCOMPARE(plugin.validate(), isValid);
}


void TestJSPlugin::check_JSPluginSpec_data()
{
    QTest::addColumn<QString>("iniFileName");
    QTest::addColumn<QString>("AppName");
    QTest::addColumn<QString>("AppType");
    QTest::addColumn<QString>("AppGUID");
    QTest::addColumn<int>("ModuleCount");
    QTest::addColumn<bool>("isValid");

    QTest::newRow("Parse common plugin information") << QFINDTESTDATA("manifest_files/plugin_eg_01.ini")
        << "HelloWorldApp" << "Tools" << "{edb64fbd-2255-408f-b690-f61e56cb9606}" << 3 << true;

    QTest::newRow("Parse manifest without any module") << QFINDTESTDATA("manifest_files/plugin_eg_02.ini")
        << "HelloWorldApp" << "Tools" << "{edb64fbd-2255-408f-b690-f61e56cb9606}" << 0 << true;

    QTest::newRow("Manifest file without GUID") << QFINDTESTDATA("manifest_files/plugin_eg_03.ini")
        << "HelloWorldApp" << "Tools" << "" << 1 << false;
}