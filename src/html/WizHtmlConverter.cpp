#include "WizHtmlConverter.h"

#include <sstream>
#include <algorithm>
#include <cctype>
#include <QDebug>

using namespace Utils::Gumbo;

namespace Utils {

WizHtmlConverter::WizHtmlConverter(const QString &html)
    : m_listLevel(-1)
    , m_blockquoteLevel(0)
{
    m_parser = new GumboParser(html);
}

WizHtmlConverter::~WizHtmlConverter()
{
    delete m_parser;
}

QString WizHtmlConverter::toMarkdown()
{
    if (!m_parser)
        return "";

    convert_to_markdown(m_parser->output()->root);
    flush();

    return m_lines.join("");
}

std::string escape_special_chars(const std::string& str) {
  std::ostringstream oss;
  for (const char& c : str) {
    switch (c) {
      case '\\':
        oss << "\\\\";
        break;
      case '`':
        oss << "\\`";
        break;
      case '*':
        oss << "\\*";
        break;
      case '_':
        oss << "\\_";
        break;
      case '[':
        oss << "\\[";
        break;
      case ']':
        oss << "\\]";
        break;
      case '<':
        oss << "\\<";
        break;
      case '>':
        oss << "\\>";
        break;
      case '#':
        oss << "\\#";
        break;
      case '|':
        oss << "\\|";
        break;
      default:
        oss << c;
        break;
    }
  }
  return oss.str();
}

std::string WizHtmlConverter::processTextNode(GumboNode* node)
{
    std::string puretext = escape_special_chars(node->v.text.text);

    // Collapse leading whitespace into one space
    std::size_t start_pos = puretext.find_first_not_of(" \t\r\n");
    if (start_pos != std::string::npos && start_pos > 0) {
        puretext.replace(0, start_pos, " ");
    }

    // Collapse trailing whitespace into one space
    std::size_t end_pos = puretext.find_last_not_of(" \t\r\n");
    if (end_pos != std::string::npos && end_pos < puretext.length() - 1) {
        puretext.replace(end_pos + 1, puretext.length() - end_pos - 1, " ");
    }

    // Replace consecutive whitespace with a single space
    auto end = std::unique(puretext.begin(), puretext.end(), [](char a, char b) {
      return std::isspace(a) && std::isspace(b);
    });
    puretext.erase(end, puretext.end());

    return puretext;
}

void WizHtmlConverter::processBlockStructure(GumboNode* node)
{
    if (node->type == GUMBO_NODE_TEXT
            || node->type == GUMBO_NODE_ELEMENT) {

        if (node->parent->type == GUMBO_NODE_ELEMENT) {
            GumboVector* neighbors = &node->parent->v.element.children;
            size_t idx = node->index_within_parent;
            if (idx > 0) {
                GumboNode* prev = nullptr;
                for (size_t i = idx; i > 0; --i) {
                    GumboNode* sib = static_cast<GumboNode*>(neighbors->data[i-1]);
                    if (sib->type == GUMBO_NODE_ELEMENT) prev = sib;
                }

                if (prev != nullptr) {
                    // TODO: define a const std::string to collect thest tags
                    // Tags need blank lines before and after
                    switch (prev->v.element.tag) {
                    case GUMBO_TAG_H1:
                    case GUMBO_TAG_H2:
                    case GUMBO_TAG_H3:
                    case GUMBO_TAG_H4:
                    case GUMBO_TAG_H5:
                    case GUMBO_TAG_H6:
                    case GUMBO_TAG_P:
                    case GUMBO_TAG_DIV:
                    case GUMBO_TAG_BLOCKQUOTE:
                    case GUMBO_TAG_OL:
                    case GUMBO_TAG_UL:
                    {
                        linebreak();
                        break;
                    }
                    default:
                        break;
                    }
                }
            }
        }
    }
}

void WizHtmlConverter::append(const std::string &str)
{
    m_current.append(str);
}

void WizHtmlConverter::linebreak(size_t n)
{
    m_current.append(std::string(n, '\n'));

    if (!m_nestedBlock.isEmpty()) {
        std::string prefix;
        auto e = m_nestedBlock.constBegin();
        while (e != m_nestedBlock.constEnd()) {
            switch (*e) {
            case GUMBO_TAG_BLOCKQUOTE:
            {
                prefix.append("> ");
                ++e;
                break;
            }
            case GUMBO_TAG_OL:
            case GUMBO_TAG_UL:
            {
                // Skip first list level
                GumboTag last = *e;
                while (++e != m_nestedBlock.constEnd() &&
                       (*e == GUMBO_TAG_OL || *e == GUMBO_TAG_UL)) {
                    last = *e;
                    prefix.append("\t");
                }

                if (m_listElemPrefixed.last()) {
                    prefix.append(last == GUMBO_TAG_UL ? "  " :
                                      std::string(std::to_string(m_listElemNo.last()).size(), ' ') + "  ");
                } else {
                    prefix.append(last == GUMBO_TAG_UL ? "* " :
                                      std::to_string(m_listElemNo.last()) + ". ");
                    m_listElemPrefixed.last() = true;
                }

                break;
            }
            default:
                ++e;
                break;
            }
        }

        flush(prefix);
    } else {
        flush();
    }
}

void WizHtmlConverter::flush()
{
    m_lines << QString::fromStdString(m_current);
    m_current.clear();
}

void WizHtmlConverter::flush(const std::string &prefix)
{
    m_lines << QString::fromStdString(prefix + m_current);
    m_current.clear();
}

void WizHtmlConverter::processChildren(GumboNode* node)
{
    GumboVector* children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i) {
        convert_to_markdown(static_cast<GumboNode*>(children->data[i]));
    }
}

// Recursive function to convert an HTML element into Markdown syntax
void WizHtmlConverter::convert_to_markdown(GumboNode* node) {
    processBlockStructure(node);

    if (node->type == GUMBO_NODE_TEXT) {
        append(processTextNode(node));
    } else if (node->type == GUMBO_NODE_ELEMENT) {
        // Markdown syntax: https://www.markdownguide.org/basic-syntax/
        switch (node->v.element.tag) {
        // Headings
        case GUMBO_TAG_H1:
        case GUMBO_TAG_H2:
        case GUMBO_TAG_H3:
        case GUMBO_TAG_H4:
        case GUMBO_TAG_H5:
        case GUMBO_TAG_H6:
        {
            append(std::string(node->v.element.tag - GUMBO_TAG_H1 + 1, '#'));
            append(" ");
            processChildren(node);
            linebreak();
            break;
        }
        // Paragraphs
        case GUMBO_TAG_P:
        case GUMBO_TAG_DIV:
        {
            processChildren(node);
            linebreak();
            break;
        }
        case GUMBO_TAG_BR:
        {
            append("  ");
            linebreak();
            break;
        }
        // Emphasis
        case GUMBO_TAG_EM:
        case GUMBO_TAG_I:
        case GUMBO_TAG_CITE:
        {
            append("*");
            processChildren(node);
            append("*");
            break;
        }
        case GUMBO_TAG_STRONG:
        case GUMBO_TAG_B:
        {
            append("**");
            processChildren(node);
            append("**");
            break;
        }
        case GUMBO_TAG_U:
        case GUMBO_TAG_INS:
        {
            auto name = std::string(gumbo_normalized_tagname(node->v.element.tag));
            append("<" + name + ">");
            processChildren(node);
            append("</" + name + ">");
            break;
        }
        case GUMBO_TAG_DEL:
        {
            append("~~");
            processChildren(node);
            append("~~");
            break;
        }
        // Blockquotes
        case GUMBO_TAG_BLOCKQUOTE:
        {
            m_blockquoteLevel++;
            m_nestedBlock.push(GUMBO_TAG_BLOCKQUOTE);
            if (node->v.element.children.length > 0) {
                // Convert the contents of the blockquote to Markdown recursively.
                for (unsigned int i = 0; i < node->v.element.children.length; ++i) {
                    GumboNode* child = static_cast<GumboNode*>(node->v.element.children.data[i]);
                    convert_to_markdown(child);
                }
            }
            m_blockquoteLevel--;
            auto tag = m_nestedBlock.pop();
            Q_ASSERT(tag == GUMBO_TAG_BLOCKQUOTE);
            break;
        }
        // Lists
        case GUMBO_TAG_UL:
        case GUMBO_TAG_OL:
        {
            // Enter new list block
            m_listLevel++;
            bool ordered = node->v.element.tag == GUMBO_TAG_OL;
            m_nestedBlock.push(ordered ? GUMBO_TAG_OL : GUMBO_TAG_UL);
            GumboVector* list_children = &node->v.element.children;
            unsigned int liNo = 0;
            for (unsigned int i = 0; i < list_children->length; ++i) {
                GumboNode* child = static_cast<GumboNode*>(list_children->data[i]);
                if (child->type == GUMBO_NODE_ELEMENT
                        && child->v.element.tag == GUMBO_TAG_LI) {
                    ++liNo;
                    m_listElemNo.push(liNo);
                    m_listElemPrefixed.push(false);
                    convert_to_markdown(child);
                    m_listElemNo.pop();
                    m_listElemPrefixed.pop();
                } else {
                    convert_to_markdown(child);
                }
            }
            // Exit current list block
            m_listLevel--;
            auto tag = m_nestedBlock.pop();
            Q_ASSERT(ordered ? tag == GUMBO_TAG_OL : tag == GUMBO_TAG_UL);
            break;
        }
        case GUMBO_TAG_LI:
        {
            GumboVector* li_children = &node->v.element.children;
            GumboNode* last_element = nullptr;
            for (unsigned int i = 0; i < li_children->length; ++i) {
                GumboNode* child = static_cast<GumboNode*>(li_children->data[i]);
                if (child->type == GUMBO_NODE_ELEMENT)
                    last_element = child;
                if (child->type == GUMBO_NODE_ELEMENT
                        && (child->v.element.tag == GUMBO_TAG_OL
                            || child->v.element.tag == GUMBO_TAG_UL)) {
                    linebreak();
                }

                convert_to_markdown(child);
            }

            if (last_element) {
                std::string tagname = getTagName(last_element);
                std::string key = "|" + tagname + "|";
                bool isInline = kTagsNonBreakingInline.find(key) != std::string::npos;
                if (isInline)
                    linebreak();
            } else {
                linebreak();
            }

            break;
        }
        // Code
        case GUMBO_TAG_CODE:
        {
            append("`");
            processChildren(node);
            append("`");
            break;
        }
        // Horizontal Rules
        case GUMBO_TAG_HR:
        {
            linebreak(2);
            append("---");
            linebreak(2);
            break;
        }
        case GUMBO_TAG_PRE:
        {
            append("```");
            linebreak();
            processChildren(node);
            linebreak();
            append("```");
            linebreak();
            break;
        }
        // Links
        case GUMBO_TAG_A:
        {
            append("[");
            processChildren(node);
            append("](");
            GumboAttribute* href_attr = gumbo_get_attribute(&node->v.element.attributes, "href");
            if (href_attr) append(href_attr->value);
            GumboAttribute* title_attr = gumbo_get_attribute(&node->v.element.attributes, "title");
            if (title_attr) {
                append(" \"");
                append(title_attr->value);
                append("\"");
            }
            append(")");
            break;
        }
        // Images
        case GUMBO_TAG_IMG:
        {
            append("![");
            GumboAttribute* alt_attr = gumbo_get_attribute(&node->v.element.attributes, "alt");
            if (alt_attr) append(alt_attr->value);
            append("](");
            GumboAttribute* src_attr = gumbo_get_attribute(&node->v.element.attributes, "src");
            if (src_attr) append(src_attr->value);
            GumboAttribute* title_attr = gumbo_get_attribute(&node->v.element.attributes, "title");
            if (title_attr) {
                append(" \"");
                append(title_attr->value);
                append("\"");
            }
            append(")");
            break;
        }
        //TODO: Table
        default:
            processChildren(node);
            break;
        }
    }
}

} // Utils
