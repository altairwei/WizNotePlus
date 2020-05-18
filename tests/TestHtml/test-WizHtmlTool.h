#ifndef TEST_WIZHTMLTOOL_H
#define TEST_WIZHTMLTOOL_H

#include <QtTest/QtTest>
#include <QObject>

class TestWizHtmlTool : public QObject
{

    Q_OBJECT

private slots:
    void check_WizHtmlExtractTags();
    void check_WizHtmlExtractTags_data();

    void check_WizHtmlInsertText();
    void check_WizHtmlInsertText_data();
};

#endif // TEST_WIZHTMLTOOL_H