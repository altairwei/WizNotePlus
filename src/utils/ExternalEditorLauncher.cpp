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
    m_watchedDocSaver = new WizDocumentSaverThread(m_dbMgr, this);

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
#ifdef Q_OS_WIN
    // MS Office Word can not handle space correctly.
    title.replace(" ", "_");
#endif
    WizMakeValidFileNameNoPath(title);
    QString cacheFileName = title;

    QFileInfo cacheFileInfo(cacheFileName);
    QString fileSuffix = cacheFileInfo.suffix();
    QString fileBaseName = cacheFileInfo.completeBaseName();
    int num = 1;

    while(tempDir.exists(cacheFileName)) {
        cacheFileName = fileBaseName + "_" + QString::number(num++)
            + (fileSuffix.isEmpty() ? "" : "." + fileSuffix);
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

        WizDatabase &db = m_dbMgr.db(noteData.strKbGUID);

        if (const auto webView = docView->web()) {
            // Update document first, but we don't need to realod document.
            webView->trySaveDocument(noteData, false,
                [this, webView, &db, noteData,
                 editorData, noteTempDir] (const QVariant&) mutable 
                {
                    // Update index.html
                    QString strHtmlFile;
                    db.documentToTempHtmlFile(noteData, strHtmlFile);

                    QString strHtml;
                    if (!WizLoadUnicodeTextFromFile(strHtmlFile, strHtml))
                        return;

                    // Export Note to file
                    if (editorData.TextEditor == 2) {
                        QString cacheFileName = makeValidCacheFileName(noteData.strTitle, noteTempDir);

                        // get pure text
                        QTextDocument doc;
                        doc.setHtml(strHtml);
                        QString strText = doc.toPlainText();
                        strText.replace("&nbsp", " ");

                        // Write to cache file
                        if(WizSaveUnicodeTextToUtf8File(cacheFileName, strText, false))
                            startExternalEditor(cacheFileName, editorData, noteData);
                        
                    } else if (editorData.TextEditor == 0) {
                        QString cacheFileName = makeValidCacheFileName(
                            noteData.strTitle + ".html", noteTempDir);
                        if (!webView)
                            return;
                        webView->page()->toHtml([=](const QString& html){
                            if(WizSaveUnicodeTextToUtf8File(cacheFileName, html, false))
                                startExternalEditor(cacheFileName, editorData, noteData);
                        });
                    }
                }
            );
        }
    }
}

/*!
    Start external editor process and watch the document \a cacheFileName.
 */
void ExternalEditorLauncher::startExternalEditor(
    QString cacheFileName,
    const WizExternalEditorData &editorData,
    const WIZDOCUMENTDATAEX &noteData)
{
    // Launch external editor process
    // FIXME: split too many items
    QString programFile = editorData.ProgramFile;
    QString args = editorData.Arguments.arg("\"" + cacheFileName + "\"");
    QString strCmd = programFile + " " + args;

    // Do not use attached QProcess to detect the state of external editor GUI programs,
    // for reasons described in https://github.com/altairwei/WizNotePlus/issues/199

    QProcess editor;
    editor.setProgram(programFile);
    editor.setArguments(splitCommand(args));
    editor.setWorkingDirectory(Utils::WizPathResolve::tempPath() + noteData.strGUID + "/");

    qDebug() << editor.program() << " " << editor.arguments();

    if (!editor.startDetached())
        m_window->showBubbleNotification(
            tr("Failed to launch external editor"),
            tr("%1 %2").arg(editor.program()).arg(editor.arguments().join(" "))
        );

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

/*!
    Copy from Qt 5.15.2

    Splits the string \a command into a list of tokens, and returns
    the list.

    Tokens with spaces can be surrounded by double quotes; three
    consecutive double quotes represent the quote character itself.
*/
QStringList ExternalEditorLauncher::splitCommand(const QString &command)
{
    QStringList args;
    QString tmp;
    int quoteCount = 0;
    bool inQuote = false;

    // handle quoting. tokens can be surrounded by double quotes
    // "hello world". three consecutive double quotes represent
    // the quote character itself.
    for (int i = 0; i < command.size(); ++i) {
        if (command.at(i) == QLatin1Char('"')) {
            ++quoteCount;
            if (quoteCount == 3) {
                // third consecutive quote
                quoteCount = 0;
                tmp += command.at(i);
            }
            continue;
        }
        if (quoteCount) {
            if (quoteCount == 1)
                inQuote = !inQuote;
            quoteCount = 0;
        }
        if (!inQuote && command.at(i).isSpace()) {
            if (!tmp.isEmpty()) {
                args += tmp;
                tmp.clear();
            }
        } else {
            tmp += command.at(i);
        }
    }
    if (!tmp.isEmpty())
        args += tmp;

    return args;
}