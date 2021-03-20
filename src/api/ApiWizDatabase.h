#ifndef API_APIWIZDATABASE_H
#define API_APIWIZDATABASE_H

#include <QObject>

class WizDatabaseManager;
class WizDatabase;
class WizDocument;

class ApiWizDatabase : public QObject
{
    Q_OBJECT

private:
    WizDatabaseManager* m_dbManager;

public:
    ApiWizDatabase(WizDatabaseManager* dbManager, QObject* parent);

    Q_INVOKABLE QObject *Database();
    Q_INVOKABLE bool CreateDocument(const QString &bstrIndexFileName, const QString &bstrTitle,
        const QString &bstrLocation, const QStringList &tagList, const QString &bstrURL);
    Q_INVOKABLE QObject *GetGroupDatabase(const QString &kbGUID);
    Q_INVOKABLE bool CheckDocumentData(QObject *doc);
};

#endif // API_APIWIZDATABASE_H
