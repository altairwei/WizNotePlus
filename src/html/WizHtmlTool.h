#ifndef HTML_WIZHTMLTOOL_H
#define HTML_WIZHTMLTOOL_H

#include <QString>
#include <QStringList>

namespace Utils {

    QStringList WizHtmlExtractTags(
        const QString &htmlText,
        const QString &tagName,
        const QString &tagAttributeName = "",
        const QString &tagAttributeValue = "");
}

#endif // HTML_WIZHTMLTOOL_H