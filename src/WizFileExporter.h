//
// Created by pikachu on 2/16/2022.
//

#ifndef WIZNOTEPLUS_WIZFILEEXPORTER_H
#define WIZNOTEPLUS_WIZFILEEXPORTER_H

#include <QObject>
class WizDatabaseManager;
class WizFileExporter : public QObject {
Q_OBJECT
public:
    explicit WizFileExporter(WizDatabaseManager& dbMgr,QObject *parent = nullptr);
private:
};


#endif //WIZNOTEPLUS_WIZFILEEXPORTER_H
