#include "IWizDatabase.h"
#include "share/WizDatabaseManager.h"
#include "share/WizDatabase.h"
#include "WizFileImporter.h"

IWizDatabase::IWizDatabase(WizDatabaseManager* dbManager, QObject* parent)
    : QObject(parent)
    , m_dbManager(dbManager)
{

}

QObject *IWizDatabase::Database()
{
    return &(m_dbManager->db());
}

bool IWizDatabase::CreateDocument(const QString &bstrIndexFileName, const QString &bstrTitle, const QString &bstrLocation, const QString &bstrURL)
{
    WizDatabase &db = m_dbManager->db();
    // Process meta data
    WIZDOCUMENTDATA doc;
    QString strHtml = WizFileImporter::loadHtmlFileToHtml(bstrIndexFileName, true);
    QString location = bstrLocation;
    if (location.isEmpty())
        location = db.getDefaultNoteLocation();
    // Create document
    bool bRet = db.createDocumentAndInit(
        strHtml, bstrIndexFileName, 0, bstrTitle, "newnote", location, bstrIndexFileName, doc);

    return bRet;
}