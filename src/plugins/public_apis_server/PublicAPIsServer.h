#ifndef PUBLICAPISSERVER_H
#define PUBLICAPISSERVER_H

#include <QObject>
#include <QHash>
#include <QString>

class QWebSocketServer;
class QWebChannel;
class WebSocketClientWrapper;
class QWebSocketCorsAuthenticator;

class PublicAPIsServer : public QObject
{
    Q_OBJECT

public:
    PublicAPIsServer(QHash<QString, QObject *> publicObjectCollection, QObject *parent = nullptr);

private:
    QWebSocketServer *m_server;
    QWebChannel *m_channel;
    WebSocketClientWrapper *m_clientWrapper;

public slots:
    void handleAuthentication(QWebSocketCorsAuthenticator *authenticator);
};

#endif // PUBLICAPISSERVER_H