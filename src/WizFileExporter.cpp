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
#include "share/jsoncpp/json/json.h"

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
        qWarning() << "Can't download document: " << doc.strTitle;
        return false;
    }

    // TODO: handle cipher, not supported now

    CString folder = doc.strTitle;
    WizMakeValidFileNameNoPath(folder);

    QDir docFolder(destFolder);
    if (!docFolder.mkpath(folder)) {
        qWarning() << "Can't make directory: " << docFolder.filePath(folder);
        return false;
    }
    docFolder.cd(folder);

    // Write meta info
    if (!writeDocumentInfoToJsonFile(doc, docFolder.filePath("metainfo.json"))) {
        qWarning() << "Can't save meta info to json file: " << docFolder.filePath("metainfo.json");
    }

    // Unzip index.html and index_files/
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

bool WizFileExporter::writeDocumentInfoToJsonFile(const WIZDOCUMENTDATA &doc, const QString &outputFIle)
{
    Json::Value metainfo;
    WIZDOCUMENTDATAEX data(doc);
    if (!data.toJson(metainfo)) {
        return false;
    }

    // Convert tagGUID to tag text
    WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
    metainfo["tags"] = db.getDocumentTagDisplayNameText(doc.strGUID).toStdString();

    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "\t";
    QString metainfoDocument = QString::fromStdString(Json::writeString(wbuilder, metainfo));
    if (!WizSaveUnicodeTextToUtf8File(outputFIle, metainfoDocument)) {
        return false;
    }

    return true;
}
