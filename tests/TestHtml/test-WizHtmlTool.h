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

    void check_WizHtmlGetContent();
    void check_WizHtmlGetContent_data();

    void check_WizHtmlGetPureText();
    void check_WizHtmlGetPureText_data();

    void check_WizReplaceTagsWithText();
    void check_WizReplaceTagsWithText_data();

    void check_WizHtmlToMarkdown();
    void check_WizHtmlToMarkdown_data();
};

#endif // TEST_WIZHTMLTOOL_H
