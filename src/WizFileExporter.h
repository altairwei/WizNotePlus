//
// Created by pikachu on 2/16/2022.
//

#ifndef WIZNOTEPLUS_WIZFILEEXPORTER_H
#define WIZNOTEPLUS_WIZFILEEXPORTER_H

#include <QObject>

class WizDatabaseManager;
class WIZDOCUMENTDATA;

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
                    QString *errorMsg = nullptr);

private:
    bool extractMarkdownToFile(const QString &htmlContent, const QString &outputFile);
    bool writeDocumentInfoToJsonFile(const WIZDOCUMENTDATA &doc, const QString &outputFIle);

private:
    WizDatabaseManager& m_dbMgr;
};


#endif //WIZNOTEPLUS_WIZFILEEXPORTER_H
