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
                    const ExportFormat format,
                    bool compress = false,
                    bool exportMetaInfo = true,
                    bool noTitleFolderIfPossible = false,
                    QString *errorMsg = nullptr);

    bool exportAttachment(const WIZDOCUMENTATTACHMENTDATAEX &att,
                          const QString &destFolder);

private:
    bool extractMarkdownToFile(const QString &htmlContent, const QString &outputFile);
    bool writeDocumentInfoToJsonFile(const WIZDOCUMENTDATA &doc, const QString &outputFIle);
    bool compressDocumentFolder(const QString &folder, bool removeSource = true);

private:
    WizDatabaseManager& m_dbMgr;
};


#endif //WIZNOTEPLUS_WIZFILEEXPORTER_H
