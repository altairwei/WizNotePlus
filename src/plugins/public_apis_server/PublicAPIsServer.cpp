#include "PublicAPIsServer.h"
#include "WebSocketClientWrapper.h"
#include "WebSocketTransport.h"

#include <QWebSocketServer>
#include <QWebChannel>
#include <QWebSocketCorsAuthenticator>

PublicAPIsServer::PublicAPIsServer(QHash<QString, QObject *> publicObjectCollection, QObject *parent)
    : QObject(parent)
{
    // create websocket server
    m_server = new QWebSocketServer(
        QStringLiteral("WizNotePlus Public APIs Server"), QWebSocketServer::NonSecureMode, this);
    if (!m_server->listen(QHostAddress::LocalHost, 8848)) {
        qFatal("Failed to open web socket server.");
    }
    
    // create web channel
    m_channel = new QWebChannel(this);
    m_clientWrapper = new WebSocketClientWrapper(m_server, this);
    connect(m_clientWrapper, &WebSocketClientWrapper::clientConnected,
                     m_channel, &QWebChannel::connectTo);
    m_channel->registerObjects(publicObjectCollection);
    connect(m_server, &QWebSocketServer::originAuthenticationRequired,
                    this, &PublicAPIsServer::handleAuthentication);
}

void PublicAPIsServer::handleAuthentication(QWebSocketCorsAuthenticator *authenticator)
{
    qDebug() << "Remote request: " + authenticator->origin();
    authenticator->setAllowed(true);
}