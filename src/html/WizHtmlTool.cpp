#include "WizHtmlTool.h"

#include <QString>
#include <QStringList>
#include <QDebug>

#include "WizGumboHelper.h"

namespace Utils {

/**
 * @brief Get the value of an attribute from a given tag string.
 * 
 * @param htmlTag HTML string.
 * @param tagAttributeName HTML tag name.
 * @return QString
 */
QString WizHtmlTagGetAttributeValue(
    const QString &htmlTag, const QString &tagAttributeName)
{
    return QString();
}


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
 * @brief Get all <a> tag from html text.
 * 
 * @param htmlText HTML string.
 * @param url HTML url.
 * @return QStringList
 */
QStringList WizHtmlEnumLinks(const QString &htmlText, const QString &url)
{
    return QStringList();    
}


/**
 * @brief Get pure text content from html string.
 * 
 * @param htmlText HTML string.
 * @param url HTML的URL
 * @return QString 
 */
QString WizHtmlGetContent(const QString &htmlText, const QString &url)
{
    return QString();
}

} // namespace Utils