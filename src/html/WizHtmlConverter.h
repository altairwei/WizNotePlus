#ifndef HTML_WIZHTMLCONVERTER_H
#define HTML_WIZHTMLCONVERTER_H

#include <QString>
#include <QStringList>
#include <QStack>

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
    void processChildren(GumboNode* node);
    void processBlockStructure(GumboNode* node);
    std::string processTextNode(GumboNode* node);

    void append(const std::string &str);
    void linebreak(size_t n = 1);
    void flush();
    void flush(const std::string &prefix);

private:
    Utils::Gumbo::GumboParser *m_parser;
    std::string m_current;
    QStringList m_lines;
    int m_listLevel;
    int m_blockquoteLevel;
    QStack<GumboTag> m_nestedBlock;
    QStack<size_t> m_listElemNo;
    QStack<bool> m_listElemPrefixed ;
};

} // Utils

#endif // HTML_WIZHTMLCONVERTER_H
