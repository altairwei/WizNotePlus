#ifndef DATABASE_WIZONLINEDATABASE_H
#define DATABASE_WIZONLINEDATABASE_H

#include "WizSyncableDatabase.h"
#include "WizDatabase.h"

class WizOnlineDatabase
        : public WizDatabase
        , public IWizSyncableDatabase
{

};

#endif // DATABASE_WIZONLINEDATABASE_H