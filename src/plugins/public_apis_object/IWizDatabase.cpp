#include "IWizDatabase.h"

IWizDatabase::IWizDatabase(WizDatabaseManager* dbManager, QObject* parent)
    : QObject(parent)
    , m_dbManager(dbManager)
{

}
