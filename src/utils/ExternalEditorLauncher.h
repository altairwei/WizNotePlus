#ifndef EXTERNAL_EDITOR_LAUNCHER_H
#define EXTERNAL_EDITOR_LAUNCHER_H

#include <QProcess>
#include <QDir>

#include "WizDef.h"
#include "share/WizObject.h"

class QFileSystemWatcher;
class WizDocumentSaverThread;
class WizMainWindow;

struct WizExternalEditorData
{
    QString Name;
    QString ProgramFile;
    QString Arguments;
    int TextEditor;
    int UTF8Encoding;
};

struct WizExternalEditTask
{
    WizExternalEditorData editorData;
    WIZDOCUMENTDATAEX docData;
    QString cacheFileName;
};

class ExternalEditorLauncher : public QObject
{
    Q_OBJECT

public:
    ExternalEditorLauncher(WizExplorerApp &app, QObject *parent = nullptr);

    void startExternalEditor(QString cacheFileName, const WizExternalEditorData& editorData, const WIZDOCUMENTDATAEX& noteData);
    void saveWatchedFile(const QString& path);
    WizDocumentSaverThread* watchedDocSaver() { return m_watchedDocSaver; }
    void waitForDone();

    static QStringList splitCommand(const QString &command);

protected:
    QString makeValidCacheFileName(const QString &docTitle, const QDir &tempDir);

public Q_SLOTS:
    void onWatchedDocumentChanged(const QString& fileName);
    void handleViewNoteInExternalEditorRequest(
        const WizExternalEditorData &editorData, const WIZDOCUMENTDATAEX &noteData);

protected:
    WizExplorerApp &m_app;
    WizMainWindow *m_window;
    WizDatabaseManager &m_dbMgr;

private:
    QFileSystemWatcher* m_externalEditorFileWatcher;
    QMap<QString, WizExternalEditTask> m_externalEditorTasks; /* filename as key. */
    WizDocumentSaverThread* m_watchedDocSaver;
};


#endif // EXTERNAL_EDITOR_LAUNCHER_H