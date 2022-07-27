#ifndef GUI_DOCUMENTVIEWER_DOCUMENTLOADERSAVER_H
#define GUI_DOCUMENTVIEWER_DOCUMENTLOADERSAVER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include "share/WizObject.h"

class WizDatabaseManager;

class WizWaitEvent
{

private:
    QMutex m_mutex;
    QWaitCondition m_waitForData;

public:
    void wait()
    {
        QMutexLocker locker(&m_mutex);
        Q_UNUSED(locker);

        m_waitForData.wait(&m_mutex);
    }

    void wakeAll()
    {
        QMutexLocker locker(&m_mutex);
        Q_UNUSED(locker);

        m_waitForData.wakeAll();
    }
};


class WizDocumentLoaderThread : public QThread
{
    Q_OBJECT

public:
    WizDocumentLoaderThread(WizDatabaseManager& dbMgr, QObject* parent);

    void stop();
    void waitForDone();

protected:
    virtual void run();
    void setCurrentDoc(QString kbGuid, QString docGuid, WizEditorMode editorMode);
    void peekCurrentDocGuid(QString& kbGUID, QString& docGUID, WizEditorMode& editorMode);

public Q_SLOTS:
    void load(const WIZDOCUMENTDATA& doc, WizEditorMode editorMode);

Q_SIGNALS:
    void loaded(const QString kbGUID, const QString strGUID, const QString strFileName, WizEditorMode editorMode);
    void loadFailed(const QString kbGUID, const QString strGUID, const QString strFileName, const QString strTitle);

private:
    bool isEmpty();

private:
    WizDatabaseManager& m_dbMgr;
    QString m_strCurrentKbGUID;
    QString m_strCurrentDocGUID;
    WizEditorMode m_editorMode;
    QMutex m_mutex;
    WizWaitEvent m_waitEvent;
    bool m_stop;
};

class WizDocumentSaverThread : public QThread
{
    Q_OBJECT

public:
    WizDocumentSaverThread(WizDatabaseManager& dbMgr, QObject* parent);
    void waitForDone();

public Q_SLOTS:
    void save(const WIZDOCUMENTDATA& doc, const QString& strHtml,
              const QString& strHtmlFile, int nFlags, bool bNotify = false);

private:
    struct SAVEDATA
    {
        WIZDOCUMENTDATA doc;
        QString html;
        QString htmlFile;
        int flags;
        bool notify;
        QObject *requester;
    };

    std::vector<SAVEDATA> m_arrayData;

    bool isEmpty();
    SAVEDATA peekFirst();

protected:
    virtual void run();
    void stop();
    void peekData(SAVEDATA& data);

Q_SIGNALS:
    void saved(const QString kbGUID, const QString strGUID, bool ok, QObject *requester);
    void saveFailed(const QString kbGUID, const QString strGUID, const QString strTitle);

private:
    WizDatabaseManager& m_dbMgr;
    QMutex m_mutex;
    WizWaitEvent m_waitEvent;
    bool m_stop;
};

#endif // GUI_DOCUMENTVIEWER_DOCUMENTLOADERSAVER_H
