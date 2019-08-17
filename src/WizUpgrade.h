#ifndef WIZUPGRADE_H
#define WIZUPGRADE_H

#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <memory>


class WizUpgradeChecker : public QObject
{
    Q_OBJECT

public:

    enum DevStage { Alpha, Beta, ReleaseCandidate, Stable };
    explicit WizUpgradeChecker(QObject *parent = 0);
    ~WizUpgradeChecker();
    void setTagName(const QString &tagName);
    void startCheck();

    static QString getWhatsNewUrl();

    void checkUpgrade();

    int compareVersion(const QString &v1, const QString &v2);
    int compareDevStage(const QString &s1, const QString &s2);
    int compareTagName(const QString &tag1, const QString &tag2);
    int findNewerReleases(const QJsonDocument &jsonResult);

Q_SIGNALS:
    void checkFinished(QJsonObject latestStable, QJsonObject latestTest);


private:
    QString m_tagName; /** Current version */
    std::shared_ptr<QNetworkAccessManager> m_net;
    QUrl m_redirectedUrl;
    QHash<QString, DevStage> m_devStage = {
        {"alpha", Alpha},
        {"beta", Beta},
        {"rc", ReleaseCandidate},
        {"stable", Stable}
    };

    QUrl redirectUrl(QUrl const &possible_redirect_url,
                     QUrl const &old_redirect_url) const;
};

#endif // WIZUPGRADE_H
