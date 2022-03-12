#include "test-WizHtmlTool.h"

#include <QString>
#include <QStringList>

#include "src/html/WizHtmlTool.h"

QTEST_MAIN(TestWizHtmlTool)

static QString wrapTextInHtml5(const QString &text) {
    return QString("<!DOCTYPE html><html><head></head><body>%1</body></html>").arg(text);
}


void TestWizHtmlTool::check_WizHtmlExtractTags()
{
    QFETCH(QString, htmltext);
    QFETCH(QString, tagname);
    QFETCH(QString, attrname);
    QFETCH(QString, attrvalue);
    QFETCH(QStringList, tagtexts);

    QStringList actuals = Utils::WizHtmlExtractTags(htmltext, tagname, attrname, attrvalue);
    QCOMPARE(actuals, tagtexts);
}

void TestWizHtmlTool::check_WizHtmlExtractTags_data()
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
    QTest::newRow("Test no matched tag")
        << wrapTextInHtml5("<div id='first'>Hello <p></div> World</p>") << "div" << "id" << "second"
        << QStringList();

    QTest::newRow("Test no attr")
        << wrapTextInHtml5("<div id='first'>Hello <p></div> World</p>") << "div" << "class" << "second"
        << QStringList();

    QTest::newRow("Test wrong overlaped <div> tag")
        << wrapTextInHtml5("<div id='first'>Hello <p></div> World</p>") << "div" << "id" << "first"
        << QStringList("<div id='first'>Hello <p></div>");

    QTest::newRow("Test wrong overlaped <p> tags")
        << wrapTextInHtml5("<div id='first'>Hello <p id='second'></div> World</p>") << "p" << "id" << "second"
        << QStringList("<p id='second'></div>");
}


void TestWizHtmlTool::check_WizHtmlInsertText()
{
    QFETCH(QString, htmlText);
    QFETCH(QString, tagName);
    QFETCH(QString, attrName);
    QFETCH(QString, attrValue);
    QFETCH(QString, insetText);
    QFETCH(QString, position);
    QFETCH(QString, outputText);

    QString resultText = Utils::WizHtmlInsertText(
        htmlText, insetText, position, tagName, attrName, attrValue);

    QCOMPARE(resultText, outputText);
}


void TestWizHtmlTool::check_WizHtmlInsertText_data()
{
    QTest::addColumn<QString>("htmlText");
    QTest::addColumn<QString>("tagName");
    QTest::addColumn<QString>("attrName");
    QTest::addColumn<QString>("attrValue");
    QTest::addColumn<QString>("insetText");
    QTest::addColumn<QString>("position");
    QTest::addColumn<QString>("outputText");

    // Check insert position
    QTest::newRow("Inset <div> tag beforeend <body>")
        << wrapTextInHtml5("<p></p>") << "body" << "" << "" << "<div id='inserted'>Hello</div>" << "beforeend"
        << wrapTextInHtml5("<p></p><div id='inserted'>Hello</div>");

    QTest::newRow("Inset <div> tag beforebegin <body>")
        << "<html><head></head><body><p></p></body></html>" << "body" << "" << "" << "<div id='inserted'>Hello</div>" << "beforebegin"
        << "<html><head></head><div id='inserted'>Hello</div><body><p></p></body></html>";

    QTest::newRow("Inset <div> tag afterbegin <body>")
        << "<html><head></head><body><p></p></body></html>" << "body" << "" << "" << "<div id='inserted'>Hello</div>" << "afterbegin"
        << "<html><head></head><body><div id='inserted'>Hello</div><p></p></body></html>";

    QTest::newRow("Inset <div> tag afterend <body>")
        << "<html><head></head><body><p></p></body></html>" << "body" << "" << "" << "<div id='inserted'>Hello</div>" << "afterend"
        << "<html><head></head><body><p></p></body><div id='inserted'>Hello</div></html>";

    // Check Robustness
    QTest::newRow("No matched tag")
        << "<body><p id='para'></p></body>" << "p" << "id" << "wrong" << "<div id='inserted'>Hello</div>" << "beforeend"
        << "<body><p id='para'></p></body>";
}


void TestWizHtmlTool::check_WizHtmlGetContent()
{
    QFETCH(QString, htmlText);
    QFETCH(QString, content);

    QString text = Utils::WizHtmlGetContent(htmlText);

    QCOMPARE(text, content);
}


void TestWizHtmlTool::check_WizHtmlGetContent_data()
{
    QTest::addColumn<QString>("htmlText");
    QTest::addColumn<QString>("content");

    QTest::newRow("Whole html document")
        << wrapTextInHtml5("<p id='para'>Hello <span>World</span></p><div>This is WizNotePlus</div>")
        << "Hello World\nThis is WizNotePlus";
}


void TestWizHtmlTool::check_WizHtmlGetPureText()
{
    QFETCH(QString, htmlText);
    QFETCH(QString, content);

    QString text = Utils::WizHtmlGetPureText(htmlText);

    QCOMPARE(text, content);

}


void TestWizHtmlTool::check_WizHtmlGetPureText_data()
{
    QTest::addColumn<QString>("htmlText");
    QTest::addColumn<QString>("content");

    QTest::newRow("Test Whole html document")
        << wrapTextInHtml5("<body><p id='para'>Hello World</p><div>This is WizNotePlus</div></body>")
        << "Hello World\nThis is WizNotePlus";

    QTest::newRow("Test partial html document")
        << "<p id='para'>Hello World</p><div>This is WizNotePlus</div>"
        << "Hello World\nThis is WizNotePlus";

    // One <br/> equals two line break.
    QTest::newRow("Test escape and entities")
        << "<p id='para'>Hello World&nbsp;&nbsp;&nbsp;&nbsp;</p><br/><div>This is WizNotePlus</div>"
        << "Hello World    \n\n\nThis is WizNotePlus";
}


void TestWizHtmlTool::check_WizHtmlExtractAttrValues()
{
    QFETCH(QString, htmlText);
    QFETCH(QString, attribute);
    QFETCH(QStringList, values);

    QStringList ret = Utils::WizHtmlExtractAttrValues(htmlText, attribute);

    QCOMPARE(ret, values);
}

void TestWizHtmlTool::check_WizHtmlExtractAttrValues_data()
{
    QTest::addColumn<QString>("htmlText");
    QTest::addColumn<QString>("attribute");
    QTest::addColumn<QStringList>("values");

    QTest::newRow("Test src of img tag")
        << wrapTextInHtml5("<body><img src='https://hello.png' /><img src='https://word.png' /></body>")
        << "src" << QStringList({"https://hello.png", "https://word.png"});
    QTest::newRow("Test relative src of img tag")
        << wrapTextInHtml5("<body><img src='index_files/hello.png' /><img src='index_files/word.png' /></body>")
        << "src" << QStringList({"index_files/hello.png", "index_files/word.png"});

    QTest::newRow("Test href of a tag")
        << wrapTextInHtml5("<body><a href='https://example.com'>Website</a><a href='https://helloworld.com'>Website</a></body>")
        << "href" << QStringList({"https://example.com", "https://helloworld.com"});
    QTest::newRow("Test href of link tag")
        << "<html><head><link rel=\"File-List\" type=\"image/png\" href=\"index_files/cfwa5sd4js46adsf4.png\" /></head><body><p></p></body></html>"
        << "href" << QStringList({"index_files/cfwa5sd4js46adsf4.png"});
}
