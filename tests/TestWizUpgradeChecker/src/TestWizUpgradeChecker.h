#ifndef TESTWIZUPGRADECHECKER_H
#define TESTWIZUPGRADECHECKER_H

#include <QtTest/QtTest>
#include <QObject>

#include "src/WizUpgrade.h"

class TestWizUpgradeChecker : public QObject
{

    Q_OBJECT

private slots:
    void checkCompareVersion();
    void checkCompareVersion_data();
    void checkCompareDevStage();
    void checkCompareDevStage_data();
    void checkCompareTagName();
    void checkCompareTagName_data();
    void checkFindNewerReleases();
    void checkFindNewerReleases_data();
};

#endif // TESTWIZUPGRADECHECKER_H