#ifndef HTML_WIZGUMBOHELPER_H
#define HTML_WIZGUMBOHELPER_H

#include <string>
#include <vector>

#include <QString>

#include "gumbo-parser/src/gumbo.h"

namespace Utils {
namespace Gumbo {

GumboOutput* parseFromString(const std::string &html);
GumboOutput* parseFromString(const QString &html);
void destroyGumboOutput(GumboOutput*);

QString innerText(GumboNode *node);
QString outerHtml(GumboNode *node);
QString outerRawHtml(GumboNode *node, std::string originHtml);

void getElementsByTagName(GumboNode *node, const QString &tagName, std::vector<GumboNode *> &tags);
void getElementsByTagAttr(GumboNode *node, const QString &tagName,
    const QString &attrName, const QString &attrValue, std::vector<GumboNode *> &tags);
QString getAttribute(GumboNode *node, const QString &attrName);
void filterTagsByAttribute(std::vector<GumboNode *> &tags, const QString &attrName, const QString &attrValue);
void insertAdjacentText(GumboNode *node, const QString &position, const QString &text, std::string &originalHtml);

} // namespace Gumbo
} // namespace Utils

#endif // HTML_WIZGUMBOHELPER_H