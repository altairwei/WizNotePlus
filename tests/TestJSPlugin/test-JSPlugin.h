#ifndef TEST_JSPLUGIN_H
#define TEST_JSPLUGIN_H

#include <QtTest/QtTest>
#include <QObject>

class TestJSPlugin : public QObject
{

    Q_OBJECT

private slots:
    void check_JSPluginSpec();
    void check_JSPluginSpec_data();
};

#endif // TEST_JSPLUGIN_H