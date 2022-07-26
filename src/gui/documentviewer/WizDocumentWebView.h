#ifndef WIZDOCUMENTWEBVIEW_H
#define WIZDOCUMENTWEBVIEW_H

#include <QWebEngineView>
#include <QTimer>
#include <QPointer>
#include <QMutex>
#include <QColorDialog>
#include <QProcess>
#include <QMap>
#include <QThread>
#include <QWaitCondition>

//#include "WizDownloadObjectDataDialog.h"
#include "WizDef.h"
#include "share/WizObject.h"
#include "share/WizWebEngineView.h"
#include "api/ApiWizHtmlEditorApp.h"
#include "DocumentLoaderSaver.h"
#include "AbstractDocumentView.h"

class WizObjectDownloaderHost;
class WizEditorInsertLinkForm;
class WizEditorInsertTableForm;
class WizDocumentWebView;
class WizDocumentTransitionView;
class CWizDocumentWebViewWorker;
class QNetworkDiskCache;
class WizSearchReplaceWidget;

struct WIZODUCMENTDATA;
struct WizExternalEditorData;

class WizDocumentView;

class WizDocumentWebViewPage: public WizWebEnginePage
{
    Q_OBJECT

public:
    explicit WizDocumentWebViewPage(WizDocumentWebView* parent);
    virtual bool acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame);
    virtual void triggerAction(WebAction action, bool checked = false);
    virtual void javaScriptConsoleMessage(JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID);

Q_SIGNALS:
    void actionTriggered(WebAction act);
    
private:
    WizDocumentWebView* m_engineView;
};

/**
 * @brief WizDocumentWebView is responsible for display notes and server as WizQtEditor.
 */
class WizDocumentWebView : public AbstractDocumentEditor
{
    Q_OBJECT

public:
    WizDocumentWebView(WizExplorerApp& app, QWidget* parent);
    ~WizDocumentWebView();

    WizDocumentView* view() const;

    void clear();

    // view and save
    void viewDocument(const WIZDOCUMENTDATA& doc, WizEditorMode editorMode);
    void setEditorMode(WizEditorMode editorMode) override;
    void trySaveDocument(const WIZDOCUMENTDATA& data, bool force, std::function<void(const QVariant &)> callback);
    void reloadNoteData(const WIZDOCUMENTDATA& data);

    bool isEditing() const { return m_currentEditorMode == modeEditor; }
    /*bool isExternalEditing() const {return m_currentEditorMode == modeExternal;}*/

    void setNoteTitleInited(bool inited);

    void setInSeperateWindow(bool inSeperateWindow);
    bool isInSeperateWindow() const;

    WizDocumentWebViewPage* getPage();
    QString documentTitle() override;

    // initialize editor style before render, only invoke once.
    void replaceDefaultCss(QString& strHtml);

    /* editor related */
    void editorResetFont();
    void editorFocus();
    void enableEditor(bool enalbe);
    QString noteResourcesPath();
    void updateSvg(QString data);

    void editorResetSpellCheck();

    void setIgnoreActiveWindowEvent(bool igoreEvent);

    bool evaluateJavaScript(const QString& js);

    // -1: command invalid
    // 0: available
    // 1: executed before
    void editorCommandQueryCommandState(const QString& strCommand, std::function<void(int state)> callback);
    void editorCommandQueryCommandValue(const QString& strCommand, std::function<void(const QString& value)> callback);
    //
    void editorCommandExecuteCommand(const QString& strCommand,
                                     const QString& arg1 = QString(),
                                     const QString& arg2 = QString(),
                                     const QString& arg3 = QString());

    bool editorCommandQueryMobileFileReceiverState();

    void editorCommandExecuteParagraph(const QString& strType);
    void editorCommandExecuteFontFamily(const QString& strFamily);
    void editorCommandExecuteFontSize(const QString& strSize);
    void editorCommandExecuteInsertHtml(const QString& strHtml, bool bNotSerialize);

    void editorCommandExecutePastePlainText();

    void saveAsPDF();
    void saveAsMarkdown();
    void saveAsMarkdown(QString& strIndexFileName, bool bSaveResource = true);
    void saveAsPlainMarkdown(QString& destFileName, std::function<void(QString fileName)> callback);
    void saveAsPlainText(QString& destFileName, std::function<void(QString fileName)> callback);
    void saveAsRenderedHtml(QString& destFileName, std::function<void(QString fileName)> callback);
    void saveAsHtml();

    void isModified(std::function<void(bool modified)> callback) override;
    void setModified(bool b);

    //use undo func provied by editor
    void undo();
    void redo();

    //js environment func
    QString getUserGuid();
    QString getUserAvatarFilePath();
    QString getUserAlias();
    bool isPersonalDocument();
    bool isOutline() const;
    QString getCurrentNoteHtml();
    bool hasEditPermissionOnCurrentNote();
    void changeCurrentDocumentType(const QString& strType);
    bool checkListClickable();
    bool shouldAddCustomCSS();
    bool canRenderMarkdown();
    bool canEditNote();
    QString getLocalLanguage();
    void onSelectionChange(const QString& currentStyle);
    void onClickedSvg(const QString& data);
    void saveCurrentNote();
    void onReturn();
    void doPaste();
    void doCopy();
    void afterCopied();

    void onMarkerUndoStatusChanged(QString data);
    void onMarkerInitiated(QString data);

    QObject *publicAPIsObject() { return m_htmlEditorApp; }

private:
    void initEditorActions();

    void addDefaultScriptsToDocumentHtml(QString htmlFileName);
    void loadDocumentInWeb(WizEditorMode editorMode);

    void getAllEditorScriptAndStyleFileName(std::map<QString, QString>& arrayFile);
    void insertScriptAndStyleCore(QString& strHtml, const std::map<QString, QString>& files);

    void tryResetTitle();
    bool onPasteCommand();

    bool isInternalUrl(const QUrl& url);
    void viewDocumentByUrl(const QString& strUrl);
    void viewAttachmentByUrl(const QString& strKbGUID, const QString& strUrl);

    void saveEditingViewDocument(const WIZDOCUMENTDATA& data, bool force, const std::function<void(const QVariant &)> callback);
    void saveReadingViewDocument(const WIZDOCUMENTDATA& data, bool force, std::function<void(const QVariant &)> callback);

    void createReadModeContextMenu(QContextMenuEvent *event);
    bool acceptUrlAsAttachment(const QUrl &url);

protected:
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void inputMethodEvent(QInputMethodEvent* event) override;
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;
    virtual void contextMenuEvent(QContextMenuEvent* event) override;
    virtual void dragEnterEvent(QDragEnterEvent* event) override;
    virtual void dragMoveEvent(QDragMoveEvent* event) override;
    virtual void dropEvent(QDropEvent* event) override;
    void setupWebActions() override;
    void hideEvent(QHideEvent *event) override;

private:
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;
    QMap<QString, QString> m_mapFile;

    QTimer m_timerAutoSave;
    WizEditorMode m_currentEditorMode;
    bool m_bNewNote;
    bool m_bNewNoteTitleInited;

    QString m_strNoteHtmlFileName;
    QString m_currentNoteHtml;

    bool m_bContentsChanged;

    bool m_ignoreActiveWindowEvent;

    // flag : if current webview is in seperate window, editor background-color will
    //different with webview in mainwindow
    bool m_bInSeperateWindow;

    int m_nWindowID;

    QPointer<WizEditorInsertLinkForm> m_editorInsertLinkForm;

    WizSearchReplaceWidget* m_searchReplaceWidget;

    ApiWizHtmlEditorApp* m_htmlEditorApp;

    QList<WizExternalEditorData> m_extEditorTask;

public:
    void onNoteLoadFinished(); // editor callback
    void discardChanges();

public Q_SLOTS:
    void onActionTriggered(QWebEnginePage::WebAction act);

    void onEditorLoadFinished(bool ok);
    void onEditorLinkClicked(QUrl url, QWebEnginePage::NavigationType navigationType, bool isMainFrame, WizWebEnginePage* page);

    void onTimerAutoSaveTimout();

    void onTitleEdited(QString strTitle) override;

    void onDocumentReady(const QString kbGUID, const QString strGUID, const QString strFileName, WizEditorMode editorMode);
    void onDocumentSaved(const QString kbGUID, const QString strGUID, bool ok, QObject *requester);

    void on_editorCommandExecuteLinkInsert_accepted();

    void on_insertCodeHtml_requset(QString strOldHtml);
    void on_insertCommentToNote_request(const QString& docGUID, const QString& comment);

    void onActionSaveTriggered();
    void handleReloadTriggered();

    //void onWatchedFileChanged(const QString& path, int TextEditor, int UTF8Encoding);
    /* editor API */

    // font
    void editorCommandExecuteBackColor(const QColor& color);
    void editorCommandExecuteForeColor(const QColor& color);
    void editorCommandExecuteBold();
    void editorCommandExecuteItalic();
    void editorCommandExecuteUnderLine();
    void editorCommandExecuteStrikeThrough();
    void editorCommandExecuteSubScript();
    void editorCommandExecuteSuperScript();

    void editorCommandExecuteLinkInsert();
    void editorCommandExecuteLinkRemove();

    // search and repalce
    void editorCommandExecuteFindReplace();
    void findPre(QString strTxt, bool bCasesensitive);
    void findNext(QString strTxt, bool bCasesensitive);
    void replaceCurrent(QString strSource, QString strTarget);
    void replaceAndFindNext(QString strSource, QString strTarget, bool bCasesensitive);
    void replaceAll(QString strSource, QString strTarget, bool bCasesensitive);

    // format
    void editorCommandExecuteIndent();
    void editorCommandExecuteOutdent();

    void editorCommandExecuteJustifyLeft();
    void editorCommandExecuteJustifyRight();
    void editorCommandExecuteJustifyCenter();
    void editorCommandExecuteJustifyJustify();

    void editorCommandExecuteInsertOrderedList();
    void editorCommandExecuteInsertUnorderedList();
    //
    void editorCommandExecuteTableInsert(int row, int col);

    // fast operation
    void editorCommandExecuteInsertDate();
    void editorCommandExecuteInsertTime();
    void editorCommandExecuteRemoveFormat();
    void editorCommandExecuteFormatPainterOn(bool multi);
    void editorCommandExecuteFormatPainterOff();
    void editorCommandExecuteInsertHorizontal();
    void editorCommandExecuteInsertCheckList();
    void editorCommandExecuteInsertImage();
    void editorCommandExecuteStartMarkup();
    void editorCommandExecuteStopMarkup();
    void editorCommandExecuteInsertPainter();
    void editorCommandExecuteInsertCode();
    void editorCommandExecuteMobileImage(bool bReceiveImage);
    void editorCommandExecuteScreenShot();
    void on_editorCommandExecuteScreenShot_imageAccepted(QPixmap pix);
    void on_editorCommandExecuteScreenShot_finished();

    void editorExecJs(QString js);
    void onViewMindMap(bool on);

Q_SIGNALS:
    void loadDocumentRequested(const WIZDOCUMENTDATA& doc, WizEditorMode mode);
    void saveDocumentRequested(const WIZDOCUMENTDATA& doc, const QString& strHtml,
                               const QString& strHtmlFile, int nFlags, bool bNotify = false);
    // signals for notify command reflect status, triggered when selection, focus, editing mode changed
    void statusChanged(const QString& currentStyle);
    void markerUndoStatusChanged(const QString& data);
    void markerInitiated(const QString& data);

    void selectAllKeyPressed();
    // signals used request reset info toolbar and editor toolbar
    void focusIn();
    void focusOut();

    void showContextMenuRequest(const QPoint& pos);
    void updateEditorToolBarRequest();

    void viewDocumentFinished();

    void isPersonalDocumentChanged();
    void hasEditPermissionOnCurrentNoteChanged();
    void canEditNoteChanged();
    void currentHtmlChanged();

    // signal connect to checklist in javascript
    void clickingTodoCallBack(bool cancel, bool needCallAgain);

    void titleEdited(WizDocumentView*, QString strTitle);

    void externalEditorOpened();
    void externalEditorClosed(int exitCode, QProcess::ExitStatus exitStatus);

    void devToolsRequested(QWebEnginePage* sourcePage);

private:
    void setWindowVisibleOnScreenShot(bool bVisible);
    void insertImage(const QString& strFileName);
    void addAttachmentThumbnail(const QString strFile, const QString& strGuid);
    void openVipPageInWebBrowser();
    void showContextMenu(const QPoint &pos);
    QString getNoteType();

    void innerFindText(QString text, bool next, bool matchCase);

    QString getHighlightKeywords();

//    bool shouldAddUserDefaultCSS();

    friend class WizDocumentWebViewPage;
    friend class ApiWizHtmlEditorApp;
};


#endif // WIZDOCUMENTWEBVIEW_H
