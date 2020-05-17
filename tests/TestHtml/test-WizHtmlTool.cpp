#include "test-WizHtmlTool.h"

#include <QString>
#include <QStringList>

#include "src/html/WizHtmlTool.h"

QTEST_MAIN(TestWizHtmlTool)

static QString wrapTextInHtml5(const QString &text) {
    return QString("<!DOCTYPE html><html><head></head><body>%1</body></html>").arg(text);
}


void TestWizHtmlTool::checkWizHtmlExtractTags()
{
    QFETCH(QString, htmltext);
    QFETCH(QString, tagname);
    QFETCH(QString, attrname);
    QFETCH(QString, attrvalue);
    QFETCH(QStringList, tagtexts);

    QStringList actuals = Utils::WizHtmlExtractTags(htmltext, tagname, attrname, attrvalue);
    QCOMPARE(actuals, tagtexts);
}

void TestWizHtmlTool::checkWizHtmlExtractTags_data()
{
    QTest::addColumn<QString>("htmltext");
    QTest::addColumn<QString>("tagname");
    QTest::addColumn<QString>("attrname");
    QTest::addColumn<QString>("attrvalue");
    QTest::addColumn<QStringList>("tagtexts");

    // Check account of tags.
    QTest::newRow("Extract 0 tag from html")
        << wrapTextInHtml5("") << "div" << "test" << "hello"
        << QStringList();

    QTest::newRow("Extract 1 tag from html")
        << wrapTextInHtml5("<div test='hello'>Hello World</div>") << "div" << "test" << "hello"
        << QStringList("<div test='hello'>Hello World</div>");

    QTest::newRow("Extract 3 tag from html")
        << wrapTextInHtml5(QString("<div test='hello'>Hello World</div>").repeated(3)) << "div" << "test" << "hello"
        << (QStringList("<div test='hello'>Hello World</div>") << "<div test='hello'>Hello World</div>"
                        << "<div test='hello'>Hello World</div>");

    //  Check encoding
    QTest::newRow("Chinese content or tag attr value")
        << wrapTextInHtml5("<div test='你好'>你好世界</div>") << "div" << "test" << "你好"
        << QStringList("<div test='你好'>你好世界</div>");

    // Check format of output html
    QTest::newRow("Trim space in html tag brackets")
        << wrapTextInHtml5("<div test='hello'>Hello World</div>") << "div" << "test" << "hello"
        << QStringList("<div test='hello'>Hello World</div>");

    QTest::newRow("Test boolean attribute")
        << wrapTextInHtml5("<div checked>Hello World</div>") << "div" << "checked" << ""
        << QStringList("<div checked>Hello World</div>");

    QTest::newRow("Attribute value without quotes")
        << wrapTextInHtml5("<div test=hello>Hello World</div>") << "div" << "test" << "hello"
        << QStringList("<div test=hello>Hello World</div>");

    // Check escape
    QTest::newRow("Test escape in tag")
        << wrapTextInHtml5("<div><span id=\"that's\">1\n</span>2\n</div>") << "span" << "id" << "that's"
        << QStringList("<span id=\"that's\">1\n</span>");

    QTest::newRow("Test HTML entities")
        << wrapTextInHtml5("<a class=\"special\">&nbsp;&nbsp;&nbsp;&nbsp;some link</a>") << "a" << "class" << "special"
        << QStringList("<a class=\"special\">&nbsp;&nbsp;&nbsp;&nbsp;some link</a>");


    // Check Robustness
    // FIXME: this is not the desired results.
    QTest::newRow("Test wrong overlaped <div> tag")
        << wrapTextInHtml5("<div id='first'>Hello <p></div> World</p>") << "div" << "id" << "first"
        << QStringList("<div id='first'>Hello <p></div>");

    QTest::newRow("Test wrong overlaped <p> tags")
        << wrapTextInHtml5("<div id='first'>Hello <p id='second'></div> World</p>") << "p" << "id" << "second"
        << QStringList({"<p id='second'></div>", "</p>"});
}