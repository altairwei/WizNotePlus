#include "WizGumboHelper.h"

#include <vector>
#include <string>
#include <iostream>

#include <QString>
#include <QDebug>

#include "gumbo-parser/src/gumbo.h"


/** serialize functions were copied from gumbo-parser examples. */

static std::string nonbreaking_inline  = "|a|abbr|acronym|b|bdo|big|cite|code|dfn|em|font|i|img|kbd|nobr|s|small|span|strike|strong|sub|sup|tt|";
static std::string empty_tags          = "|area|base|basefont|bgsound|br|command|col|embed|event-source|frame|hr|image|img|input|keygen|link|menuitem|meta|param|source|spacer|track|wbr|";
static std::string preserve_whitespace = "|pre|textarea|script|style|";
static std::string special_handling    = "|html|body|";
static std::string no_entity_sub       = "|script|style|";

static inline void rtrim(std::string &s)
{
    s.erase(s.find_last_not_of(" \n\r\t") + 1);
}

static inline void ltrim(std::string &s)
{
    s.erase(0, s.find_first_not_of(" \n\r\t"));
}

static void replace_all(std::string &s, const char *s1, const char *s2)
{
    std::string t1(s1);
    size_t len = t1.length();
    size_t pos = s.find(t1);
    while (pos != std::string::npos)
    {
        s.replace(pos, len, s2);
        pos = s.find(t1, pos + len);
    }
}

static std::string substitute_xml_entities_into_text(const std::string &text)
{
    std::string result = text;
    // replacing & must come first
    replace_all(result, "&", "&amp;");
    replace_all(result, "<", "&lt;");
    replace_all(result, ">", "&gt;");
    return result;
}

static std::string substitute_xml_entities_into_attributes(char quote, const std::string &text)
{
    std::string result = substitute_xml_entities_into_text(text);
    if (quote == '"')
    {
        replace_all(result, "\"", "&quot;");
    }
    else if (quote == '\'')
    {
        replace_all(result, "'", "&apos;");
    }
    return result;
}

static std::string handle_unknown_tag(GumboStringPiece *text)
{
    std::string tagname = "";
    if (text->data == NULL)
    {
        return tagname;
    }
    // work with copy GumboStringPiece to prevent asserts
    // if try to read same unknown tag name more than once
    GumboStringPiece gsp = *text;
    gumbo_tag_from_original_text(&gsp);
    tagname = std::string(gsp.data, gsp.length);
    return tagname;
}

static std::string get_tag_name(GumboNode *node)
{
    std::string tagname;
    // work around lack of proper name for document node
    if (node->type == GUMBO_NODE_DOCUMENT)
    {
        tagname = "document";
    }
    else
    {
        tagname = gumbo_normalized_tagname(node->v.element.tag);
    }
    if (tagname.empty())
    {
        tagname = handle_unknown_tag(&node->v.element.original_tag);
    }
    return tagname;
}

static std::string build_doctype(GumboNode *node)
{
    std::string results = "";
    if (node->v.document.has_doctype)
    {
        results.append("<!DOCTYPE ");
        results.append(node->v.document.name);
        std::string pi(node->v.document.public_identifier);
        if ((node->v.document.public_identifier != NULL) && !pi.empty())
        {
            results.append(" PUBLIC \"");
            results.append(node->v.document.public_identifier);
            results.append("\" \"");
            results.append(node->v.document.system_identifier);
            results.append("\"");
        }
        results.append(">\n");
    }
    return results;
}

static std::string build_attributes(GumboAttribute *at, bool no_entities)
{
    std::string atts = " ";
    atts.append(at->name);

    // how do we want to handle attributes with empty values
    // <input type="checkbox" checked />  or <input type="checkbox" checked="" />

    if ((!std::string(at->value).empty()) ||
        (at->original_value.data[0] == '"') ||
        (at->original_value.data[0] == '\''))
    {

        // determine original quote character used if it exists
        char quote = at->original_value.data[0];
        std::string qs = "";
        if (quote == '\'')
            qs = std::string("'");
        if (quote == '"')
            qs = std::string("\"");
        atts.append("=");
        atts.append(qs);
        if (no_entities)
        {
            atts.append(at->value);
        }
        else
        {
            atts.append(substitute_xml_entities_into_attributes(quote, std::string(at->value)));
        }
        atts.append(qs);
    }
    return atts;
}

// forward declaration
static std::string serialize(GumboNode*);


// serialize children of a node
// may be invoked recursively

static std::string serialize_contents(GumboNode* node) {
    std::string contents        = "";
    std::string tagname         = get_tag_name(node);
    std::string key             = "|" + tagname + "|";
    bool no_entity_substitution = no_entity_sub.find(key) != std::string::npos;
    bool keep_whitespace        = preserve_whitespace.find(key) != std::string::npos;
    bool is_inline              = nonbreaking_inline.find(key) != std::string::npos;

    // build up result for each child, recursively if need be
    GumboVector *children = &node->v.element.children;

    for (unsigned int i = 0; i < children->length; ++i)
    {
        GumboNode *child = static_cast<GumboNode *>(children->data[i]);

        if (child->type == GUMBO_NODE_TEXT)
        {
            if (no_entity_substitution)
            {
                contents.append(std::string(child->v.text.text));
            }
            else
            {
                contents.append(substitute_xml_entities_into_text(std::string(child->v.text.text)));
            }
        }
        else if (child->type == GUMBO_NODE_ELEMENT || child->type == GUMBO_NODE_TEMPLATE)
        {
            contents.append(serialize(child));
        }
        else if (child->type == GUMBO_NODE_WHITESPACE)
        {
            // keep all whitespace to keep as close to original as possible
            contents.append(std::string(child->v.text.text));
        }
        else if (child->type != GUMBO_NODE_COMMENT)
        {
            // Does this actually exist: (child->type == GUMBO_NODE_CDATA)
            fprintf(stderr, "unknown element of type: %d\n", child->type);
        }
    }
    return contents;
}


// serialize a GumboNode back to html/xhtml
// may be invoked recursively

static std::string serialize(GumboNode* node) {
    // special case the document node
    if (node->type == GUMBO_NODE_DOCUMENT)
    {
        std::string results = build_doctype(node);
        results.append(serialize_contents(node));
        return results;
    }

    std::string close = "";
    std::string closeTag = "";
    std::string atts = "";
    std::string tagname = get_tag_name(node);
    std::string key = "|" + tagname + "|";
    bool need_special_handling = special_handling.find(key) != std::string::npos;
    bool is_empty_tag = empty_tags.find(key) != std::string::npos;
    bool no_entity_substitution = no_entity_sub.find(key) != std::string::npos;
    bool is_inline = nonbreaking_inline.find(key) != std::string::npos;

    // build attr string
    const GumboVector *attribs = &node->v.element.attributes;
    for (unsigned int i = 0; i < attribs->length; ++i)
    {
        GumboAttribute *at = static_cast<GumboAttribute *>(attribs->data[i]);
        atts.append(build_attributes(at, no_entity_substitution));
    }

    // determine closing tag type
    if (is_empty_tag)
    {
        close = "/";
    }
    else
    {
        closeTag = "</" + tagname + ">";
    }

    // serialize your contents
    std::string contents = serialize_contents(node);

    if (need_special_handling)
    {
        ltrim(contents);
        rtrim(contents);
        contents.append("\n");
    }

    // build results
    std::string results;
    results.append("<" + tagname + atts + close + ">");
    if (need_special_handling)
        results.append("\n");
    results.append(contents);
    results.append(closeTag);
    if (need_special_handling)
        results.append("\n");
    return results;
}

namespace Utils {

namespace Gumbo {


/** Parse html string to Gumbo node tree, results must be destroyed. */
GumboOutput* parseFromString(const std::string &html)
{
    const char *buf = html.data();
    size_t bufLen = html.length();
    // If you used contents.c_str(), it'd be harder to match up original
    // positions, because c_str() creates a copy of the string and you can't do
    // pointer arithmetic betweent contents.data() and the original_* pointers.
    GumboOutput* output = gumbo_parse_with_options(
        &kGumboDefaultOptions, buf, bufLen);

    return output;
}


GumboOutput* parseFromString(const QString &html)
{
    // Gumbo only accepts UTF-8 encoding string.
    std::string htmlString = html.toUtf8().toStdString();
    return parseFromString(htmlString);
}


/** Free the memory of GumboOutput, used with parseFromString. */
void destroyGumboOutput(GumboOutput* output)
{
    gumbo_destroy_output(&kGumboDefaultOptions, output);
}


/** Get the inner text content of node. */
QString innerText(GumboNode *node)
{
    if (node->type == GUMBO_NODE_TEXT) {
        return QString(node->v.text.text);
    }
    else if (node->type == GUMBO_NODE_ELEMENT &&
                node->v.element.tag != GUMBO_TAG_SCRIPT &&
                node->v.element.tag != GUMBO_TAG_STYLE)
    {
        QString contents = "";
        GumboVector *children = &node->v.element.children;
        for (unsigned int i = 0; i < children->length; ++i) {
            GumboNode * pNode = (GumboNode *)children->data[i];
            std::string pTagName = get_tag_name(pNode);
            bool is_inline = nonbreaking_inline.find("|" + pTagName + "|") != std::string::npos;
            bool is_special_handling = special_handling.find("|" + pTagName + "|") != std::string::npos;
            const QString text = innerText(pNode);
            if (i != 0 && !text.isEmpty()) {
                if (!is_inline && !is_special_handling) {
                    contents.append("\n");
                }
            }
            contents.append(text);
        }
        return contents;
    }
    else
    {
        return QString("");
    }
}

/** 以QTextDocument.toPlainText标准实现这个函数 */
QString plainText(GumboNode *node)
{
    return QString();
}


/** Serialize the GumboNode to html string. */
QString outerHtml(GumboNode *node)
{
    return QString::fromStdString(serialize(node));
}


/** Extract substring of raw html text. */
QString outerRawHtml(GumboNode *node, std::string originHtml)
{
    if (node->type != GUMBO_NODE_ELEMENT) {
        return QString();
    }

    const GumboElement *thisElem = &(node->v.element);
    size_t start_offset = thisElem->start_pos.offset;
    size_t end_offset = thisElem->end_pos.offset + thisElem->original_end_tag.length;

    const std::string raw = originHtml.substr(start_offset, end_offset - start_offset);

    return QString::fromStdString(raw);
}


static GumboTag toNodeType(const QString tag) {
    std::string t = tag.toUtf8().toStdString();
    const char *tagName = t.c_str();
    return gumbo_tag_enum(tagName);
}


/** Search nodes for given tag name. */
void getElementsByTagName(GumboNode *node, const QString &tagName, std::vector<GumboNode *> &tags)
{
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }

    const GumboTag tagType = toNodeType(tagName);
    if (node->v.element.tag == tagType)
        tags.push_back(node);

    GumboVector *children = &node->v.element.children;
    for (unsigned int i = 0; i < children->length; ++i)
    {
        getElementsByTagName(static_cast<GumboNode *>(children->data[i]), tagName, tags);
    }
}


void getElementsByTagAttr(
    GumboNode *node,
    const QString &tagName,
    const QString &attrName,
    const QString &attrValue,
    std::vector<GumboNode *> &tags)
{
    std::vector<GumboNode *> tempTags;
    getElementsByTagName(node, tagName, tempTags);
    if (!attrName.isEmpty()) {
        filterTagsByAttribute(tempTags, attrName, attrValue);
    }

    tags = tempTags;
}


QString getAttribute(GumboNode *node, const QString &attrName)
{
    if (node->type != GUMBO_NODE_ELEMENT) {
        return QString();
    }

    std::string attrNameString = attrName.toUtf8().toStdString();
    GumboAttribute* attr = gumbo_get_attribute(
        &node->v.element.attributes, attrNameString.c_str());

    return QString(attr->value);
}


/**
 * @brief Insert plain text or html text to original html string.
 * 
 * @param node 
 * @param position See https://developer.mozilla.org/en-US/docs/Web/API/Element/insertAdjacentText
 * @param text 
 * @param originalHtml 
 * @return QString 
 */
void insertAdjacentText(GumboNode *node, const QString &position, const QString &text, std::string &originalHtml)
{
    if (node->type != GUMBO_NODE_ELEMENT) {
        return;
    }

    GumboElement *elem = &node->v.element;
    size_t pos = 0;
    if (position == "beforebegin")
    {
        pos = elem->start_pos.offset;
    }
    else if (position == "afterbegin")
    {
        pos = elem->start_pos.offset + elem->original_tag.length;
    }
    else if (position == "beforeend")
    {
        pos = elem->end_pos.offset;
    }
    else if (position == "afterend")
    {
        pos = elem->end_pos.offset + elem->original_end_tag.length;
    }
    else
    {
        return;
    }

    originalHtml.insert(pos, text.toStdString());
}


void filterTagsByAttribute(
    std::vector<GumboNode *> &tags, const QString &attrName, const QString &attrValue)
{
    tags.erase(std::remove_if(tags.begin(), tags.end(), [=](const GumboNode *tag) {
        // Remove non-element node.
        if (tag->type != GUMBO_NODE_ELEMENT)
            return true;
        std::string attrNameString = attrName.toUtf8().toStdString();
        GumboAttribute* attr = gumbo_get_attribute(&tag->v.element.attributes, attrNameString.c_str());
        if (attr != nullptr) {
            // Remove node whose attr does not match given value.
            return  QString(attr->value) != attrValue;
        } else {
            // Remove node which hasn't this attr.
            return true;
        }
    }), tags.end());
}


} // namespace Gumbo

} // namespace Utils