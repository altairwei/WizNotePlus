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

void convert_to_markdown(GumboNode* node, std::string& output);

void convert_list(GumboNode* node, std::string& output, unsigned int level)
{
    if (node->type != GUMBO_NODE_ELEMENT) {
      return;
    }

    switch (node->v.element.tag) {
    case GUMBO_TAG_UL:
    case GUMBO_TAG_OL:
    {
        GumboVector* list_children = &node->v.element.children;
        unsigned int liNo = 0;
        if (level == 0)
            output.append("\n");
        for (unsigned int i = 0; i < list_children->length; ++i) {
            GumboNode* child = static_cast<GumboNode*>(list_children->data[i]);
            if (child->type == GUMBO_NODE_ELEMENT
                    && child->v.element.tag == GUMBO_TAG_LI) {
                output.append("\n");
                output.append(std::string(level, '\t'));
                output.append(node->v.element.tag == GUMBO_TAG_UL ?
                                  "* " : std::to_string(++liNo) + ". ");
                convert_list(child, output, level);
            } else {
                convert_to_markdown(child, output);
            }
        }
        if (level == 0)
            output.append("\n\n");
        break;
    }
    case GUMBO_TAG_LI:
    {
        GumboVector* li_children = &node->v.element.children;
        for (unsigned int i = 0; i < li_children->length; ++i) {
            GumboNode* child = static_cast<GumboNode*>(li_children->data[i]);
            if (child->type == GUMBO_NODE_ELEMENT
                    && (child->v.element.tag == GUMBO_TAG_OL
                        || child->v.element.tag == GUMBO_TAG_UL)) {
                convert_list(child, output, level + 1);
            } else {
                convert_to_markdown(child, output);
            }
        }
        break;
    }
    default:
        convert_to_markdown(node, output);
        break;
    }
}

// Recursive function to convert an HTML element into Markdown syntax
void convert_to_markdown(GumboNode* node, std::string& output) {
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
                convert_to_markdown(static_cast<GumboNode*>(heading_children->data[i]), output);
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
                convert_to_markdown(static_cast<GumboNode*>(paragraph_children->data[i]), output);
            }
            output.append("\n\n");
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
                convert_to_markdown(static_cast<GumboNode*>(emphasis_children->data[i]), output);
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
                convert_to_markdown(static_cast<GumboNode*>(strong_children->data[i]), output);
            }
            output.append("**");
            break;
        }
        // Blockquotes
        case GUMBO_TAG_BLOCKQUOTE:
        {
            if (node->v.element.children.length > 0) {
                output.append("> ");

                // Convert the contents of the blockquote to Markdown recursively.
                for (unsigned int i = 0; i < node->v.element.children.length; ++i) {
                    GumboNode* child = static_cast<GumboNode*>(node->v.element.children.data[i]);
                    convert_to_markdown(child, output);

                    // Add a newline character to the end of each line.
                    output.append("\n> ");
                }

                // If the blockquote has multiple paragraphs, add an additional newline character.
                if (node->v.element.children.length > 1) {
                    output.append("\n");
                }
            }
            break;
        }
        // Lists
        case GUMBO_TAG_UL:
        case GUMBO_TAG_OL:
        {
            convert_list(node, output, 0);
            break;
        }
        // Code
        case GUMBO_TAG_CODE:
        {
            output.append("`");
            GumboVector* code_children = &node->v.element.children;
            for (unsigned int i = 0; i < code_children->length; ++i) {
                convert_to_markdown(static_cast<GumboNode*>(code_children->data[i]), output);
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
                convert_to_markdown(static_cast<GumboNode*>(pre_children->data[i]), output);
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
                convert_to_markdown(static_cast<GumboNode*>(children->data[i]), output);
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
                convert_to_markdown(static_cast<GumboNode*>(default_children->data[i]), output);
            }
            break;
        }
        }
    }
}

QString WizHtmlToMarkdown(const QString &htmlText)
{
    std::string rawHtmlString = htmlText.toStdString();
    GumboOutput* output = Utils::Gumbo::parseFromString(rawHtmlString);

    // Convert HTML to Markdown
    std::string markdown_output;
    convert_to_markdown(output->root, markdown_output);

    Utils::Gumbo::destroyGumboOutput(output);

    return QString::fromStdString(markdown_output);
}

} // namespace Utils
