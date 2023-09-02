#include "test-WizHtmlTool.h"

#include <QString>
#include <QStringList>

#include "src/html/WizHtmlTool.h"

QTEST_MAIN(TestWizHtmlTool)

static QString wrapHTML(const QString &text) {
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
        << wrapHTML("") << "div" << "test" << "hello"
        << QStringList();

    QTest::newRow("Extract 1 tag from html")
        << wrapHTML("<div test='hello'>Hello World</div>") << "div" << "test" << "hello"
        << QStringList("<div test='hello'>Hello World</div>");

    QTest::newRow("Extract 3 tag from html")
        << wrapHTML(QString("<div test='hello'>Hello World</div>").repeated(3)) << "div" << "test" << "hello"
        << (QStringList("<div test='hello'>Hello World</div>") << "<div test='hello'>Hello World</div>"
                        << "<div test='hello'>Hello World</div>");

    //  Check encoding
    QTest::newRow("Chinese content or tag attr value")
        << wrapHTML("<div test='你好'>你好世界</div>") << "div" << "test" << "你好"
        << QStringList("<div test='你好'>你好世界</div>");

    // Check format of output html
    QTest::newRow("Trim space in html tag brackets")
        << wrapHTML("<div test='hello'>Hello World</div>") << "div" << "test" << "hello"
        << QStringList("<div test='hello'>Hello World</div>");

    QTest::newRow("Test boolean attribute")
        << wrapHTML("<div checked>Hello World</div>") << "div" << "checked" << ""
        << QStringList("<div checked>Hello World</div>");

    QTest::newRow("Attribute value without quotes")
        << wrapHTML("<div test=hello>Hello World</div>") << "div" << "test" << "hello"
        << QStringList("<div test=hello>Hello World</div>");

    // Check escape
    QTest::newRow("Test escape in tag")
        << wrapHTML("<div><span id=\"that's\">1\n</span>2\n</div>") << "span" << "id" << "that's"
        << QStringList("<span id=\"that's\">1\n</span>");

    QTest::newRow("Test HTML entities")
        << wrapHTML("<a class=\"special\">&nbsp;&nbsp;&nbsp;&nbsp;some link</a>") << "a" << "class" << "special"
        << QStringList("<a class=\"special\">&nbsp;&nbsp;&nbsp;&nbsp;some link</a>");


    // Check Robustness
    // FIXME: this is not the desired results.
    QTest::newRow("Test no matched tag")
        << wrapHTML("<div id='first'>Hello <p></div> World</p>") << "div" << "id" << "second"
        << QStringList();

    QTest::newRow("Test no attr")
        << wrapHTML("<div id='first'>Hello <p></div> World</p>") << "div" << "class" << "second"
        << QStringList();

    QTest::newRow("Test wrong overlaped <div> tag")
        << wrapHTML("<div id='first'>Hello <p></div> World</p>") << "div" << "id" << "first"
        << QStringList("<div id='first'>Hello <p></div>");

    QTest::newRow("Test wrong overlaped <p> tags")
        << wrapHTML("<div id='first'>Hello <p id='second'></div> World</p>") << "p" << "id" << "second"
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
        << wrapHTML("<p></p>") << "body" << "" << "" << "<div id='inserted'>Hello</div>" << "beforeend"
        << wrapHTML("<p></p><div id='inserted'>Hello</div>");

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
        << wrapHTML("<p id='para'>Hello <span>World</span></p><div>This is WizNotePlus</div>")
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
        << wrapHTML("<body><p id='para'>Hello World</p><div>This is WizNotePlus</div></body>")
        << "Hello World\nThis is WizNotePlus";

    QTest::newRow("Test partial html document")
        << "<p id='para'>Hello World</p><div>This is WizNotePlus</div>"
        << "Hello World\nThis is WizNotePlus";

    // One <br/> equals two line break.
    QTest::newRow("Test escape and entities")
        << "<p id='para'>Hello World&nbsp;&nbsp;&nbsp;&nbsp;</p><br/><div>This is WizNotePlus</div>"
        << "Hello World    \n\n\nThis is WizNotePlus";
}

void TestWizHtmlTool::check_WizReplaceTagsWithText()
{
    QFETCH(QString, htmlText);
    QFETCH(QString, tagName);
    QFETCH(QString, tagAttributeName);
    QFETCH(QString, tagAttributeValue);
    QFETCH(QString, result);

    auto callback = [](
            const QMap<QString, QString>& attrs,
            const QString &) -> QString {
        return QString("Found %1 attribute(s)").arg(attrs.count());
    };

    QString output = Utils::WizReplaceTagsWithText(
        htmlText, callback, tagName,
        tagAttributeName, tagAttributeValue);

    QCOMPARE(output, result);
}

void TestWizHtmlTool::check_WizReplaceTagsWithText_data()
{
    QTest::addColumn<QString>("tagName");
    QTest::addColumn<QString>("tagAttributeName");
    QTest::addColumn<QString>("tagAttributeValue");
    QTest::addColumn<QString>("htmlText");
    QTest::addColumn<QString>("result");

    QTest::newRow("Simple tag replacement")
        << "strong" << "" << ""
        << "<p>Hello, <strong>world</strong>!</p>"
        << "<p>Hello, Found 0 attribute(s)!</p>";
    QTest::newRow("Simple tag replacement without whitespace")
        << "strong" << "" << ""
        << "<p>Hello, all<strong>world</strong>!all</p>"
        << "<p>Hello, allFound 0 attribute(s)!all</p>";
    QTest::newRow("Multiple tag replacement")
        << "span" << "class" << "highlight"
        << "<p><span>One</span> <span class=\"highlight\">Two</span> <span>Three</span></p>"
        << "<p><span>One</span> Found 1 attribute(s) <span>Three</span></p>";

    QTest::newRow("Replace img")
        << "img" << "alt" << "hello world"
        << "<h1>Heading level 1</h1><img src='https://github.com' alt='hello world'>"
        << "<h1>Heading level 1</h1>Found 2 attribute(s)";
    QTest::newRow("Replace img at starting")
        << "img" << "alt" << "hello world"
        << "<img src='https://github.com' alt='hello world'>"
           "<h1>Heading level 1</h1>"
        << "Found 2 attribute(s)"
           "<h1>Heading level 1</h1>";
    QTest::newRow("Replace multiple img")
        << "img" << "alt" << "hello world"
        << "<h1>Heading level 1</h1>"
           "<img src='https://github.com' alt='hello world'>"
           "<img alt='hello world'>"
        << "<h1>Heading level 1</h1>"
           "Found 2 attribute(s)"
           "Found 1 attribute(s)";
    QTest::newRow("Replace multiple img with special location")
        << "img" << "alt" << "hello world"
        << "<img alt='hello world'>"
           "<h1>Heading level 1</h1>"
           "<img src='https://github.com' alt='hello world'>"
        << "Found 1 attribute(s)"
           "<h1>Heading level 1</h1>"
           "Found 2 attribute(s)";

    QTest::newRow("Replace heading")
        << "h1" << "id" << "head1"
        << "<h1 id='head1'>Heading level 1</h1>~~~"
           "<img src='https://github.com' alt='hello world'>"
        << "Found 1 attribute(s)~~~"
           "<img src='https://github.com' alt='hello world'>";
    QTest::newRow("Replace multiple headings")
        << "h1" << "id" << "head1"
        << "<h1 id='head1'>Heading level 1</h1>"
           "<img src='https://github.com' alt='hello world'>"
           "<h1 id='head1' class='goodhead'>Heading level 1</h1>"
        << "Found 1 attribute(s)"
           "<img src='https://github.com' alt='hello world'>"
           "Found 2 attribute(s)";

    QTest::newRow("Empty HTML text")
        << "div" << "" << ""
        << ""
        << "";
    QTest::newRow("HTML text with no tags")
        << "div" << "" << ""
        << "This is some text without any tags."
        << "This is some text without any tags.";
    QTest::newRow("HTML text with nested tags")
        << "em" << "" << ""
        << "<p><strong><em>Bold and italic</em></strong> text</p>"
        << "<p><strong>Found 0 attribute(s)</strong> text</p>";
    QTest::newRow("HTML text with self-closing tags")
        << "img" << "" << ""
        << "<img src=\"example.png\" alt=\"Example Image\"/>"
        << "Found 2 attribute(s)";
    QTest::newRow("HTML text with attributes to match")
        << "div" << "data-id" << "123"
        << "<div class=\"my-class\" data-id=\"123\">Some text</div>"
        << "Found 2 attribute(s)";

    QTest::newRow("HTML text with nested tags in different order")
        << "p" << "" << ""
        << "<div><p>First <em>nested</em> tag</p><p>Second <strong>nested</strong> tag</p></div>"
        << "<div>Found 0 attribute(s)Found 0 attribute(s)</div>";
    QTest::newRow("HTML text with multiple tags with the same attribute value")
        << "div" << "class" << "my-class"
        << "<div class=\"my-class\">First</div><p class=\"my-class\">Second</p><div class=\"my-class\">Third</div>"
        << "Found 1 attribute(s)<p class=\"my-class\">Second</p>Found 1 attribute(s)";

    // Practical cases
    QTest::newRow("Image within div")
        << "img" << "" << ""
        << R"(
            <body class="wiz-editor-body " data-wiz-document-type="common" spellcheck="false">
                <p># Hello World</p>
                <p><br></p>
                <div class="wiz-image-container"><img border="0" src="index_files/a2df657a-e108-4b3a-82c4-ad0428080f85.png">
                    <div class="wiz-image-title">This is a title</div>
                </div>
            </body>)"
        << R"(
            <body class="wiz-editor-body " data-wiz-document-type="common" spellcheck="false">
                <p># Hello World</p>
                <p><br></p>
                <div class="wiz-image-container">Found 2 attribute(s)
                    <div class="wiz-image-title">This is a title</div>
                </div>
            </body>)";

    /*!
        Offset of gumbo-parser is based on bytes, not characters. This file
        should be saved with UTF-8 with BOM, otherwise following tests will
        fail on Windows.
    */
    QTest::newRow("Even Chinese characters")
        << "strong" << "" << ""
        << "<p>你好, <strong>世界</strong>! 你好吗？</p>"
        << "<p>你好, Found 0 attribute(s)! 你好吗？</p>";
    QTest::newRow("Odd Chinese characters")
        << "strong" << "" << ""
        << "<p>你好, 世界! 你 <strong>好</strong>吗？</p>"
        << "<p>你好, 世界! 你 Found 0 attribute(s)吗？</p>";
    QTest::newRow("Odd Chinese characters without whitespace")
        << "strong" << "" << ""
        << "<p>你好, 世界! 你<strong>好</strong>吗？</p>"
        << "<p>你好, 世界! 你Found 0 attribute(s)吗？</p>";
    QTest::newRow("Even Chinese characters inside replacement")
        << "span" << "class" << "highlight"
        << "<p><span>One</span> <span class=\"highlight\">二两</span> <span>Three</span></p>"
        << "<p><span>One</span> Found 1 attribute(s) <span>Three</span></p>";
    QTest::newRow("Odd Chinese characters inside replacement")
        << "span" << "class" << "highlight"
        << "<p><span>One</span> <span class=\"highlight\">二</span>查 <span>Three</span></p>"
        << "<p><span>One</span> Found 1 attribute(s)查 <span>Three</span></p>";
}

void TestWizHtmlTool::check_WizHtmlToMarkdown()
{
    QFETCH(QString, htmlText);
    QFETCH(QString, mdText);

    QString text = Utils::WizHtmlToMarkdown(htmlText);

    QCOMPARE(text, mdText);
}

void TestWizHtmlTool::check_WizHtmlToMarkdown_data()
{
    QTest::addColumn<QString>("htmlText");
    QTest::addColumn<QString>("mdText");

    // Test Empty String
    QTest::newRow("Test Empty String") << "" << "";
    QTest::newRow("Test Plain String") << "Hello World!" << "Hello World!";
    // TODO: add more tests
    QTest::newRow("Strip whitespace")
        << wrapHTML("\n\t   \n<p>Hello World!</p>")
        << "Hello World!\n";

    // Test Headings
    QTest::newRow("Test H1 heading")
        << wrapHTML("<h1>Heading level 1</h1>") << "# Heading level 1\n";
    QTest::newRow("Test H2 heading")
        << wrapHTML("<h2>Heading level 2</h2>") << "## Heading level 2\n";
    QTest::newRow("Test H3 heading")
        << wrapHTML("<h3>Heading level 3</h3>") << "### Heading level 3\n";
    QTest::newRow("Test H4 heading")
        << wrapHTML("<h4>Heading level 4</h4>") << "#### Heading level 4\n";
    QTest::newRow("Test H5 heading")
        << wrapHTML("<h5>Heading level 5</h5>") << "##### Heading level 5\n";
    QTest::newRow("Test H6 heading")
        << wrapHTML("<h6>Heading level 6</h6>") << "###### Heading level 6\n";

    // Test Paragraphs
    QTest::newRow("Normal paragraphs")
        << wrapHTML("<p>I really like using Markdown.</p>")
        << "I really like using Markdown.\n";
    QTest::newRow("Line breaks")
        << wrapHTML("<p>This is the first line.<br>And this is the second line.</p>")
        << "This is the first line.  \nAnd this is the second line.\n";
    QTest::newRow("Breaks in paragraphs")
        << wrapHTML(R"(
            <h2 id="foo">foo</h2>
            <p>bar</p>
            <p><br></p>
            <p>Above is a break!</p>
        )")
        << "## foo\n"
           "\n"
           "bar\n"
           "\n"
           "  \n\n" // <p> will add a more linebreak.
           "\n"
           "Above is a break!\n";

    // Test Formating
    QTest::newRow("Bold")
        << wrapHTML("I just love <strong>bold text</strong>.")
        << "I just love **bold text**.";
    QTest::newRow("Bold without space")
        << wrapHTML("Love<strong>is</strong>bold")
        << "Love**is**bold";
    QTest::newRow("Italic")
        << wrapHTML("Italicized text is the <em>cat's meow</em>.")
        << "Italicized text is the *cat's meow*.";
    QTest::newRow("Italic without space")
        << wrapHTML("A<em>cat</em>meow")
        << "A*cat*meow";
    QTest::newRow("Bold and Italic")
        << wrapHTML("This text is <em><strong>really important</strong></em>.")
        << "This text is ***really important***.";
    QTest::newRow("Bold and Italic without space")
        << wrapHTML("This is really<em><strong>very</strong></em>important text.")
        << "This is really***very***important text.";
    QTest::newRow("Mutiple complex inline formating")
        << wrapHTML(R"(
            <p>
              <strong>Bold</strong><br />
              <em>Italic</em><br />
              <em><strong>Bola and italic</strong></em><br />
              <del>Strikethrought</del><br />
              <u>Underline</u><br />
              <ins>Underline2</ins>
            </p>
        )")
        << "**Bold**  \n"
           "*Italic*  \n"
           "***Bola and italic***  \n"
           "~~Strikethrought~~  \n"
           "<u>Underline</u>  \n"
           "<ins>Underline2</ins>\n";

    // Test links
    QTest::newRow("Normal link with a title")
        << R"(<p><a href="https://example.com" title="Example website">Example</a></p>)"
        << "[Example](https://example.com \"Example website\")\n";
    QTest::newRow("Image within link")
        << R"(
            <p>
              <a href="https://software-made-easy.github.io/MarkdownEdit/markdownedit.html">
                <img src="/home/tim/qtprojegt/MarkdownEdit/doc/images/Example.png"
                  alt="Example" />Klick the image for the preview</a>
            </p>
        )"
        << "[![Example](/home/tim/qtprojegt/MarkdownEdit/doc/images/Example.png)"
           "Klick the image for the preview](https://software-made-easy.github.io"
           "/MarkdownEdit/markdownedit.html)\n";

    // Test Blockquotes
    QTest::newRow("Simple blockquotes")
        << R"(
            <blockquote>
              <p>Dorothy followed her through many of the beautiful rooms in her castle.</p>
            </blockquote>
        )"
        << "> Dorothy followed her through many of the beautiful rooms in her castle.\n";
    QTest::newRow("Blockquotes with Multiple Paragraphs")
        << R"(
            <blockquote>
              <p>Dorothy followed her through many of the beautiful rooms in her castle.</p>

              <p>The Witch bade her clean the pots and kettles and sweep the floor and keep the fire fed with wood.</p>
            </blockquote>)"
        << "> Dorothy followed her through many of the beautiful rooms in her castle.\n"
           "> \n"
           "> The Witch bade her clean the pots and kettles and sweep the floor and keep the fire fed with wood.\n";
    QTest::newRow("Nested Blockquotes")
        << R"(
            <blockquote>
              <p>Dorothy followed her through many of the beautiful rooms in her castle.</p>

              <blockquote>
                <p>The Witch bade her clean the pots and kettles and sweep the floor and keep the fire fed with wood.</p>
              </blockquote>
            </blockquote>
        )"
        << "> Dorothy followed her through many of the beautiful rooms in her castle.\n"
           "> \n"
           "> > The Witch bade her clean the pots and kettles and sweep the floor and keep the fire fed with wood.\n";
    QTest::newRow("Multiple level of nested blockquotes")
        << R"(
            <blockquote>
              <blockquote><p>1</p></blockquote>
              <blockquote>
                <blockquote><p>1.2</p></blockquote>
              </blockquote>
              <blockquote>
                <blockquote>
                  <blockquote><p>1.3</p></blockquote>
                </blockquote>
              </blockquote>
            </blockquote>
        )"
        << "> > 1\n"
           "> \n"
           "> > > 1.2\n"
           "> \n"
           "> > > > 1.3\n";
    QTest::newRow("Multiple level of nested blockquotes 2")
        << R"(
            <blockquote>
              <blockquote>
                <p>1</p>
                <blockquote>
                  <p>1.2</p>
                  <blockquote><p>1.3</p></blockquote>
                </blockquote>
              </blockquote>
            </blockquote>
        )"
        << "> > 1\n"
           "> > \n"
           "> > > 1.2\n"
           "> > > \n"
           "> > > > 1.3\n";
    QTest::newRow("Blockquotes with Other Elements")
        << R"(
            <blockquote>
              <h4 class="no-anchor" id="the-quarterly-results-look-great">The quarterly results look great!</h4>

              <ul>
                <li>Revenue was off the chart.</li>
                <li>Profits were higher than ever.</li>
              </ul>

              <p><em>Everything</em> is going according to <strong>plan</strong>.</p>
            </blockquote>
        )"
        << "> #### The quarterly results look great!\n"
           "> \n"
           "> * Revenue was off the chart.\n"
           "> * Profits were higher than ever.\n"
           "> \n"
           "> *Everything* is going according to **plan**.\n";
    QTest::newRow("Nested Blockquotes with list")
        << R"(
            <blockquote>
              <p>Dorothy followed her through many of the beautiful rooms in her castle.</p>

              <blockquote>
                <ul>
                  <li>Revenue was off the chart.</li>
                  <li>Profits were higher than ever.</li>
                </ul>
              </blockquote>
            </blockquote>
        )"
        << "> Dorothy followed her through many of the beautiful rooms in her castle.\n"
           "> \n"
           "> > * Revenue was off the chart.\n"
           "> > * Profits were higher than ever.\n";
    QTest::newRow("Complex Nested Blockquotes with list")
        << R"(
            <blockquote>
              <p>1</p>
              <blockquote>
                <p>2</p>
                <blockquote>
                  <p>3</p>
                </blockquote>
              </blockquote>
              <p>1 again</p>
            </blockquote>
            <ul>
              <li>
                <blockquote>
                  <p>in list</p>
                </blockquote>
              </li>
            </ul>
        )"
        << "> 1\n"
           "> \n"
           "> > 2\n"
           "> > \n"
           "> > > 3\n"
           "> \n"
           "> 1 again\n"
           "\n"
           "* > in list\n";
    QTest::newRow("Blockquotes within list")
        << R"(
            <ul>
              <li>
                <blockquote>
                  <p>in list 1</p>
                </blockquote>
              </li>
              <li>
                <blockquote>
                  <p>in list 2</p>
                </blockquote>
              </li>
            </ul>
        )"
        << "* > in list 1\n"
           "* > in list 2\n";

    // Test Lists
    QTest::newRow("Test ordered lists")
        << wrapHTML(R"(
            <ol>
              <li>First item</li>
              <li>Second item</li>
              <li>Third item</li>
              <li>Fourth item</li>
            </ol>
        )")
        << "1. First item\n"
           "2. Second item\n"
           "3. Third item\n"
           "4. Fourth item\n";
    QTest::newRow("Test nested ordered lists")
        << wrapHTML(R"(
            <ol>
              <li>First item</li>
              <li>Second item</li>
              <li>Third item
                <ol>
                  <li>Indented item</li>
                  <li>Indented item</li>
                </ol>
              </li>
              <li>Fourth item</li>
            </ol>
        )")
        << "1. First item\n"
           "2. Second item\n"
           // FIXME: white space? It's also seen in some examples,
           // such as https://en.wikipedia.org/wiki/Markdown#Examples
           "3. Third item \n"
           "\t1. Indented item\n"
           "\t2. Indented item\n"
           "4. Fourth item\n";
    QTest::newRow("Test unordered lists")
        << wrapHTML(R"(
            <ul>
              <li>First item</li>
              <li>Second item</li>
              <li>Third item</li>
              <li>Fourth item</li>
            </ul>
        )")
        << "* First item\n"
           "* Second item\n"
           "* Third item\n"
           "* Fourth item\n";
    QTest::newRow("Test nested unordered lists")
        << wrapHTML(R"(
            <ul>
              <li>First item</li>
              <li>Second item</li>
              <li>Third item
                <ul>
                  <li>Indented item</li>
                  <li>Indented item</li>
                </ul>
              </li>
              <li>Fourth item</li>
            </ul>
        )")
        << "* First item\n"
           "* Second item\n"
           "* Third item \n"
           "\t* Indented item\n"
           "\t* Indented item\n"
           "* Fourth item\n";
    QTest::newRow("Do not escape number with dot, too complex")
        << wrapHTML(R"(
            <ul>
              <li>1968. A great year!</li>
              <li>I think 1969 was second best.</li>
            </ul>
        )")
        << "* 1968. A great year!\n"
           "* I think 1969 was second best.\n";
    QTest::newRow("Multiple lines in same list element")
        << wrapHTML(R"(
            <ul>
              <li>
                <p>A paragraph</p>
                <p>Same paragraph</p>
              </li>
              <li>
                <p>A paragraph</p>
                <p>Same paragraph</p>
              </li>
            </ul>
        )")
        << "* A paragraph\n"
           "  \n"
           "  Same paragraph\n"
           "* A paragraph\n"
           "  \n"
           "  Same paragraph\n";
    QTest::newRow("Simple nested list")
        << R"(
            <ul>
              <li>foo</li>
              <li>
                <ul>
                  <li>bar</li>
                  <li>foo</li>
                </ul>
              </li>
            </ul>
         )"
        << "* foo\n"
           "* \n" // FIXME: blank line?
           "\t* bar\n"
           "\t* foo\n";
    QTest::newRow("List entry with break")
        << R"(
            <ul>
              <li>list entry with<br />break</li>
              <li>
                <ul>
                  <li>Another<br />break<br />foo<br />bar</li>
                </ul>
              </li>
              <li>Hello World</li>
            </ul>
        )"
        << "* list entry with  \n"
           "\tbreak\n"
           "* \n"
           "\t*   Another  \n"
           "\tbreak  \n"
           "\tfoo  \n"
           "\tbar\n"
           "* Hello World\n";

    // TODO: test structure controlling, such as line breaks
}
