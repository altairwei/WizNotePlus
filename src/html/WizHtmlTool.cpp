#include "WizHtmlTool.h"

#include <QString>
#include <QStringList>
#include <QDebug>

#include "gumbo-parser/gumbo.h"

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
    std::string htmlString = htmlText.toUtf8().toStdString();
    const char *buf = htmlString.c_str();
    GumboOutput* output = gumbo_parse(buf);

    // Find all tags
    std::vector<GumboNode *> tags;
    Utils::Gumbo::getElementsByTagName(output->root, tagName, tags);
    if (!tagAttributeName.isEmpty()) {
        Utils::Gumbo::filterTagsByAttribute(tags, tagAttributeName, tagAttributeValue);
    }

    // Extract outer html
    QStringList results;
    for (auto tag : tags) {
        results.append(Utils::Gumbo::outerHtml(tag));
    }

    // Destroy node tree.
    gumbo_destroy_output(&kGumboDefaultOptions, output);

    return results;
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