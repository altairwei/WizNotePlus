#ifndef EXTERNAL_EDITOR_LAUNCHER_H
#define EXTERNAL_EDITOR_LAUNCHER_H

#include <QProcess>

#include "WizDef.h"
#include "share/WizObject.h"

class QFileSystemWatcher;
class WizDocumentWebViewSaverThread;
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
    QProcess *editorProcess;
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
    WizDocumentWebViewSaverThread* watchedDocSaver() { return m_watchedDocSaver; }
    void waitForDone();

public Q_SLOTS:
    void onWatchedDocumentChanged(const QString& fileName);
    void handleViewNoteInExternalEditorRequest(
        const WizExternalEditorData &editorData, const WIZDOCUMENTDATAEX &noteData);
    void handleExternalEditorFinished(int exitCode, QProcess::ExitStatus exitStatus);

protected:
    WizExplorerApp &m_app;
    WizMainWindow *m_window;
    WizDatabaseManager &m_dbMgr;

private:
    QFileSystemWatcher* m_externalEditorFileWatcher;
    QMap<QString, WizExternalEditTask> m_externalEditorTasks; /* filename as key. */
    QList<QProcess*> m_externalEditorProcesses;
    WizDocumentWebViewSaverThread* m_watchedDocSaver;
};


#endif // EXTERNAL_EDITOR_LAUNCHER_H