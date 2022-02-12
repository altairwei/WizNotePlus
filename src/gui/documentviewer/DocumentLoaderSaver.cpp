#include "DocumentLoaderSaver.h"

#include <QApplication>
#include <QDebug>
#include <QMessageBox>

#include "database/WizDatabaseManager.h"
#include "database/WizDatabase.h"
#include "share/WizThreads.h"

WizDocumentLoaderThread::WizDocumentLoaderThread(WizDatabaseManager &dbMgr, QObject *parent)
    : QThread(parent)
    , m_dbMgr(dbMgr)
    , m_editorMode(modeReader)
    , m_stop(false)
{
}

void WizDocumentLoaderThread::load(const WIZDOCUMENTDATA &doc, WizEditorMode editorMode)
{
    setCurrentDoc(doc.strKbGUID, doc.strGUID, editorMode);

    if (!isRunning())
    {
        start();
    }
}

void WizDocumentLoaderThread::stop()
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);

    m_stop = true;

    m_waitEvent.wakeAll();
}

void WizDocumentLoaderThread::waitForDone()
{
    stop();

    while (this->isRunning())
    {
        QApplication::processEvents();
    }

    this->disconnect();
}

void WizDocumentLoaderThread::run()
{
    //FIXME: Document needs to be loaded once at least.
    //  So, two conditional m_stop-return within the loop were commented.
    while (!m_stop)
    {
        //if (m_stop)
        //    return;
        //
        QString kbGuid;
        QString docGuid;
        WizEditorMode editorMode = modeReader;
        peekCurrentDocGuid(kbGuid, docGuid, editorMode);
        //if (m_stop)
        //    return;
        //
        if (docGuid.isEmpty())
            continue;
        //
        WizDatabase& db = m_dbMgr.db(kbGuid);
        WIZDOCUMENTDATA data;
        if (!db.documentFromGuid(docGuid, data))
        {
            continue;
        }
        //
        QString strHtmlFile;
        if (db.documentToTempHtmlFile(data, strHtmlFile))
        {
            emit loaded(kbGuid, docGuid, strHtmlFile, editorMode);
        }
        else
        {
            emit loadFailed(kbGuid, docGuid, strHtmlFile, data.strTitle);
        }
    }
}

void WizDocumentLoaderThread::setCurrentDoc(QString kbGUID, QString docGUID, WizEditorMode editorMode)
{
    {
        QMutexLocker locker(&m_mutex);
        Q_UNUSED(locker);

        m_strCurrentKbGUID = kbGUID;
        m_strCurrentDocGUID = docGUID;
        m_editorMode = editorMode;
    }

    m_waitEvent.wakeAll();
}

bool WizDocumentLoaderThread::isEmpty()
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);

    return m_strCurrentDocGUID.isEmpty();
}

void WizDocumentLoaderThread::peekCurrentDocGuid(QString& kbGUID, QString& docGUID, WizEditorMode& editorMode)
{
    if (isEmpty())
    {
        m_waitEvent.wait();
    }

    {
        QMutexLocker locker(&m_mutex);
        Q_UNUSED(locker);

        kbGUID = m_strCurrentKbGUID;
        docGUID = m_strCurrentDocGUID;
        editorMode = m_editorMode;

        m_strCurrentKbGUID.clear();
        m_strCurrentDocGUID.clear();
    }
}




WizDocumentSaverThread::WizDocumentSaverThread(WizDatabaseManager &dbMgr, QObject *parent)
    : QThread(parent)
    , m_dbMgr(dbMgr)
    , m_stop(false)
{
}

void WizDocumentSaverThread::save(const WIZDOCUMENTDATA& doc, const QString& strHtml,
                                          const QString& strHtmlFile, int nFlags, bool bNotify /*= fasle*/)
{
    SAVEDATA data;
    data.doc = doc;
    data.html = strHtml;
    data.htmlFile = strHtmlFile;
    data.flags = nFlags;
    data.notify = bNotify;
    data.requester = sender();

    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);

    m_arrayData.push_back(data);

    m_waitEvent.wakeAll();

    if (!isRunning())
    {
        start();
    }
}

void WizDocumentSaverThread::waitForDone()
{
    stop();

    while (this->isRunning())
    {
        QApplication::processEvents();
    }

    this->disconnect();
}

void WizDocumentSaverThread::stop()
{
    m_stop = true;
    m_waitEvent.wakeAll();
}

bool WizDocumentSaverThread::isEmpty()
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);

    return m_arrayData.empty();
}

WizDocumentSaverThread::SAVEDATA WizDocumentSaverThread::peekFirst()
{
    QMutexLocker locker(&m_mutex);
    Q_UNUSED(locker);

    SAVEDATA data = m_arrayData[0];
    m_arrayData.erase(m_arrayData.begin());
    return data;
}

void WizDocumentSaverThread::peekData(SAVEDATA& data)
{
    while (1)
    {
        if (m_stop)
            return;

        if (isEmpty())
        {
            m_waitEvent.wait();
        }

        if (isEmpty())
        {
            if (m_stop)
                return;
            continue;
        }

        data = peekFirst();

        break;
    }
}

void WizDocumentSaverThread::run()
{
    while (true)
    {
        SAVEDATA data;
        peekData(data);

        if (data.doc.strGUID.isEmpty())
        {
            if (m_stop)
                return;
            continue;
        }

        WizDatabase& db = m_dbMgr.db(data.doc.strKbGUID);

        WIZDOCUMENTDATA doc;
        if (!db.documentFromGuid(data.doc.strGUID, doc))
        {
            qDebug() << "fault error: can't find doc in database: " << doc.strGUID;
            continue;
        }

        qDebug() << "Saving note: " << doc.strTitle;

        bool ok = db.updateDocumentData(doc, data.html, data.htmlFile, data.flags, data.notify);

        if (ok)
        {
            qDebug() << "Save note done: " << doc.strTitle;
        }
        else
        {
            qDebug() << "Save note failed: " << doc.strTitle;
            emit saveFailed(db.kbGUID(), doc.strGUID, doc.strTitle);
        }

        emit saved(db.kbGUID(), doc.strGUID, ok, data.requester);
    }
}

