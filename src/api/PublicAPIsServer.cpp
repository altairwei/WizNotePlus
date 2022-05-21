#include "PublicAPIsServer.h"
#include "WebSocketClientWrapper.h"
#include "WebSocketTransport.h"
#include "utils/WizPathResolve.h"

#include <QWebSocketServer>
#include <QWebChannel>
#include <QWebSocketCorsAuthenticator>
#include <QMessageBox>
#include <QFile>

#define ALLOWED_ORIGINS_FILENAME Utils::WizPathResolve::dataStorePath() + "allowedOrigins"

PublicAPIsServer::PublicAPIsServer(QHash<QString, QObject *> publicObjectCollection, QObject *parent)
    : QObject(parent)
{
    // create websocket server
    m_server = new QWebSocketServer(
        QStringLiteral("WizNotePlus Public APIs Server"), QWebSocketServer::NonSecureMode, this);

    if (!m_server->listen(QHostAddress::LocalHost, 8848)) {
        QMessageBox::critical(nullptr,
            tr("Abort"), tr("Unable to listen to port 8848, "
                            "browser webclipping plugin depends on it."));
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

    // Read allowed origins
    QFile oriFile(ALLOWED_ORIGINS_FILENAME);
    if (oriFile.open(QFile::ReadOnly)) {
        // Read to QStringList
        QTextStream textStream(&oriFile);
        while (true)
        {
            QString line = textStream.readLine();
            if (line.isNull())
                break;
            else
                m_allowedOrigins.append(line);
        }
    }
}

void PublicAPIsServer::handleAuthentication(QWebSocketCorsAuthenticator *authenticator)
{
    QString origin = authenticator->origin();
    qDebug() << "Remote APIs request: " + origin;

    // Check allowed origins
    for (QString allowedOrigin : m_allowedOrigins) {
        if (allowedOrigin == origin) {
            authenticator->setAllowed(true);
            return;
        }
    }

    // Prompt request to user
    QMessageBox ask;
    ask.setWindowTitle(tr("Remote APIs request"));
    ask.setText("There is a remote APIs request from: \n" + origin + 
        "\nDo you accept this request? or Save it to allowed origns list?");
    ask.addButton(QMessageBox::Yes);
    ask.addButton(QMessageBox::Save);
    ask.addButton(QMessageBox::No);
    ask.setDefaultButton(QMessageBox::No);
    ask.setWindowFlags(Qt::WindowStaysOnTopHint);
    int res = ask.exec();

    switch(res) {
        case QMessageBox::Yes:
            authenticator->setAllowed(true);
            break;
        case QMessageBox::Save:
            m_allowedOrigins << origin;
            appendToAllowedOriginsFile(origin);
            break;
        case QMessageBox::No:
            authenticator->setAllowed(false);
            break;
        default:
            authenticator->setAllowed(false);
            break;
    }
}

void PublicAPIsServer::appendToAllowedOriginsFile(const QString &origin)
{
    QFile oriFile(ALLOWED_ORIGINS_FILENAME);
    if (oriFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        QTextStream textStream(&oriFile);
        textStream.setCodec("UTF-8");
        textStream << origin + "\n";
        textStream.flush();
        oriFile.close();
    }
}
