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
#include "html/WizHtmlTool.h"

#define GUIDLEN 38

WizFileExporter::WizFileExporter(WizDatabaseManager& dbMgr, QObject *parent)
    : QObject(parent)
    , m_dbMgr(dbMgr)
    , m_errMsg("Unknown error")
    , m_compress(false)
    , m_exportMetaInfo(true)
    , m_noTitleFolderIfPossible(false)
    , m_handleRichTextInMarkdown(false)
    , m_convertRichTextToMarkdown(false)
{

}

bool WizFileExporter::exportNote(
    const WIZDOCUMENTDATA &doc,
    const QString &destFolder,
    const ExportFormat format)
{
    WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
    if (!WizMakeSureDocumentExistAndBlockWidthEventloop(db, doc)) {
        m_errMsg = "Can't download document: " + doc.strTitle;
        return false;
    }

    CString folder = doc.strTitle;
    WizMakeValidFileNameNoPath(folder);

    QDir docFolder(destFolder);
    if (!docFolder.mkpath(folder)) {
        m_errMsg = "Can't make directory: " + docFolder.filePath(folder);
        return false;
    }
    docFolder.cd(folder);

    // Write meta info
    if (m_exportMetaInfo) {
        if (!writeDocumentInfoToJsonFile(doc, docFolder.filePath("metainfo.json"))) {
            m_errMsg = "Can't save meta info to json file: " +
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
            m_errMsg = "Can't export attachment: " + att.strName.remove(0, GUIDLEN);
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
        m_errMsg = "Can't unzip document to " + docFolder.absolutePath();
        return false;
    }

    if (m_compress) {
        if (!compressDocumentFolder(docFolder.absolutePath())) {
            m_errMsg = "Can't remove folder after compress";
            return false;
        }
    }

    // Output compressing requires the title folder
    if (!m_compress && m_noTitleFolderIfPossible) {
        QFileInfoList entries = docFolder.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        if (entries.length() != 1)
            return true;

        QDir dest = docFolder;
        dest.cdUp();

        QFileInfo &indexFile = entries.first();
        QString suffix = indexFile.suffix();
        QString destFileName = folder.endsWith(suffix) ? folder : folder + "." + suffix;

        m_errMsg = "Error for noTitleFolderIfPossible";
        if (!QFile::rename(indexFile.absoluteFilePath(),
                           dest.absoluteFilePath(indexFile.fileName())))
            return false;
        if (!docFolder.removeRecursively())
            return false;
        if (!dest.rename(indexFile.fileName(), destFileName))
            return false;
        m_errMsg.clear();
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
    QString rawHtml = htmlContent;
    rawHtml = removeHiddenImageTags(rawHtml);
    if (m_handleRichTextInMarkdown) {
        rawHtml = mixedImageToMarkdown(rawHtml);
        rawHtml = mixedTableToMarkdown(rawHtml);
    }

    QTextDocument doc;
    doc.setHtml(rawHtml);
    QString strText = doc.toPlainText();
    // This is a remedy of a long-term BUG which escapes empties erroneously.
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

bool WizFileExporter::compressDocumentFolder(const QString &folder, bool removeSource)
{
    if (!JlCompress::compressDir(folder + ".zip", folder, true))
        return false;

    if (removeSource && !QDir(folder).removeRecursively())
        return false;

    return true;
}

QString WizFileExporter::mixedImageToMarkdown(const QString &html)
{
    auto convImg = [](
            const QMap<QString, QString> &attrs,
            const QString &) -> QString {
        QString imgStr = "![";
        if (attrs.contains("alt"))
            imgStr.append(attrs["alt"]);
        imgStr.append("](");
        if (attrs.contains("src"))
            imgStr.append(attrs["src"]);
        if (attrs.contains("title")) {
            imgStr.append(" \"");
            imgStr.append(attrs["src"]);
            imgStr.append("\"");
        }
        imgStr.append(")");
        return imgStr;
    };

    return Utils::WizReplaceTagsWithText(html, convImg, "img");
}

QString WizFileExporter::mixedTableToMarkdown(const QString &html)
{
    auto convTable = [](
            const QMap<QString, QString> &attrs,
            const QString &tagHtml) -> QString {
        // TODO: format html string
        return tagHtml.toHtmlEscaped();
    };

    return Utils::WizReplaceTagsWithText(html, convTable, "table");
}

QString WizFileExporter::removeHiddenImageTags(const QString &html)
{
    auto fun = [](
            const QMap<QString, QString> &,
            const QString &) -> QString {
        return "";
    };

    // This tag was introduced by Wiz.Editor.md plugin for
    // compatibility to WizNote Windows-specific client.
    return Utils::WizReplaceTagsWithText(
                html, fun, "ed_tag", "name", "markdownimage");
}
