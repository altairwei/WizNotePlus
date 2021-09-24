#include "ExternalEditorLauncher.h"

#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QTextDocument>
#include <QFileSystemWatcher>
#include <QStringList>

#include "gui/documentviewer/WizDocumentView.h"
#include "gui/documentviewer/WizDocumentWebView.h"
#include "utils/WizPathResolve.h"
#include "utils/WizMisc.h"
#include "share/WizMisc.h"
#include "database/WizDatabaseManager.h"
#include "database/WizDatabase.h"
#include "WizFileImporter.h"
#include "WizMainWindow.h"

ExternalEditorLauncher::ExternalEditorLauncher(WizExplorerApp &app, QObject *parent)
    : QObject(parent)
    , m_app(app)
    , m_window(qobject_cast<WizMainWindow*>(m_app.mainWindow()))
    , m_dbMgr(app.databaseManager())
    , m_externalEditorFileWatcher(new QFileSystemWatcher(this))
{
    // initialize document saver thread
    m_watchedDocSaver = new WizDocumentWebViewSaverThread(m_dbMgr, this);

    connect(m_externalEditorFileWatcher, &QFileSystemWatcher::fileChanged, 
            this, &ExternalEditorLauncher::onWatchedDocumentChanged);
}

void ExternalEditorLauncher::waitForDone()
{
    // Stop m_docSaver thread
    auto saver = m_watchedDocSaver;
    m_watchedDocSaver = nullptr;
    saver->waitForDone();
}

/*!
    Ensure that all characters in the \a docTitle are legal and automatically
    add the file suffix number if the file exists in the folder \a tempDir .
 */
QString ExternalEditorLauncher::makeValidCacheFileName(const QString &docTitle, const QDir &tempDir)
{
    CString title = docTitle;
    WizMakeValidFileNameNoPath(title);
    QString cacheFileName = title;

    QFileInfo cacheFileInfo(cacheFileName);
    QString fileSuffix = cacheFileInfo.suffix();
    QString fileBaseName = cacheFileInfo.completeBaseName();
    int num = 1;

    while(tempDir.exists(cacheFileName)) {
        cacheFileName = fileBaseName + "_" + QString::number(num++) + (fileSuffix.isEmpty() ? "" : "." + fileSuffix);
    }

    return tempDir.absoluteFilePath(cacheFileName);
}

void ExternalEditorLauncher::handleViewNoteInExternalEditorRequest(
    const WizExternalEditorData &editorData,
    const WIZDOCUMENTDATAEX &noteData)
{
    if (auto docView = qobject_cast<WizDocumentView*>(sender())) {
        // Make sure editor will get separate cache file.
        QDir noteTempDir(Utils::WizPathResolve::tempPath() + noteData.strGUID + "/");
        QString cacheFileName = makeValidCacheFileName(noteData.strTitle, noteTempDir);

        WizDatabase &db = m_dbMgr.db(noteData.strKbGUID);
        // Update document first, but we don't need to realod document.
        docView->web()->trySaveDocument(noteData, false,
            [this, &docView, &db, noteData, editorData,
             cacheFileName, noteTempDir] (const QVariant&) mutable 
            {
                // Update index.html
                QString strHtmlFile;
                db.documentToTempHtmlFile(noteData, strHtmlFile);

                QString strHtml;
                if (!WizLoadUnicodeTextFromFile(strHtmlFile, strHtml))
                    return;

                // Export Note to file
                if (editorData.TextEditor == 2) {
                    if (noteTempDir.exists(cacheFileName))
                        noteTempDir.remove(cacheFileName);

                    // get pure text
                    QTextDocument doc;
                    doc.setHtml(strHtml);
                    QString strText = doc.toPlainText();
                    strText.replace("&nbsp", " ");

                    // Write to cache file
                    if(WizSaveUnicodeTextToUtf8File(cacheFileName, strText, false))
                    {
                        startExternalEditor(cacheFileName, editorData, noteData);
                    }
                    
                } else if (editorData.TextEditor == 0) {
                    cacheFileName += ".html";
                    if (noteTempDir.exists(cacheFileName))
                        noteTempDir.remove(cacheFileName);

                    docView->web()->page()->toHtml([=](const QString& strHtml){
                        if(WizSaveUnicodeTextToUtf8File(cacheFileName, strHtml, false))
                            startExternalEditor(cacheFileName, editorData, noteData);
                    });
                }
            }
        );
    }
}

/*!
    Start external editor process and watch the document \a cacheFileName.
 */
void ExternalEditorLauncher::startExternalEditor(QString cacheFileName, const WizExternalEditorData &editorData, const WIZDOCUMENTDATAEX &noteData)
{
    // Launch external editor process
    // FIXME: split too many items
    QString programFile = "\"" + editorData.ProgramFile + "\"";
    QString args = editorData.Arguments.arg("\"" + cacheFileName + "\"");
    QString strCmd = programFile + " " + args;

    // Do not use QProcess instances to detect the state of external editor GUI programs,
    // for reasons described in https://github.com/altairwei/WizNotePlus/issues/199

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    auto cmdList = QProcess::splitCommand(strCmd);
    auto prog = cmdList.takeFirst();
    QProcess::startDetached(prog, cmdList);
#else
    QProcess::startDetached(strCmd);
#endif

    // Watch the cache file
    if (!m_externalEditorFileWatcher->addPath(cacheFileName)) {
        QMessageBox::critical(m_app.mainWindow(),
            tr("Failed to launch external editor"),
            tr("There may be a system dependent limit to the number "
               "of files that can be monitored simultaneously.")
        );
        return;
    }

    // Remeber notes data
    WizExternalEditTask newTask = { editorData, noteData, cacheFileName };
    m_externalEditorTasks.insert(cacheFileName, newTask);
}

void ExternalEditorLauncher::onWatchedDocumentChanged(const QString& fileName)
{
    saveWatchedFile(fileName);

    QFileInfo watchedFile(fileName);

    // Many applications save an open file by writing a new file and then deleting the old one.
    // So we watch that file again.
    if(!m_externalEditorFileWatcher->files().contains(fileName)){
        if (watchedFile.exists()) {
            if (!m_externalEditorFileWatcher->addPath(fileName)) {
                m_window->showBubbleNotification(
                    tr("Unable to continue monitoring files"),
                    tr("Failed to monitor the file required "
                        "by external editor: %1").arg(watchedFile.fileName())
                );
            }
        } else {
            m_window->showBubbleNotification(
                tr("Unable to continue monitoring files"),
                tr("File does not exist: %1").arg(watchedFile.fileName())
            );
        }
    }
}

/*!
    Save the watched document \a fileName which is changed by external editor.
 */
void ExternalEditorLauncher::saveWatchedFile(const QString &fileName)
{
    QFileInfo changedFileInfo(fileName);

    if ( !changedFileInfo.exists())
        return;

    // Get note's data
    QString noteGUID = changedFileInfo.absoluteDir().dirName();
    const WizExternalEditTask& task = m_externalEditorTasks[fileName];
    bool isUTF8 = task.editorData.UTF8Encoding == 0 ? false : true;
    bool isPlainText = task.editorData.TextEditor == 0 ? false : true;
    // Get modified document's text
    QString strHtml;
    if (isPlainText) {
        // Plain Text
        strHtml = WizFileImporter::loadTextFileToHtml(fileName, "UTF-8");
        //TODO: add markdown img to <link rel="File-List" type="image/png" href="" /> elements in html head.
        strHtml = QString("<!DOCTYPE html><html><head></head><body>%1</body></html>").arg(strHtml);
    } else {
        strHtml = WizFileImporter::loadHtmlFileToHtml(fileName, "UTF-8");
    }
    // Get document's file name
    //FIXME: Windows client has encoding problem.
    QString indexFileName = Utils::WizMisc::extractFilePath(fileName) + "index.html";
    // Start saver thread
    m_watchedDocSaver->save(task.docData, strHtml, indexFileName, 0, true);
}