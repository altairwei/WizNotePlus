//
// Created by pikachu on 2/16/2022.
//

#include "WizFileExporter.h"

#include <QFile>
#include <QTextDocument>
#include <QDir>
#include <QDebug>

#include "database/WizDatabase.h"
#include "database/WizDatabaseManager.h"
#include "share/WizMisc.h"

WizFileExporter::WizFileExporter(WizDatabaseManager& dbMgr, QObject *parent)
    : QObject(parent)
    , m_dbMgr(dbMgr)
{

}

bool WizFileExporter::exportNote(
    const WIZDOCUMENTDATA &doc,
    const QString &destFolder,
    const ExportFormat format,
    bool compress /*= false*/
)
{
    WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
    if (!WizMakeSureDocumentExistAndBlockWidthEventloop(db, doc)) {
        return false;
    }

    // TODO: handle cipher, not supported now

    CString folder = doc.strTitle;
    WizMakeValidFileNameNoPath(folder);
    QDir docFolder(destFolder);
    if (!docFolder.exists())
        return false;
    docFolder.mkpath(folder);
    docFolder.cd(folder);

    // TODO: write meta info to meta.json
    // TODO: unzip index_files/ and index.html

    QString strHtmlFile = docFolder.filePath("index.html");
    if (db.documentToHtmlFile(doc, docFolder.absolutePath() + "/")) {
        QFile file(strHtmlFile);
        if (!file.open(QIODevice::ReadOnly)) {
            return false;
        }
        QString strHtml = file.readAll();
        file.close();

        switch (format) {
        case Markdown:
        {
            QString outputFilePath = docFolder.filePath("index.md");
            if (!extractMarkdownToFile(strHtml, outputFilePath))
                return false;
            docFolder.remove("index.html");
            break;
        }
        case HTML:
        {

        }
        default:
            break;
        }

    } else {
        qWarning() << "Can't unzip document to " << docFolder.absolutePath();
        return false;
    }

    return true;
}

bool WizFileExporter::extractMarkdownToFile(const QString &htmlContent, const QString &outputFile) {
    QTextDocument doc;
    doc.setHtml(htmlContent);
    QString strText = doc.toPlainText();
    strText = strText.replace("&nbsp", " ");

    // write text to file
    if (!WizSaveUnicodeTextToUtf8File(outputFile, strText)) {
        return false;
    }

    return true;
}
