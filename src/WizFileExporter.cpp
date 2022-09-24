//
// Created by pikachu on 2/16/2022.
//

#include "WizFileExporter.h"

#include <QFile>
#include <QFileInfo>
#include <QTextDocument>
#include <QDir>
#include <QDebug>
#include <quazip/JlCompress.h>

#include "database/WizDatabase.h"
#include "database/WizDatabaseManager.h"
#include "share/WizMisc.h"
#include "share/jsoncpp/json/json.h"
#include "share/WizObject.h"

#define GUIDLEN 38

WizFileExporter::WizFileExporter(WizDatabaseManager& dbMgr, QObject *parent)
    : QObject(parent)
    , m_dbMgr(dbMgr)
{

}

bool WizFileExporter::exportNote(
    const WIZDOCUMENTDATA &doc,
    const QString &destFolder,
    const ExportFormat format,
    bool compress /*= false*/,
    bool exportMetaInfo /*= true*/,
    bool noTitleFolderIfPossible /*= false*/,
    QString *errorMsg /*= nullptr*/)
{
    WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
    if (!WizMakeSureDocumentExistAndBlockWidthEventloop(db, doc)) {
        if (errorMsg)
            *errorMsg = "Can't download document: " + doc.strTitle;
        return false;
    }

    CString folder = doc.strTitle;
    WizMakeValidFileNameNoPath(folder);

    QDir docFolder(destFolder);
    if (!docFolder.mkpath(folder)) {
        if (errorMsg)
            *errorMsg = "Can't make directory: " +
                docFolder.filePath(folder);
        return false;
    }
    docFolder.cd(folder);

    // Write meta info
    if (exportMetaInfo) {
        if (!writeDocumentInfoToJsonFile(doc, docFolder.filePath("metainfo.json"))) {
            if (errorMsg)
                *errorMsg =  "Can't save meta info to json file: " +
                    docFolder.filePath("metainfo.json");
            return false;
        }
    }

    // Export attachments, download if neccessary
    CWizDocumentAttachmentDataArray arrayAttachment;
    db.getDocumentAttachments(doc.strGUID, arrayAttachment);
    if (!arrayAttachment.empty())
        docFolder.mkpath("attachments");
    for (auto &att : arrayAttachment) {
        if (!exportAttachment(att, docFolder.filePath("attachments"))) {
            if (errorMsg)
                *errorMsg = "Can't export attachment: " +
                    att.strName.remove(0, GUIDLEN);
            return false;
        }
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
        default:
            break;
        }

    } else {
        if (errorMsg)
            *errorMsg = "Can't unzip document to " +
                docFolder.absolutePath();
        return false;
    }

    if (compress) {
        if (!compressDocumentFolder(docFolder.absolutePath())) {
            if (errorMsg) *errorMsg = "Can't remove folder after compress";
        }
    }

    if (!compress && noTitleFolderIfPossible) {
        QFileInfoList entries = docFolder.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        if (entries.length() != 1)
            return true;

        QDir dest = docFolder;
        dest.cdUp();

        QFileInfo &indexFile = entries.first();
        QString suffix = indexFile.suffix();
        QString destFileName = folder.endsWith(suffix) ? folder : folder + "." + suffix;

        if (errorMsg) *errorMsg = "Error for noTitleFolderIfPossible";
        if (!QFile::rename(indexFile.absoluteFilePath(),
                           dest.absoluteFilePath(indexFile.fileName())))
            return false;
        if (!docFolder.removeRecursively())
            return false;
        if (!dest.rename(indexFile.fileName(), destFileName))
            return false;
        if (errorMsg) errorMsg->clear();
    }

    return true;
}

bool WizFileExporter::exportAttachment(
        const WIZDOCUMENTATTACHMENTDATAEX &att,
        const QString &destFolder)
{
    WizDatabase& db = m_dbMgr.db(att.strKbGUID);
    bool bIsLocal = db.isObjectDataDownloaded(att.strGUID, "attachment");
    QString strFileName = db.getAttachmentFileName(att.strGUID);
    bool bExists = WizPathFileExists(strFileName);
    if (!bIsLocal || !bExists) {
        bool ok = WizMakeSureAttachmentExistAndBlockWidthEventloop(db, att);
        if (!ok) return false;
    }

    QFileInfo attfile(strFileName);
    QString newFilename = destFolder + "/" + attfile.fileName().remove(0, GUIDLEN);

    if (QFile::exists(newFilename))
        QFile::remove(newFilename);

    return QFile::copy(attfile.absoluteFilePath(), newFilename);
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

bool WizFileExporter::compressDocumentFolder(const QString &folder, bool removeSource) {
    if (!JlCompress::compressDir(folder + ".zip", folder, true))
        return false;

    if (removeSource && !QDir(folder).removeRecursively())
        return false;

    return true;
}
