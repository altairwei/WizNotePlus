#include "WizHtmlConverter.h"

#include <sstream>
#include <algorithm>
#include <cctype>

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

    return QString::fromStdString(output);
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

void convert_structure(GumboNode* node, std::string& output)
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
                        if (node->parent->v.element.tag == GUMBO_TAG_BLOCKQUOTE)
                            output.append(">\n");
                        else
                            output.append("\n");
                        break;
                    }
                    default:
                        break;
                    }
                }
            }

            if (node->parent->v.element.tag == GUMBO_TAG_BLOCKQUOTE)
                output.append("> ");
        }

    }
}

// Recursive function to convert an HTML element into Markdown syntax
void WizHtmlConverter::convert_to_markdown(GumboNode* node) {
    convert_structure(node, output);

    if (node->type == GUMBO_NODE_TEXT) {
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

        output.append(puretext);
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
            output.append(std::string(node->v.element.tag - GUMBO_TAG_H1 + 1, '#'));
            output.append(" ");
            GumboVector* heading_children = &node->v.element.children;
            for (unsigned int i = 0; i < heading_children->length; ++i) {
                convert_to_markdown(static_cast<GumboNode*>(heading_children->data[i]));
            }
            output.append("\n");
            break;
        }
        // Paragraphs
        case GUMBO_TAG_P:
        case GUMBO_TAG_DIV:
        {
            GumboVector* paragraph_children = &node->v.element.children;
            for (unsigned int i = 0; i < paragraph_children->length; ++i) {
                convert_to_markdown(static_cast<GumboNode*>(paragraph_children->data[i]));
            }
            output.append("\n");
            break;
        }
        case GUMBO_TAG_BR:
        {
            output.append("  \n");
            break;
        }
        // Emphasis
        case GUMBO_TAG_EM:
        case GUMBO_TAG_I:
        case GUMBO_TAG_CITE:
        {
            output.append("*");
            GumboVector* emphasis_children = &node->v.element.children;
            for (unsigned int i = 0; i < emphasis_children->length; ++i) {
                convert_to_markdown(static_cast<GumboNode*>(emphasis_children->data[i]));
            }
            output.append("*");
            break;
        }
        case GUMBO_TAG_STRONG:
        case GUMBO_TAG_B:
        {
            output.append("**");
            GumboVector* strong_children = &node->v.element.children;
            for (unsigned int i = 0; i < strong_children->length; ++i) {
                convert_to_markdown(static_cast<GumboNode*>(strong_children->data[i]));
            }
            output.append("**");
            break;
        }
        // Blockquotes
        case GUMBO_TAG_BLOCKQUOTE:
        {
            m_blockquoteLevel++;
            if (node->v.element.children.length > 0) {
                // Convert the contents of the blockquote to Markdown recursively.
                for (unsigned int i = 0; i < node->v.element.children.length; ++i) {
                    GumboNode* child = static_cast<GumboNode*>(node->v.element.children.data[i]);
                    convert_to_markdown(child);
                }
            }
            m_blockquoteLevel--;
            break;
        }
        // Lists
        case GUMBO_TAG_UL:
        case GUMBO_TAG_OL:
        {
            // Enter new list block
            m_listLevel++;
            GumboVector* list_children = &node->v.element.children;
            unsigned int liNo = 0;
            for (unsigned int i = 0; i < list_children->length; ++i) {
                GumboNode* child = static_cast<GumboNode*>(list_children->data[i]);
                if (child->type == GUMBO_NODE_ELEMENT
                        && child->v.element.tag == GUMBO_TAG_LI) {
                    if (++liNo > 1 && m_blockquoteLevel > 0) {
                        int rep = m_blockquoteLevel;
                        while(rep-- > 0)
                            output.append("> ");
                    }

                    output.append(std::string(m_listLevel, '\t'));
                    output.append(node->v.element.tag == GUMBO_TAG_UL ?
                                      "* " : std::to_string(liNo) + ". ");
                }

                convert_to_markdown(child);
            }
            // Exit current list block
            m_listLevel--;
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
                    output.append("\n");
                }

                convert_to_markdown(child);
            }

            if (last_element) {
                std::string tagname = getTagName(last_element);
                std::string key = "|" + tagname + "|";
                bool isInline = kTagsNonBreakingInline.find(key) != std::string::npos;
                if (isInline)
                    output.append("\n");
            } else {
                output.append("\n");
            }

            break;
        }
        // Code
        case GUMBO_TAG_CODE:
        {
            output.append("`");
            GumboVector* code_children = &node->v.element.children;
            for (unsigned int i = 0; i < code_children->length; ++i) {
                convert_to_markdown(static_cast<GumboNode*>(code_children->data[i]));
            }
            output.append("`");
            break;
        }
        // Horizontal Rules
        case GUMBO_TAG_HR:
        {
            output.append("\n\n---\n\n");
            break;
        }
        case GUMBO_TAG_PRE:
        {
            output.append("```\n");
            GumboVector* pre_children = &node->v.element.children;
            for (unsigned int i = 0; i < pre_children->length; ++i) {
                convert_to_markdown(static_cast<GumboNode*>(pre_children->data[i]));
            }
            output.append("\n```\n");
            break;
        }
        // Links
        case GUMBO_TAG_A:
        {
            output.append("[");
            GumboVector* children = &node->v.element.children;
            for (unsigned int i = 0; i < children->length; ++i) {
                convert_to_markdown(static_cast<GumboNode*>(children->data[i]));
            }
            output.append("](");
            GumboAttribute* href_attr = gumbo_get_attribute(&node->v.element.attributes, "href");
            if (href_attr) {
                output.append(href_attr->value);
            }
            output.append(")");
            break;
        }
        // Images
        case GUMBO_TAG_IMG:
        {
            output.append("![");
            GumboAttribute* alt_attr = gumbo_get_attribute(&node->v.element.attributes, "alt");
            if (alt_attr) output.append(alt_attr->value);
            output.append("](");
            GumboAttribute* src_attr = gumbo_get_attribute(&node->v.element.attributes, "src");
            if (src_attr) output.append(src_attr->value);
            GumboAttribute* title_attr = gumbo_get_attribute(&node->v.element.attributes, "title");
            if (title_attr) {
                output.append(" \"");
                output.append(title_attr->value);
                output.append("\"");
            }
            output.append(")");
            break;
        }
        //TODO: Table
        default:
        {
            GumboVector* default_children = &node->v.element.children;
            for (unsigned int i = 0; i < default_children->length; ++i) {
                convert_to_markdown(static_cast<GumboNode*>(default_children->data[i]));
            }
            break;
        }
        }
    }
}

} // Utils
