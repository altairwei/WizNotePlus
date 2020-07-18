#include "WizHtmlTool.h"

#include <QString>
#include <QStringList>
#include <QDebug>
#include <QTextDocument>

#include "WizGumboHelper.h"

namespace Utils {


/**
 * @brief Get specific tags from given html text.
 * 
 *  This function will return the original pieces html string of tags.
 *  If you have any questions, please check the test cases.
 *           
 * @param htmlText HTML string.
 * @param tagName HTML tag name.
 * @param tagAttributeName HTML attribute name.
 * @param tagAttributeValue HTML attribute value.
 * @return QStringList 
 */
QStringList WizHtmlExtractTags(
    const QString &htmlText,
    const QString &tagName,
    const QString &tagAttributeName /*= ""*/,
    const QString &tagAttributeValue /*= ""*/)
{
    // Gumbo only accepts UTF-8 encoding string.
    std::string rawHtmlString = htmlText.toStdString();
    GumboOutput* output = Utils::Gumbo::parseFromString(rawHtmlString);

    // Find all matched tags
    std::vector<GumboNode *> tags;
    Utils::Gumbo::getElementsByTagAttr(
        output->root, tagName, tagAttributeName, tagAttributeValue, tags);

    // Extract outer html
    QStringList results;
    for (auto tag : tags) {
        results.append(Utils::Gumbo::outerRawHtml(tag, rawHtmlString));
    }

    // Destroy node tree.
    Utils::Gumbo::destroyGumboOutput(output);

    return results;
}


QString WizHtmlInsertText(
    const QString &htmlText,
    const QString &text,
    const QString &position,
    const QString &tagName,
    const QString &tagAttributeName /*= ""*/,
    const QString &tagAttributeValue /*= ""*/)
{
    std::string rawHtmlString = htmlText.toStdString();
    GumboOutput* output = Utils::Gumbo::parseFromString(rawHtmlString);

    std::vector<GumboNode *> tags;
    Utils::Gumbo::getElementsByTagAttr(
        output->root, tagName, tagAttributeName, tagAttributeValue, tags);

    if (tags.empty()) {
        return htmlText;
    }

    GumboNode *firstFound = tags[0];
    Utils::Gumbo::insertAdjacentText(firstFound, position, text, rawHtmlString);

    Utils::Gumbo::destroyGumboOutput(output);

    return QString::fromStdString(rawHtmlString);
}


/**
 * @brief Get pure text content from html string.
 * 
 *   TODO: add more test cases.
 * 
 * @param htmlText HTML string.
 * @return QString 
 */
QString WizHtmlGetContent(const QString &htmlText)
{
    GumboOutput *output = Utils::Gumbo::parseFromString(htmlText);
    const QString content = Utils::Gumbo::innerText(output->root);
    Utils::Gumbo::destroyGumboOutput(output);

    return content;
}


/**
 * @brief Get pure text content from html string.
 * 
 * @param htmlText 
 * @return QString 
 */
QString WizHtmlGetPureText(const QString &htmlText)
{
   QTextDocument doc;
   doc.setHtml(htmlText);
   QString content = doc.toPlainText(); //auto deHtmlEscaped
   return content;
}


} // namespace Utils