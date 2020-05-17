#ifndef TEST_WIZHTMLTOOL_H
#define TEST_WIZHTMLTOOL_H

#include <QtTest/QtTest>
#include <QObject>

class TestWizHtmlTool : public QObject
{

    Q_OBJECT

private slots:
    void checkWizHtmlExtractTags();
    void checkWizHtmlExtractTags_data();
};

#endif // TEST_WIZHTMLTOOL_H