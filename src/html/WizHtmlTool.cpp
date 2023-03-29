#include "WizHtmlTool.h"

#include <QString>
#include <QStringList>
#include <QDebug>
#include <QTextDocument>
#include <sstream>
#include <algorithm>
#include <cctype>

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


struct Replacement
{
    size_t start;
    size_t end;
    std::string text;
};

QString WizReplaceTagsWithText(const QString &htmlText,
    std::function<QString (const QMap<QString, QString> &, const QString &)> callback,
    const QString &tagName,
    const QString &tagAttributeName,
    const QString &tagAttributeValue)
{
    Gumbo::GumboParser parser(htmlText);
    std::string rawHtmlString = parser.html();
    GumboOutput* output = parser.get();

    // Find all matched tags
    std::vector<GumboNode *> tags;
    Utils::Gumbo::getElementsByTagAttr(
        output->root, tagName, tagAttributeName, tagAttributeValue, tags);

    QList<Replacement> replacements;
    for (auto tag : tags) {
        QMap<QString, QString> attrs;
        GumboElement* element = &tag->v.element;
        GumboVector* attributes = &element->attributes;
        for (unsigned int i = 0; i < attributes->length; ++i)
        {
            GumboAttribute* attribute = static_cast<GumboAttribute*>(attributes->data[i]);
            attrs[attribute->name] = attribute->value;
        }

        QString tagSource = Gumbo::outerRawHtml(tag, rawHtmlString);
        unsigned int start_pos = element->start_pos.offset;
        unsigned int end_pos = element->end_pos.offset;
        replacements.append({
            start_pos,
            // Handle self-closing tags
            start_pos == end_pos ?
                start_pos + element->original_tag.length :
                end_pos + element->original_end_tag.length,
            callback(attrs, tagSource).toStdString()
        });
    }

    std::sort(
        replacements.begin(), replacements.end(),
        [](const Replacement &s1, const Replacement &s2) {
            if (s1.start == s2.start)
                return s1.end < s2.end;
            else
                return s1.start < s2.start;
        });

    std::string outputHtml = rawHtmlString;
    std::ostringstream oss;
    size_t pos = 0;
    foreach (const Replacement& rep, replacements) {
        oss << rawHtmlString.substr(pos, rep.start - pos) << rep.text;
        pos = rep.end;
    }

    // Concatenate the tail
    oss << rawHtmlString.substr(pos);

    return QString::fromStdString(oss.str());
}

} // namespace Utils