#ifndef TEST_JSPLUGIN_H
#define TEST_JSPLUGIN_H

#include <QtTest/QTest>
#include <QObject>

class TestJSPlugin : public QObject
{

    Q_OBJECT

private slots:
    void check_JSPluginSpec();
    void check_JSPluginSpec_data();

    void check_JSPluginModuleSpec();
    void check_JSPluginModuleSpec_data();
};

#endif // TEST_JSPLUGIN_H