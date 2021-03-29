#include "ApiWizDatabase.h"

#include <QDebug>

#include "share/WizMisc.h"
#include "database/WizDatabaseManager.h"
#include "database/WizDatabase.h"

#include "WizFileImporter.h"


ApiWizDatabase::ApiWizDatabase(WizDatabaseManager* dbManager, QObject* parent)
    : QObject(parent)
    , m_dbManager(dbManager)
{

}

QObject *ApiWizDatabase::Database()
{
    return &(m_dbManager->db());
}

QObject *ApiWizDatabase::GetGroupDatabase(const QString &kbGUID)
{
    return &(m_dbManager->db(kbGUID));
}

bool ApiWizDatabase::CreateDocument(const QString &bstrIndexFileName, const QString &bstrTitle, 
    const QString &bstrLocation, const QStringList &tagList, const QString &bstrURL)
{
    WizDatabase &db = m_dbManager->db();
    // Process meta data
    WIZDOCUMENTDATA docData;
    QString strHtml = WizFileImporter::loadHtmlFileToHtml(bstrIndexFileName, "UTF-8");
    QString location = bstrLocation;
    if (location.isEmpty())
        location = db.getDefaultNoteLocation();
    // Create document
    bool bRet = db.createDocumentAndInit(
        strHtml, bstrIndexFileName, 0, bstrTitle, "newnote", location, bstrURL, docData);
    if (!bRet)
        return bRet;
    // Add tags
    WizDocument doc(db, docData);
    for (QString strTagName : tagList) {
        // only create tag for unique name
        WIZTAGDATA tag;
        CWizTagDataArray arrayTag;
        db.tagByName(strTagName, arrayTag);
        for (WIZTAGDATA tagItem : arrayTag)
        {
            if (!tagItem.strGUID.isEmpty())
            {
                tag = tagItem;
                break;
            }
        }
        if (!tag.strGUID.isEmpty())
        {
            qInfo() << QString("Tag name already exist: %1").arg(strTagName);
             doc.addTag(tag);
        }
        else
        {
            db.createTag("", strTagName, "", tag);
            doc.addTag(tag);
        }
    }

    return bRet;
}

bool ApiWizDatabase::CheckDocumentData(QObject *pDoc)
{
    WizDocument *doc = qobject_cast<WizDocument *>(pDoc);
    if (doc) {
        return WizMakeSureDocumentExistAndBlockWidthDialog(doc->db(), doc->data());
    } else {
        return false;
    }
}