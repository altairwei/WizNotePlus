#ifndef IWIZDATABASE_H
#define IWIZDATABASE_H

#include <QObject>

class WizDatabaseManager;
class WizDatabase;

class IWizDatabase : public QObject
{
    Q_OBJECT

private:
    WizDatabaseManager* m_dbManager;

public:
    IWizDatabase(WizDatabaseManager* dbManager, QObject* parent);

    Q_INVOKABLE QObject *Database();
};

#endif // WIZDATABASE_H
