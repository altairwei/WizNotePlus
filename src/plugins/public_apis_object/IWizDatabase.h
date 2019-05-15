#ifndef IWIZDATABASE_H
#define IWIZDATABASE_H

#include <QObject>

class WizDatabaseManager;

class IWizDatabase : public QObject
{
    Q_OBJECT

private:
    WizDatabaseManager* m_dbManager;

public:
    IWizDatabase(WizDatabaseManager* dbManager, QObject* parent);
};

#endif // WIZDATABASE_H
