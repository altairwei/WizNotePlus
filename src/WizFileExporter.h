//
// Created by pikachu on 2/16/2022.
//

#ifndef WIZNOTEPLUS_WIZFILEEXPORTER_H
#define WIZNOTEPLUS_WIZFILEEXPORTER_H

#include <QObject>

class WizDatabaseManager;
struct WIZDOCUMENTDATA;
struct WIZDOCUMENTATTACHMENTDATAEX;

class WizFileExporter : public QObject {

    Q_OBJECT

public:
    explicit WizFileExporter(WizDatabaseManager& dbMgr, QObject *parent = nullptr);

    enum ExportFormat {
        Markdown = 1 << 0,
        HTML     = 1 << 1,
        MHTML    = 1 << 2,
        PDF      = 1 << 3
    };

    bool exportNote(const WIZDOCUMENTDATA &doc,
                    const QString &destFolder,
                    const ExportFormat format);

    bool exportAttachment(const WIZDOCUMENTATTACHMENTDATAEX &att,
                          const QString &destFolder);

    QString errorMessage() { return m_errMsg; }
    void setCompress(bool b) { m_compress = b; }
    void setExportMetaInfo(bool b) { m_exportMetaInfo = b; }
    void setNoTitleFolderIfPossible(bool b) { m_noTitleFolderIfPossible = b; }
    void setHandleRichTextInMarkdown(bool b) { m_handleRichTextInMarkdown = b; }
    void setConvertRichTextToMarkdown(bool b) { m_convertRichTextToMarkdown = b; }

private:
    bool extractMarkdownToFile(const QString &htmlContent, const QString &outputFile);
    bool writeDocumentInfoToJsonFile(const WIZDOCUMENTDATA &doc, const QString &outputFIle);
    bool compressDocumentFolder(const QString &folder, bool removeSource = true);
    QString mixedImageToMarkdown(const QString &html);
    QString mixedTableToMarkdown(const QString &html);
    QString removeHiddenImageTags(const QString &html);

private:
    WizDatabaseManager& m_dbMgr;
    QString m_errMsg;
    bool m_compress;
    bool m_exportMetaInfo;
    bool m_noTitleFolderIfPossible;
    bool m_handleRichTextInMarkdown;
    bool m_convertRichTextToMarkdown;
};


#endif //WIZNOTEPLUS_WIZFILEEXPORTER_H
