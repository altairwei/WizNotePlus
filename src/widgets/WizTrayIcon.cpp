#include "WizTrayIcon.h"
#include <QApplication>
#include <QWidget>
#include <QDebug>
#include "database/WizSyncableDatabase.h"
#include "WizDef.h"

WizTrayIcon::WizTrayIcon(WizExplorerApp& app, QObject* parent)
    : QSystemTrayIcon(parent)
    , m_app(app)
{
    connect(this, SIGNAL(messageClicked()), SLOT(onMessageClicked()));
}

WizTrayIcon::WizTrayIcon(WizExplorerApp& app, const QIcon& icon, QObject* parent)
    : QSystemTrayIcon(icon, parent)
    , m_app(app)
{
    connect(this, SIGNAL(messageClicked()), SLOT(onMessageClicked()));
}

WizTrayIcon::~WizTrayIcon()
{
}

void WizTrayIcon::showMessage(const QString& title, const QString& msg, QSystemTrayIcon::MessageIcon icon, int msecs, const QVariant& param)
{
    m_messageType = wizBubbleNormal;
    m_messageData = param;
#ifdef Q_OS_LINUX
    // On Linux, system tray icon will be changed to message icon and cannot be restored.
    QSystemTrayIcon::showMessage(title, msg, QSystemTrayIcon::NoIcon, msecs);
#else
    QSystemTrayIcon::showMessage(title, msg, icon, msecs);
#endif
}

void WizTrayIcon::showMessage(const QVariant& param)
{
    QList<QVariant> paramList = param.toList();
    if (paramList.count() < 2)
        return;

    m_messageType = wizBubbleNoMessage;
    m_messageData.clear();

    int type = paramList.first().toInt();
    if (type == wizBubbleMessageCenter)
    {
        Q_ASSERT(paramList.count() == 4);
        m_messageType = wizBubbleMessageCenter;
        m_messageData = paramList.at(1);
#ifdef Q_OS_LINUX
        // On Linux, system tray icon will be changed to message icon and cannot be restored.
        QSystemTrayIcon::showMessage(paramList.at(2).toString(), paramList.at(3).toString(), QSystemTrayIcon::NoIcon);
#else
        QSystemTrayIcon::showMessage(paramList.at(2).toString(), paramList.at(3).toString(), QSystemTrayIcon::Information);
#endif
    }
}

void WizTrayIcon::onMessageClicked()
{
#ifndef Q_OS_LINUX
    if (m_messageType == wizBubbleMessageCenter)
    {
        m_app.mainWindow()->raise();
        qint64 id = m_messageData.toLongLong();
        emit viewMessageRequest(id);

        m_messageType = wizBubbleNoMessage;
        m_messageData.clear();
    }
    else if (m_messageType == wizBubbleNormal)
    {
        m_app.mainWindow()->raise();
        emit viewMessageRequestNormal(m_messageData);

        m_messageType = wizBubbleNoMessage;
        m_messageData.clear();
    }
#endif
}


