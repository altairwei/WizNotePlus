#ifndef WIZFILEREADER_H
#define WIZFILEREADER_H

#include "share/WizDatabaseManager.h"

struct WIZTAGDATA;

class WizFileImporter : public QObject
{
    Q_OBJECT
public:
    explicit WizFileImporter(WizDatabaseManager& dbMgr,QObject *parent = nullptr);

    void importFiles(const QStringList& strFiles, const QString& strTargetFolderLocation);
    void importFiles(const QStringList& strFiles, const QString& strKbGUID, const WIZTAGDATA& tag);
    void importFiles(const QStringList& strFiles, const QString& strKbGUID, const QString& strTargetFolderLocation, const WIZTAGDATA& tag);

    static QString loadHtmlFileToHtml(const QString& strFileName);
    QString loadTextFileToHtml(const QString& strFileName);
    static QString loadTextFileToHtml(const QString& strFileName, bool isUTF8);
    QString loadImageFileToHtml(const QString& strFileName);
    //
signals:
    void importFinished(bool ok, const QString& text, const QString& kbGuid);
    void importProgress(int total,int loaded);


private:
    bool importFile(const QString& strFile, const QString& strKbGUID, const QString& strLocation, const WIZTAGDATA& tag);

private:
    WizDatabaseManager& m_dbMgr;
    QString m_strKbGuid;
};

#endif // WIZFILEREADER_H
