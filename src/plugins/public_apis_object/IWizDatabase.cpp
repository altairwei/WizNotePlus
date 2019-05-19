#include "IWizDatabase.h"
#include "share/WizDatabaseManager.h"
#include "share/WizDatabase.h"

IWizDatabase::IWizDatabase(WizDatabaseManager* dbManager, QObject* parent)
    : QObject(parent)
    , m_dbManager(dbManager)
{

}

QObject *IWizDatabase::Database()
{
    return &(m_dbManager->db());
}