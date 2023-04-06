#ifndef HTML_WIZHTMLCONVERTER_H
#define HTML_WIZHTMLCONVERTER_H

#include <QString>

#include "WizGumboHelper.h"

namespace Utils {

class WizHtmlConverter
{
public:
    WizHtmlConverter(const QString &html);
    ~WizHtmlConverter();

    QString toMarkdown();

private:
    void convert_to_markdown(GumboNode* node);

private:
    Utils::Gumbo::GumboParser *m_parser;
    std::string output;
    int m_listLevel;
    int m_blockquoteLevel;
};

} // Utils

#endif // HTML_WIZHTMLCONVERTER_H
