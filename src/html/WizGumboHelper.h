#ifndef HTML_WIZGUMBOHELPER_H
#define HTML_WIZGUMBOHELPER_H

#include <string>
#include <vector>

#include <QString>

#include "gumbo-parser/src/gumbo.h"

namespace Utils {
namespace Gumbo {

extern const std::string kTagsNonBreakingInline;
extern const std::string kTagsBlockLevel;
extern const std::string kTagsEmpty;
extern const std::string kTagsPreserveWhitespace;
extern const std::string kTagsSpecialHandling;
extern const std::string kTagsNoEntitySub;

std::string getTagName(GumboNode *node);

GumboOutput* parseFromString(const std::string &html);
GumboOutput* parseFromString(const QString &html);
void destroyGumboOutput(GumboOutput*);

QString innerText(GumboNode *node);
QString outerHtml(GumboNode *node);
QString outerRawHtml(GumboNode *node, const std::string &originHtml);

void getElementsByTagName(GumboNode *node, const QString &tagName, std::vector<GumboNode *> &tags);
void getElementsByTagAttr(GumboNode *node, const QString &tagName,
    const QString &attrName, const QString &attrValue, std::vector<GumboNode *> &tags);
QString getAttribute(GumboNode *node, const QString &attrName);
void filterTagsByAttribute(std::vector<GumboNode *> &tags, const QString &attrName, const QString &attrValue);
void insertAdjacentText(GumboNode *node, const QString &position, const QString &text, std::string &originalHtml);

class GumboParser {
public:
    explicit GumboParser(const QString &html);
    ~GumboParser();

    GumboOutput* output() const { return output_; }
    const std::string& html() const { return html_; }

private:
    std::string html_;
    GumboOutput* output_;
};

} // namespace Gumbo
} // namespace Utils

#endif // HTML_WIZGUMBOHELPER_H
