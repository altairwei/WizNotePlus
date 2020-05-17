#ifndef HTML_WIZGUMBOHELPER_H
#define HTML_WIZGUMBOHELPER_H

#include <QString>
#include <vector>

#include "gumbo-parser/gumbo.h"

namespace Utils {
namespace Gumbo {

QString innerText(GumboNode *node);
QString outerHtml(GumboNode *node);

void getElementsByTagName(GumboNode *node, const QString &tagName, std::vector<GumboNode *> &tags);
void filterTagsByAttribute(std::vector<GumboNode *> &tags, const QString &attrName, const QString &attrValue);

} // namespace Gumbo
} // namespace Utils

#endif // HTML_WIZGUMBOHELPER_H