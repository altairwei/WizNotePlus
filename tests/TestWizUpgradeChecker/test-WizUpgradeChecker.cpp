#include "test-WizUpgradeChecker.h"

#include <QString>
#include <QTest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSignalSpy>
#include <QList>

QTEST_MAIN(TestWizUpgradeChecker)

static WizUpgradeChecker checker;

void TestWizUpgradeChecker::checkCompareVersion()
{
    QFETCH(QString, version1);
    QFETCH(QString, version2);
    QFETCH(int, result);

    QCOMPARE(checker.compareVersion(version1, version2), result);
}

void TestWizUpgradeChecker::checkCompareVersion_data()
{
    QTest::addColumn<QString>("version1");
    QTest::addColumn<QString>("version2");
    QTest::addColumn<int>("result");

    QTest::newRow("Version check test-1") << "2.7.3" << "2.8.0" << -1 ;
    QTest::newRow("Version check test-2") << "2.8.3" << "2.8.0" << 1 ;
    QTest::newRow("Version check test-3") << "2.8.0" << "2.8.0" << 0 ;
}

void TestWizUpgradeChecker::checkCompareDevStage()
{
    QFETCH(QString, stage1);
    QFETCH(QString, stage2);
    QFETCH(int, result);
    QCOMPARE(checker.compareDevStage(stage1, stage2), result);
}

void TestWizUpgradeChecker::checkCompareDevStage_data()
{
    QTest::addColumn<QString>("stage1");
    QTest::addColumn<QString>("stage2");
    QTest::addColumn<int>("result");

    QTest::newRow("DevStage check test-1") << "alpha"  << "beta"   << -1 ;
    QTest::newRow("DevStage check test-2") << "rc"     << "beta"   << 1 ;
    QTest::newRow("DevStage check test-3") << "stable" << "beta"   << 1 ;
    QTest::newRow("DevStage check test-4") << "stable" << "stable" << 0 ;
    QTest::newRow("DevStage check test-5") << "rc"     << "rc"     << 0 ;
    QTest::newRow("DevStage check test-6") << "rc"     << "stable" << -1 ;
}

void TestWizUpgradeChecker::checkCompareTagName()
{
    QFETCH(QString, tag1);
    QFETCH(QString, tag2);
    QFETCH(int, result);
    QCOMPARE(checker.compareTagName(tag1, tag2), result);
}

void TestWizUpgradeChecker::checkCompareTagName_data()
{
    QTest::addColumn<QString>("tag1");
    QTest::addColumn<QString>("tag2");
    QTest::addColumn<int>("result");

    QTest::newRow("TagName check: full tag 1")      << "v2.7.3-beta.1"  << "v2.8.0-beta.0"  << -1 ;
    QTest::newRow("TagName check: full tag 2")      << "v2.8.0-beta.1"  << "v2.8.0-beta.0"  << 1 ;
    QTest::newRow("TagName check: full tag 3")      << "v2.8.0-beta.1"  << "v2.8.0-beta.1"  << 0 ;
    QTest::newRow("TagName check: short tag 1")     << "v2.7.0"         << "v2.8.0-beta.1"  << -1 ;
    QTest::newRow("TagName check: short tag 2")     << "v2.7.3"         << "v2.8.0"         << -1 ;
    QTest::newRow("TagName check: short tag 3")     << "v2.8.3"         << "v2.8.0"         << 1 ;
    QTest::newRow("TagName check: short tag 4")     << "v2.8.3-alpha.3" << "v2.8.0"         << 1 ;
    QTest::newRow("TagName check: short tag 5")     << "v2.7.3-beta.2"  << "v2.8.0"         << -1 ;
    QTest::newRow("TagName check: illegal tag 1")   << "wrong version"  << "v2.8.0"         << 0 ;
    QTest::newRow("TagName check: illegal tag 2")   << "v2.8.0"         << "wrong versoin"  << 0 ;
}

void TestWizUpgradeChecker::checkFindNewerReleases()
{
    QFETCH(QString, currentTagName);
    QFETCH(QJsonDocument, response);
    QFETCH(int, count);
    QFETCH(QJsonObject, latestStable);
    QFETCH(QJsonObject, latestTest);

    WizUpgradeChecker *checker = new WizUpgradeChecker();

    checker->setTagName(currentTagName);
    QSignalSpy spy(checker, &WizUpgradeChecker::checkFinished);
    int newerCount = checker->findNewerReleases(response);

    QCOMPARE(newerCount, count);

    QCOMPARE(spy.count(), 1);
    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toJsonObject(), latestStable);
    QCOMPARE(args.at(1).toJsonObject(), latestTest);

    delete checker;
}

void TestWizUpgradeChecker::checkFindNewerReleases_data()
{
    QTest::addColumn<QString>("currentTagName");
    QTest::addColumn<QJsonDocument>("response");
    QTest::addColumn<int>("count");
    QTest::addColumn<QJsonObject>("latestStable");
    QTest::addColumn<QJsonObject>("latestTest");

    QJsonArray releases;
    QJsonObject release0 = {{"tag_name", "v2.8.2-beta.0"}   , {"prerelease", true}};
    releases.append(release0);
    QJsonObject release1 = {{"tag_name", "v2.8.1"}          , {"prerelease", false}};
    releases.append(release1);
    QJsonObject release2 = {{"tag_name", "v2.8.0-beta.1"}   , {"prerelease", true}};
    releases.append(release2);
    QJsonObject release3 = {{"tag_name", "v2.8.0-beta.0"}   , {"prerelease", true}};
    releases.append(release3);
    QJsonObject release4 = {{"tag_name", "v2.7.3"}          , {"prerelease", false}};
    releases.append(release4);

    QJsonDocument response;
    response.setArray(releases);
    QString currentTagName1 = QString("v%1-%2.%3").arg("2.7.3").arg("stable").arg(0);
    QString currentTagName2 = QString("v%1-%2.%3").arg("2.8.1").arg("stable").arg(0);
    QString currentTagName3 = QString("v%1-%2.%3").arg("2.8.0").arg("beta").arg(0);

    QTest::newRow("Check GitHub APIs 1") << currentTagName1 << response << 4 << release1        << release0;
    QTest::newRow("Check GitHub APIs 2") << currentTagName2 << response << 1 << QJsonObject()   << release0;
    QTest::newRow("Check GitHub APIs 3") << currentTagName3 << response << 3 << release1        << release0;
}