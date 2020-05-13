#include "WizUpgrade.h"

#include <QJsonArray>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "share/WizEventLoop.h"

#define RELEASE_URL "https://api.github.com/repos/altairwei/WizNotePlus/releases"

WizUpgradeChecker::WizUpgradeChecker(QObject *parent) :
    QObject(parent)
{
}

WizUpgradeChecker::~WizUpgradeChecker()
{
}

void WizUpgradeChecker::setTagName(const QString &tagName)
{
    m_tagName = tagName;
}

QString WizUpgradeChecker::getWhatsNewUrl()
{
    //return WizApiEntry::standardCommandUrl("changelog");
    return QString();
}

void WizUpgradeChecker::checkUpgrade()
{
    QString releaseUrl = RELEASE_URL;

    if (!m_net.get()) {
        m_net = std::make_shared<QNetworkAccessManager>();
    }

    // Query release information
    QNetworkReply* reply = m_net->get(QNetworkRequest(releaseUrl));
    WizAutoTimeOutEventLoop loop(reply);
    loop.exec();

    if (loop.error() != QNetworkReply::NoError)
    {
        Q_EMIT checkFinished({}, {});
        return;
    }

    // Get JSON string of result
    QJsonDocument jsonResult = QJsonDocument::fromJson(loop.result().constData());
    findNewerReleases(jsonResult);

}

/**
 * @brief Find out the newer releases, and notify latest release.
 * 
 * @param jsonResult The response of GitHub release API.
 * @return int The count of newer releases.
 */
int WizUpgradeChecker::findNewerReleases(const QJsonDocument &jsonResult)
{
    QJsonArray allReleases = jsonResult.array();
    if (allReleases.isEmpty()) {
        Q_EMIT checkFinished({}, {});
        return 0;
    }

    const QString currentTagName = m_tagName;
    
    QJsonObject latestStable;
    QJsonObject latestTest;
    int newerReleaseCount = 0;

    // First object of the array is the latest release.
    for (const QJsonValue &relObj : allReleases) {
        const QJsonObject release = relObj.toObject();

        // Skip unknown object
        if (release.isEmpty() || 
                !release["tag_name"].isString() ||
                !release["prerelease"].isBool())
            continue;

        QString tagName = release["tag_name"].toString();
        
        // Stop if release is not newer than current one.
        if (compareTagName(tagName, currentTagName) != 1)
            break;

        newerReleaseCount++;
        // "prerelease == true" is a testing release
        if (release["prerelease"].toBool()) {
            if (latestTest.isEmpty())
                latestTest = release;
        } else {
            if (latestStable.isEmpty())
                latestStable = release;
        }

    }

    Q_EMIT checkFinished(latestStable, latestTest);

    return newerReleaseCount;
}

/**
 * @brief Compare two version numbers version1 and version2.
 * 
 * @param v1 Version string, such as "2.8.1", only three parts are allowed.
 * @param v2 Version string, such as "2.7.3", only three parts are allowed..
 * @return int If version1 > version2 return 1; if version1 < version2 return -1;otherwise return 0.
 */
int WizUpgradeChecker::compareVersion(const QString &v1, const QString &v2)
{
    QStringList v1Parts = v1.split(".");
    int v1Major = v1Parts.at(0).toInt();
    int v1Minor = v1Parts.at(1).toInt();
    int v1Patch = v1Parts.at(2).toInt();

    QStringList v2Parts = v2.split(".");
    int v2Major = v2Parts.at(0).toInt();
    int v2Minor = v2Parts.at(1).toInt();
    int v2Patch = v2Parts.at(2).toInt();

    if (v1Major > v2Major) 
        return 1;
    else if (v1Major < v2Major) 
        return -1;

    if (v1Minor > v2Minor) 
        return 1;
    else if (v1Minor < v2Minor)
        return -1;

    if (v1Patch > v2Patch)
        return 1;
    else if (v1Patch < v2Patch)
        return -1;
    else
        return 0;
}

/**
 * @brief Compare two developing stage stage1 and stage2.
 * 
 * @param s1 Developing stage, see enum DevStage.
 * @param s2 Developing stage, see enum DevStage.
 * @return int If version1 > version2 return 1; if version1 < version2 return -1;otherwise return 0.
 */
int WizUpgradeChecker::compareDevStage(const QString &s1, const QString &s2)
{
    int s1Order = m_devStage[s1.toLower()];
    int s2Order = m_devStage[s2.toLower()];

    if (s1Order > s2Order)
        return 1;
    else if (s1Order < s2Order)
        return -1;
    else
        return 0;
}

/**
 * @brief Compare two tag name tag1 and tag2.
 * 
 * @param tag1 Such as "v2.8.0-beta.1"
 * @param tag2 Such as "v2.7.3-alpha.2"
 * @return int 
 */
int WizUpgradeChecker::compareTagName(const QString &tag1, const QString &tag2)
{
    const QString tagNamePattern = "^v(?<version>\\d+\\.\\d+\\.\\d+)(|-(?<devStage>\\w+)\\.(?<devStageVer>\\d+))$";

    QRegularExpressionMatch tag1Parts = QRegularExpression(tagNamePattern).match(tag1);
    if (!tag1Parts.hasMatch()) {
        qWarning() << QString("WizUpgradeChecker: '%1' is not a valid tagName!").arg(tag1);
        return 0;
    }
    QString tag1Version = tag1Parts.captured("version");
    QString tag1DevStage = tag1Parts.captured("devStage");
    if (tag1DevStage.isEmpty())
        tag1DevStage = "stable";
    QString tag1DevStageVersion = tag1Parts.captured("devStageVer");
    if (tag1DevStageVersion.isEmpty())
        tag1DevStageVersion = "0";

    QRegularExpressionMatch tag2Parts = QRegularExpression(tagNamePattern).match(tag2);
    if (!tag2Parts.hasMatch()) {
        qWarning() << QString("WizUpgradeChecker: '%1' is not a valid tagName!").arg(tag2);
        return 0;
    }
    QString tag2Version = tag2Parts.captured("version");
    QString tag2DevStage = tag2Parts.captured("devStage");
    if (tag2DevStage.isEmpty())
        tag2DevStage = "stable";
    QString tag2DevStageVersion = tag2Parts.captured("devStageVer");
    if (tag2DevStageVersion.isEmpty())
        tag2DevStageVersion = "0";

    // Main version
    if (compareVersion(tag1Version, tag2Version) == 1)
        return 1;
    else if (compareVersion(tag1Version, tag2Version) == -1)
        return -1;

    // Developing stage
    if (compareDevStage(tag1DevStage, tag2DevStage) == 1)
        return 1;
    else if (compareDevStage(tag1DevStage, tag2DevStage) == -1)
        return -1;

    // Developing stage version
    if (tag1DevStageVersion.toInt() > tag2DevStageVersion.toInt())
        return 1;
    else if (tag1DevStageVersion.toInt() < tag2DevStageVersion.toInt())
        return -1;
    else
        return 0;
}

QUrl WizUpgradeChecker::redirectUrl(QUrl const &possible_redirect_url, \
                              QUrl const &old_redirect_url) const
{
    QUrl redirect_url;

    if(!possible_redirect_url.isEmpty() \
            && possible_redirect_url != old_redirect_url)
    {
            redirect_url = possible_redirect_url;
    }

    return redirect_url;
}
