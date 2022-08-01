#include "WizDocumentWebView.h"

#include <QRunnable>
#include <QList>
#include <QMimeData>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QRegExp>
#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>
#include <QPrinter>
#include <QFileDialog>
#include <QTextEdit>
#include <QMultiMap>
#include <QPrintDialog>
#include <QPrinterInfo>
#include <QMessageBox>
#include <QProcess>
#include <QFileSystemWatcher>
#include <QCommandLineParser>
#include <QTimer>
#include <QTextDocument>

#include <QApplication>
#include <QUndoStack>
#include <QDesktopServices>
#include <QNetworkDiskCache>
#include <QWebEngineProfile>

#ifdef Q_OS_MAC
#include <QMacPasteboardMime>
#endif

#include "share/WizGlobal.h"
#include "share/WizMisc.h"
#include "share/WizAnalyzer.h"
#include "share/WizMessageBox.h"
#include "share/WizObjectDataDownloader.h"
#include "share/WizThreads.h"

#include "WizDef.h"

#include "utils/WizPathResolve.h"
#include "utils/WizLogger.h"
#include "utils/WizMisc.h"
#include "utils/WizStyleHelper.h"

#include "database/WizDatabaseManager.h"

#include "sync/WizAvatarHost.h"
#include "sync/WizToken.h"
#include "sync/WizApiEntry.h"
#include "core/WizAccountManager.h"
#include "core/WizNoteManager.h"

#include "widgets/WizCodeEditorDialog.h"
#include "widgets/WizScreenShotWidget.h"
#include "widgets/WizEmailShareDialog.h"
#include "widgets/WizShareLinkDialog.h"
#include "widgets/WizScrollBar.h"
#include "widgets/WizExecutingActionDialog.h"

#include "mac/WizMacHelper.h"

#include "WizMainWindow.h"
#include "WizDocumentTransitionView.h"
#include "WizFileImporter.h"

#include "html/WizHtmlReader.h"
#include "api/ApiWizHtmlEditorApp.h"
#include "jsplugin/JSPluginManager.h"

#include "WizTitleBar.h"
#include "WizDocumentView.h"
#include "WizSearchReplaceWidget.h"
#include "WizEditorInsertLinkForm.h"
#include "WizEditorInsertTableForm.h"
#include "WizSvgEditorDialog.h"

enum WizLinkType {
    WizLink_Doucment,
    WizLink_Attachment
};

WizDocumentWebViewPage::WizDocumentWebViewPage(WizDocumentWebView* parent)
    : WizWebEnginePage(parent)
    , m_engineView(parent)
{
    Q_ASSERT(m_engineView);

    action(QWebEnginePage::Undo)->setShortcut(QKeySequence());
    action(QWebEnginePage::Redo)->setShortcut(QKeySequence());
    action(QWebEnginePage::Copy)->setShortcut(QKeySequence());
    action(QWebEnginePage::Cut)->setShortcut(QKeySequence());
    action(QWebEnginePage::Paste)->setShortcut(QKeySequence());
    action(QWebEnginePage::SelectAll)->setShortcut(QKeySequence());
}

bool WizDocumentWebViewPage::acceptNavigationRequest(const QUrl &url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
    if (NavigationTypeBackForward == type || NavigationTypeReload == type)
        return false;
    //
    return WizWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
}

void WizDocumentWebViewPage::triggerAction(QWebEnginePage::WebAction typeAction, bool checked)
{
    if (typeAction == QWebEnginePage::Back || typeAction == QWebEnginePage::Forward) {
        return;
    }

    if (typeAction == QWebEnginePage::Paste) {
        if (m_engineView->onPasteCommand())
            return;
    } else if (typeAction == QWebEnginePage::Undo || typeAction == QWebEnginePage::Redo) {
        //FIXME: cannot forbid webpage short cutter key after QT5.4.2, webpage short cutter will override menubar short cutter
        Q_EMIT actionTriggered(typeAction);
        return;
    }

    WizWebEnginePage::triggerAction(typeAction, checked);

    Q_EMIT actionTriggered(typeAction);
}

void WizDocumentWebViewPage::javaScriptConsoleMessage(QWebEnginePage::JavaScriptConsoleMessageLevel level, const QString& message, int lineNumber, const QString& sourceID)
{
    Q_UNUSED(sourceID);

    qDebug() << "[Console]line: " << lineNumber << ", " << message;
}


static int nWindowIDCounter = 0;

WizDocumentWebView::WizDocumentWebView(WizExplorerApp& app, QWidget* parent)
    : AbstractDocumentEditor(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_bNewNote(false)
    , m_bNewNoteTitleInited(false)
    , m_bContentsChanged(false)
    , m_bInSeperateWindow(false)
    , m_nWindowID(nWindowIDCounter ++)
    , m_searchReplaceWidget(nullptr)
    , m_ignoreActiveWindowEvent(false)
{
    WizDocumentWebViewPage* page = new WizDocumentWebViewPage(this);
    setupPage(page);
    connect(page, &WizDocumentWebViewPage::linkClicked,
            this, &WizDocumentWebView::onEditorLinkClicked);

    if (m_app.userSettings().isEnableSpellCheck()) {
        QWebEngineProfile *profile = page->profile();
        profile->setSpellCheckEnabled(true);
        profile->setSpellCheckLanguages({"en-US"});
    }

    // minimum page size hint
    //setMinimumSize(400, 250);

    // only accept focus by mouse click as the best way to trigger toolbar reset
    setFocusPolicy(Qt::ClickFocus);
    setAttribute(Qt::WA_AcceptTouchEvents, false);

    // FIXME: should accept drop picture, attachment, link etc.
    setAcceptDrops(true);

    m_htmlEditorApp = new ApiWizHtmlEditorApp(this, this);
    connect(this, &WizDocumentWebView::isPersonalDocumentChanged,
        m_htmlEditorApp, &ApiWizHtmlEditorApp::isPersonalDocumentChanged);
    connect(this, &WizDocumentWebView::hasEditPermissionOnCurrentNoteChanged,
        m_htmlEditorApp, &ApiWizHtmlEditorApp::hasEditPermissionOnCurrentNoteChanged);
    connect(this, &WizDocumentWebView::canEditNoteChanged,
        m_htmlEditorApp, &ApiWizHtmlEditorApp::canEditNoteChanged);
    connect(this, &WizDocumentWebView::currentHtmlChanged,
        m_htmlEditorApp, &ApiWizHtmlEditorApp::currentHtmlChanged);
    connect(this, &WizDocumentWebView::clickingTodoCallBack,
        m_htmlEditorApp, &ApiWizHtmlEditorApp::clickingTodoCallBack);


    // refers
    qRegisterMetaType<WizEditorMode>("WizEditorMode");

    // loading and saving thread
    m_timerAutoSave.setInterval(1*60*1000); // 1 minutes
    connect(&m_timerAutoSave, SIGNAL(timeout()), SLOT(onTimerAutoSaveTimout()));

    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    addObjectToJavaScriptClient("WizExplorerApp", mainWindow->publicAPIsObject());
    addObjectToJavaScriptClient("WizQtEditor", m_htmlEditorApp);

    connect(this, SIGNAL(loadFinishedEx(bool)), SLOT(onEditorLoadFinished(bool)));
    connect(view()->titleBar(), SIGNAL(onViewMindMap(bool)), SLOT(onViewMindMap(bool)));

}

WizDocumentWebView::~WizDocumentWebView()
{
}

void WizDocumentWebView::setupWebActions()
{
    WizWebEngineView::setupWebActions();

    // Add "Save Note" action.
    QAction* action = new QAction(tr("Save Note"), this);
    action->setShortcut(QKeySequence::Save);
    connect(action, &QAction::triggered, this, &WizDocumentWebView::onActionSaveTriggered);
    addAction(action);

    // reload
    connect(pageAction(QWebEnginePage::Reload), &QAction::triggered, 
            this, &WizDocumentWebView::handleReloadTriggered, Qt::UniqueConnection);

    // Open temp file location
    connect(viewAction(OpenTempFileLocation), &QAction::triggered, [this]() {
        QUrl tmpfileFolder = page()->url().adjusted(QUrl::RemoveFilename);
        QDesktopServices::openUrl(tmpfileFolder);
    });
}

WizDocumentWebViewPage* WizDocumentWebView::getPage() {
    return qobject_cast<WizDocumentWebViewPage*>(page());
}

bool WizDocumentWebView::onPasteCommand()
{
    QClipboard* clip = QApplication::clipboard();
    Q_ASSERT(clip);

    if (isEditing())
    {
        const QMimeData *mimeData = clip->mimeData();
        qDebug() << "Clipboard MIME Data has formats: " << mimeData->formats();
        // On MacOS: copying files will result in "text/uri-list", "text/plain", "application/x-qt-image" formats.
        // So, we should test `hasUrls()` before `hasImage()`.
        if (mimeData->hasUrls()) {
            int nAccepted = 0;

            QList<QUrl> urls = mimeData->urls();
            QList<QUrl>::const_iterator it;
            for (it = urls.constBegin(); it != urls.constEnd(); it++) {
                QUrl url = *it;
                if (acceptUrlAsAttachment(url))
                    nAccepted++;
            }

            return nAccepted > 0;
        } else if (mimeData->hasImage()) {
            // save clipboard image to
            QString strImagePath = noteResourcesPath();
            CString strFileName = strImagePath + WizIntToStr(WizGetTickCount()) + ".png";
            if (!clip->image().save(strFileName)) {
                TOLOG("ERROR: Can't save clipboard image to file");
                return false;
            }

            QString strHtml;
            if (!WizImage2Html(strFileName, strHtml, strImagePath))
                return false;

            editorCommandExecuteInsertHtml(strHtml, true);

            return true;
        }

    }

    return false;
}

void WizDocumentWebView::inputMethodEvent(QInputMethodEvent* event)
{
    // On X windows, fcitx flick while preediting, only update while webview end process.
    // maybe it's a QT-BUG?
#ifdef Q_OS_LINUX
    setUpdatesEnabled(false);
    //QWebView::inputMethodEvent(event);
    setUpdatesEnabled(true);
#else
    QWebEngineView::inputMethodEvent(event);
#endif

#ifdef Q_OS_MAC
    /*
    /// comment below code, for move cursor will cause extrem high cpu, made input stop
    //int nLength = 0;
    int nOffset = 0;
    for (int i = 0; i < event->attributes().size(); i++) {
        const QInputMethodEvent::Attribute& a = event->attributes().at(i);
        if (a.type == QInputMethodEvent::Cursor) {
            //nLength = a.length;
            nOffset = a.start;
            break;
        }
    }

    // Move cursor
    // because every time input method event triggered, old preedit text will be
    // deleted and the new one inserted, this action made cursor restore to the
    // beginning of the input context, move it as far as offset indicated after
    // default implementation should correct this issue!!!
    for (int i = 0; i < nOffset; i++) {
        page()->triggerAction(QWebPage::MoveToNextChar);
    }
    */
#endif // Q_OS_MAC
}

/**
 * @brief Save current note when action_save triggered.
 * 
 */
void WizDocumentWebView::onActionSaveTriggered()
{
    trySaveDocument(view()->note(), false, [=](const QVariant&){});
}

void WizDocumentWebView::keyPressEvent(QKeyEvent* event)
{
    // WARNING!!
    // Because of unknown bugs, QKeyEvent cannot be passed to the QWebEngineView 
    // which is child of QTabWidget. This problem also appeared in official example:
    // Qt5.11.1/Examples/Qt-5.11.1/webenginewidgets/simplebrowser

    // special cases process
    if (event->key() == Qt::Key_Escape)
    {
        // FIXME: press esc will insert space at cursor if not clear focus
        clearFocus();
        return;
    }
    else if (event->key() == Qt::Key_S
             && event->modifiers() == Qt::ControlModifier)
    {
        trySaveDocument(view()->note(), false, [=](const QVariant&){});
        return;
    }
#if QT_VERSION >= 0x050402
//    else if (event->modifiers() == Qt::ControlModifier)
//    {
//        return;
//    }
    //FIXME: cannot use global save button after QT5.4.2 
    else if (event->key() == Qt::Key_V && event->modifiers() == Qt::ControlModifier)
    {
        WizGetAnalyzer().logAction("paste");

        triggerPageAction(QWebEnginePage::Paste);
        return;
    }
#endif
#if QT_VERSION < 0x050000
    #ifdef Q_OS_MAC
    else if (event->key() == Qt::Key_Z)
    {
        //Ctrl+Shift+Z,  shortcut for redo can't catch by actions in QT4
        Qt::KeyboardModifiers keyMod = QApplication::keyboardModifiers();
        bool isSHIFT = keyMod.testFlag(Qt::ShiftModifier);
        bool isCTRL = keyMod.testFlag(Qt::ControlModifier);
        if (isCTRL && isSHIFT) {
            redo();
            return;
        } else if (isCTRL && !isSHIFT) {
            undo();
            return;
        }
    }
    #endif

#endif


#ifdef Q_OS_LINUX
    setUpdatesEnabled(false);
    //QWebView::keyPressEvent(event);
    setUpdatesEnabled(true);
#else

    QWebEngineView::keyPressEvent(event);
#endif

    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        tryResetTitle();
    }

    emit updateEditorToolBarRequest();
}

void WizDocumentWebView::mousePressEvent(QMouseEvent* event)
{
    QWebEngineView::mousePressEvent(event);
    emit updateEditorToolBarRequest();
}

void WizDocumentWebView::focusInEvent(QFocusEvent *event)
{
    if (m_currentEditorMode == modeEditor) {
        Q_EMIT focusIn();
    }

    QWebEngineView::focusInEvent(event);
}

void WizDocumentWebView::focusOutEvent(QFocusEvent *event)
{
    // because qt will clear focus when context menu popup, we need keep focus there.
    if (event->reason() == Qt::PopupFocusReason)
    {
        return;
    }
    else if (m_ignoreActiveWindowEvent && event->reason() == Qt::ActiveWindowFocusReason)
    {
        //NOTE:when display CWizTipsWidget will let editor lose focus, then tips on toolbar disappear
        // use ignore produced ActiveWindowFocusReason when display tips to display tips
        return;
    }

    Q_EMIT focusOut();
    QWebEngineView::focusOutEvent(event);
}

void WizDocumentWebView::hideEvent(QHideEvent *event)
{
    if (m_searchReplaceWidget)
        m_searchReplaceWidget->hide();
    WizWebEngineView::hideEvent(event);
}

/**
 * @brief handle right menu event
 * 
 * @param event 
 */
void WizDocumentWebView::contextMenuEvent(QContextMenuEvent *event)
{
    if (isEditing()) {
        // Edit mode
        showContextMenu(mapToGlobal(event->pos()));
    } else {
        // Read mode
        createReadModeContextMenu(event);
    }
}

/**
 * @brief generate right menu under read pattern
 * 
 */
void WizDocumentWebView::createReadModeContextMenu(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    const QList<QAction *> actions = menu->actions();

    // remove back & forward actions
    auto backAction = std::find_if(actions.cbegin(), actions.cend(), [=](QAction * ac){
        return (ac->text() == "&Back" || ac->iconText() == "Back");
    });
    if (backAction != actions.cend()) {
        menu->removeAction(*backAction);
    }
    auto forwardAction = std::find_if(actions.cbegin(), actions.cend(), [=](QAction * ac){
        return (ac->text() == "&Forward" || ac->iconText() == "Forward");
    });
    if (forwardAction != actions.cend()) {
        menu->removeAction(*forwardAction);
    }

    // handle open location of document
    if(page()->url().isLocalFile()) {
        auto inspectElement = std::find(actions.cbegin(), actions.cend(), page()->action(QWebEnginePage::InspectElement));
        QAction *before(inspectElement == actions.cend() ? viewAction(OpenDevTools) : *inspectElement);
        menu->insertAction(before, viewAction(OpenTempFileLocation));
    }

    menu->popup(event->globalPos());
}

void WizDocumentWebView::handleReloadTriggered()
{
    reloadNoteData(view()->note());
}

void WizDocumentWebView::discardChanges()
{
    isModified([=](bool modified){
        // Close rich text editor
        enableEditor(false);
        m_currentEditorMode = modeReader;
        // Stop auto save
        m_timerAutoSave.stop();
        // Reload document if necessary
        if (modified)
            reloadNoteData(view()->note());
    });
}

void WizDocumentWebView::dragEnterEvent(QDragEnterEvent *event)
{
    if (!isEditing())
        return;

    int nAccepted = 0;
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> li = event->mimeData()->urls();
        QList<QUrl>::const_iterator it;
        for (it = li.begin(); it != li.end(); it++) {
            QUrl url = *it;
            if (url.toString().startsWith("file:///")) {
                nAccepted++;
            }
        }

        if (nAccepted == li.size()) {
            event->acceptProposedAction();
        }
    } else if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS)) {
        if (!event->mimeData()->data(WIZNOTE_MIMEFORMAT_DOCUMENTS).isEmpty()) {
            setFocus();
            event->acceptProposedAction();
        }
    }
}

void WizDocumentWebView::dragMoveEvent(QDragMoveEvent* event)
{
    if (event->mimeData()->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS)) {
            if (!event->mimeData()->data(WIZNOTE_MIMEFORMAT_DOCUMENTS).isEmpty()) {
                setFocus();
                event->acceptProposedAction();
            }
    }
}

void WizDocumentWebView::onActionTriggered(QWebEnginePage::WebAction act)
{
    // webpage will override menubar short cutter after qt5.4.2, and cannot avoid this action, some operation (undo, redo et.) need operat by editer
    // need webpage feedback operation to webview to operate
    if (act == QWebEnginePage::Paste)
    {
        tryResetTitle();
    }
    else if (QWebEnginePage::Undo == act)
    {
        //WizGetAnalyzer().LogAction("Undo");
        undo();
    }
    else if (QWebEnginePage::Redo == act)
    {
        //WizGetAnalyzer().LogAction("Redo");
        redo();
    }
}

void WizDocumentWebView::tryResetTitle()
{
    if (m_bNewNoteTitleInited)
        return;

    // if note already modified, maybe title changed by use manuallly
    if (view()->note().tCreated.secsTo(view()->note().tModified) != 0)
        return;

    //
    page()->toPlainText([=](const QString& text){
        QString strTitle = WizStr2Title(text.left(128));
        if (strTitle.isEmpty())
            return;

        view()->resetTitle(strTitle);
        //
        m_bNewNoteTitleInited = true;
    });
}

bool WizDocumentWebView::acceptUrlAsAttachment(const QUrl &url)
{
    //paste image file as images, add attachment for other file
    QString strFileName = url.toLocalFile();

#ifdef Q_OS_MAC
    if (wizIsYosemiteFilePath(strFileName))
    {
        strFileName = wizConvertYosemiteFilePathToNormalPath(strFileName);
    }
#endif

    QFileInfo info(strFileName);

    //FIXME: //TODO: should merged with add attachment in attachment list
    if (info.size() == 0) {
        WizMessageBox::warning(nullptr, tr("Info"),
            tr("Can not add a 0 bit size file as attachment! File name : ' %1 '").arg(strFileName));
        return false;
    } else if (info.isBundle()) {
        WizMessageBox::warning(nullptr, tr("Info"),
            tr("Can not add a bundle file as attachment! File name : ' %1 '").arg(strFileName));
        return false;
    }

    QList<QByteArray> imageFormats = QImageReader::supportedImageFormats();
    if (imageFormats.contains(info.suffix().toLower().toUtf8())) {
        QString strHtml;
        if (WizImage2Html(strFileName, strHtml, noteResourcesPath())) {
            editorCommandExecuteInsertHtml(strHtml, true);
            return true;
        }
    }
    else {
        WIZDOCUMENTDATA doc = view()->note();
        WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
        WIZDOCUMENTATTACHMENTDATA data;
        data.strKbGUID = doc.strKbGUID; // needed by under layer
        if (!db.addAttachment(doc, strFileName, data))
        {
            TOLOG1("[drop] add attachment failed %1", strFileName);
            return false;
        }
        addAttachmentThumbnail(strFileName, data.strGUID);
        return true;
    }
}

void WizDocumentWebView::dropEvent(QDropEvent* event)
{
    const QMimeData* mimeData = event->mimeData();

    int nAccepted = 0;
    if (mimeData->hasUrls())
    {
        QList<QUrl> li = mimeData->urls();
        QList<QUrl>::const_iterator it;
        for (it = li.constBegin(); it != li.constEnd(); it++) {
            QUrl url = *it;
            if (acceptUrlAsAttachment(url))
                nAccepted++;
        }
    }
    else if (mimeData->hasFormat(WIZNOTE_MIMEFORMAT_DOCUMENTS))
    {
        QString strData(mimeData->data(WIZNOTE_MIMEFORMAT_DOCUMENTS));
        if (!strData.isEmpty())
        {
            QString strLinkHtml;
            QStringList doclist = strData.split(';');
            foreach (QString doc, doclist) {
                QStringList docIds = doc.split(':');
                if (docIds.count() == 2)
                {
                    WIZDOCUMENTDATA document;
                    WizDatabase& db = m_dbMgr.db(docIds.first());
                    if (db.documentFromGuid(docIds.last(), document))
                    {
                        QString strHtml, strLink;
                        WizNoteToHtmlLink(document, strHtml, strLink);
                        strLinkHtml += "<span>&nbsp;" + strHtml + "&nbsp;</span>";
                    }
                }
            }

            editorCommandExecuteInsertHtml(strLinkHtml, false);

            nAccepted ++;
        }
    }

    if (nAccepted > 0) {
        event->accept();
        trySaveDocument(view()->note(), false, [=](const QVariant&){});
    }
}

WizDocumentView* WizDocumentWebView::view() const
{
    QWidget* pParent = parentWidget();
    while(pParent) {
        if (WizDocumentView* view = dynamic_cast<WizDocumentView*>(pParent)) {
            return view;
        }

        pParent = pParent->parentWidget();
    }

    return 0;
}

void WizDocumentWebView::clear()
{
    load(QUrl("about:blank"));
}

void WizDocumentWebView::onTimerAutoSaveTimout()
{
    trySaveDocument(view()->note(), false, [=](const QVariant&){});
}

void WizDocumentWebView::onTitleEdited(QString strTitle)
{
    emit titleEdited(view(), strTitle);
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    mainWindow->titleChanged();
    //
    WIZDOCUMENTDATA document = view()->note();
    document.strTitle = strTitle;
    // Only sync when contents unchanged. If contents changed would sync after document saved.
    isModified([=](bool modified){

        if (modified)
        {
            WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
            mainWindow->quickSyncKb(document.strKbGUID);
        }
    });
}

void WizDocumentWebView::onDocumentReady(const QString kbGUID, const QString strGUID, const QString strFileName, WizEditorMode editorMode)
{
    const auto &note = view()->note();
    if (note.strKbGUID != kbGUID || note.strGUID != strGUID)
        return;

    m_mapFile.insert(strGUID, strFileName);

    WIZDOCUMENTDATA doc;
    if (!m_dbMgr.db(kbGUID).documentFromGuid(strGUID, doc))
        return;

    loadDocumentInWeb(editorMode);
}

void WizDocumentWebView::onDocumentSaved(const QString kbGUID, const QString strGUID, bool ok, QObject *requester)
{
    const auto &note = view()->note();
    if (requester != this || note.strKbGUID != kbGUID || note.strGUID != strGUID)
        return;

    if (!ok)
    {
        TOLOG("Save document failed");
    }

    view()->sendDocumentSavedSignal(strGUID, kbGUID);

    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    mainWindow->quickSyncKb(kbGUID);
}

void WizDocumentWebView::viewDocument(const WIZDOCUMENTDATA& doc, WizEditorMode editorMode)
{
    // set data
    // FIXME: not very well to decide a file is new through create time
    qint64 seconds = doc.tCreated.secsTo(QDateTime::currentDateTime());
    m_bNewNote = (seconds >= 0 && seconds <= 1) ? true : false;
    m_bNewNoteTitleInited = m_bNewNote ? false : true;
    //
    setModified(false);

    if (m_bNewNote)
    {
        editorMode = modeEditor;
    }

    // ask extract and load
    Q_EMIT loadDocumentRequested(doc, editorMode);
}

void WizDocumentWebView::reloadNoteData(const WIZDOCUMENTDATA& data)
{
    Q_ASSERT(!data.strGUID.isEmpty());

    // reset only if user not in editing mode
    if (isEditing() && hasFocus())
        return;

    // reload may triggered when update from server or locally reflected by modify
    Q_EMIT loadDocumentRequested(data, m_currentEditorMode);
}


void WizDocumentWebView::setNoteTitleInited(bool inited)
{
    m_bNewNoteTitleInited = inited;
}

void WizDocumentWebView::setInSeperateWindow(bool inSeperateWindow)
{
    m_bInSeperateWindow = inSeperateWindow;
}

bool WizDocumentWebView::isInSeperateWindow() const
{
    return m_bInSeperateWindow;
}

QString WizDocumentWebView::documentTitle()
{
    return view()->note().strTitle;
}

void WizDocumentWebView::replaceDefaultCss(QString& strHtml)
{
    QString strFileName = Utils::WizPathResolve::resourcesPath() + "files/wizeditor/default.css";
    QFile f(strFileName);
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << "[Editor]Failed to get default css code";
        return;
    }

    QString strCss;
    if (!WizLoadUnicodeTextFromFile(strFileName, strCss))
        return;

    QString strFont = m_app.userSettings().defaultFontFamily();
    int nSize = m_app.userSettings().defaultFontSize();
    QString lineHeight = m_app.userSettings().editorLineHeight();
    QString paraSpacing = m_app.userSettings().editorParaSpacing();
    QString pagePadding = m_app.userSettings().editorPagePadding();
    QString backgroundColor = m_app.userSettings().editorBackgroundColor();

    strCss.replace("/*default-font-family*/", QString("font-family:%1;").arg(strFont));
    strCss.replace("/*default-font-size*/", QString("font-size:%1px;").arg(nSize));

    if (backgroundColor.isEmpty())
    {
        backgroundColor = m_bInSeperateWindow ? "#F5F5F5" : "#FFFFFF";
    }

    strCss.replace("/*default-line-height*/", QString("line-height:%1;").arg(lineHeight));
    strCss.replace("/*default-para-spacing*/", QString("margin-top:%1px; margin-bottom:%1px").arg(paraSpacing));
    strCss.replace("/*default-page-padding*/", QString("padding-left:%1px; padding-right:%1px;").arg(pagePadding));
    strCss.replace("/*default-background-color*/", QString("background-color:%1;").arg(backgroundColor));

    const QString customCssId("wiz_custom_css");

    WizHtmlRemoveStyle(strHtml, customCssId);
    WizHtmlInsertStyle(strHtml, customCssId, strCss);
}

void WizDocumentWebView::editorResetFont()
{
    WIZDOCUMENTDATA data = view()->note();
    trySaveDocument(data, false, [=](const QVariant& vRet){
        //
        reloadNoteData(data);
    });
}

void WizDocumentWebView::editorResetSpellCheck()
{
    QWebEngineProfile *profile = page()->profile();
    if (m_app.userSettings().isEnableSpellCheck()) {
        profile->setSpellCheckEnabled(true);
        profile->setSpellCheckLanguages({"en-US"});
    } else {
        profile->setSpellCheckEnabled(false);
    }
}


void WizDocumentWebView::editorFocus()
{
    page()->runJavaScript("WizEditor.focus();");
    emit focusIn();
}

/**
 * @brief open native rich text edit
 * @param enalbe
 */
void WizDocumentWebView::enableEditor(bool enalbe)
{
    if (enalbe)
    {
        page()->runJavaScript("WizEditor.on();");
        setFocus();
    }
    else
    {
        QString code = QString("WizEditor.off({reader: {type:'%1'}});").arg(getNoteType());
        //
        page()->runJavaScript(code);
        //
        clearFocus();
    }
}

void WizDocumentWebView::setIgnoreActiveWindowEvent(bool igoreEvent)
{
    m_ignoreActiveWindowEvent = igoreEvent;
}

bool WizDocumentWebView::evaluateJavaScript(const QString& js)
{
    page()->runJavaScript(js);
    return true;
}


void WizDocumentWebView::on_insertCommentToNote_request(const QString& docGUID, const QString& comment)
{
    if (docGUID != view()->note().strGUID)
        return;

    if (!isEditing() && WizIsMarkdownNote(view()->note()))
    {
        WizMessageBox::information(this, tr("Info"), tr("Do not support insert comment into markdown note."));
        return;
    }
    //
    // beblow should optimize, for template style note, insert position should not be the front
    if (isEditing())
    {
        QString htmlBody = "<div>" + comment + "</div><hr>";
        htmlBody.replace("\n", "<br>");
        htmlBody.replace("'", "&#39;");
        //
        QString code = QString("var objDiv = document.createElement('div');objDiv.innerHTML= unescape('%1');"
                               "var first=document.body.firstChild;document.body.insertBefore(objDiv,first);").arg(htmlBody);
        //
        page()->runJavaScript(code);
        //
        saveEditingViewDocument(view()->note(), true, [=](const QVariant &){});

    }
    else
    {
        QString commentsText = QUrl::fromPercentEncoding(comment.toUtf8());
        QString commentsHtml = ::WizText2Html(commentsText);
        //
        QString documentGuid = docGUID;
        QString kbGuid = view()->note().strKbGUID;
        //
        ::WizExecutingActionDialog::executeAction(QObject::tr("Saving comments..."), WIZ_THREAD_DEFAULT, [=]{
            //
            WizDatabase& db = m_dbMgr.db(kbGuid);
            //
            WIZDOCUMENTDATA doc;
            if (!db.documentFromGuid(documentGuid, doc))
                return;
            //
            QString strTempPath = ::Utils::WizPathResolve::tempPath() + WizGenGUIDLowerCaseLetterOnly() + "/";
            ::WizEnsurePathExists(strTempPath);
            //
            if (!db.documentToHtmlFile(doc, strTempPath))
                return;
            //
            QString htmlFileName = strTempPath + "index.html";
            if (!QFileInfo::exists(htmlFileName))
                return;
            //
            QString html;
            if (!WizLoadUnicodeTextFromFile(htmlFileName, html))
                return;
            //
            WizHtmlInsertHtmlBeforeAllBodyChildren(html, commentsHtml);
            //
            if (!::WizSaveUnicodeTextToUtf8File(htmlFileName, html, true))
                return;
            //
            db.updateDocumentDataWithFolder(doc, strTempPath, true);
        });
    }
}

void WizDocumentWebView::setWindowVisibleOnScreenShot(bool bVisible)
{
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    if (mainWindow)
    {
        bVisible ? mainWindow->show() : mainWindow->hide();
    }
}

QString WizDocumentWebView::noteResourcesPath()
{
    Q_ASSERT(!m_strNoteHtmlFileName.isEmpty());
    if (m_strNoteHtmlFileName.isEmpty())
        return QString();
    //
    QString path = Utils::WizMisc::extractFilePath(m_strNoteHtmlFileName);
    path += "index_files/";
    Utils::WizMisc::ensurePathExists(path);
    //
    return path;
}

void WizDocumentWebView::insertImage(const QString& strFileName)
{
    QString strHtml;
    if (WizImage2Html(strFileName, strHtml, noteResourcesPath())) {
        editorCommandExecuteInsertHtml(strHtml, true);
    }
}


void WizDocumentWebView::addAttachmentThumbnail(const QString strFile, const QString& strGuid)
{
    QImage img;
    qreal scale = 2;
    ::WizCreateThumbnailForAttachment(img, strFile, QSize(32, 32), scale);
    QString strDestFile = Utils::WizPathResolve::tempPath() + WizGenGUIDLowerCaseLetterOnly() + ".png";
    img.save(strDestFile, "PNG");
    QString strLink = QString("wiz://open_attachment?guid=%1").arg(strGuid);
    QSize szImg = img.size();
    QString strHtml = WizGetImageHtmlLabelWithLink(strDestFile, szImg / scale, strLink);
    editorCommandExecuteInsertHtml(strHtml, true);
}

QString WizDocumentWebView::getNoteType()
{
    return WizGetNoteType(view()->note());
}

bool WizDocumentWebView::isOutline() const
{
    const WIZDOCUMENTDATA& doc = view()->note();
    return doc.strType == "outline";
}

QString WizDocumentWebView::getHighlightKeywords()
{
    const WIZDOCUMENTDATAEX& doc = view()->note();
    CWizStdStringArray texts;
    texts.push_back(doc.strHighlightText);
    texts.push_back(doc.strHighlightTitle);
    //
    std::set<QString> lowerCaseKeywords;
    std::set<QString> keywords;
    for (auto it : texts)
    {
        QString text = it;
        int begin = 0;
        while (true) {
            int start = text.indexOf("<em>",  begin);
            if (start == -1)
                break;
            start += 4;
            int end = text.indexOf("</em>", start);
            if (end == -1)
                break;
            //
            QString keyword = text.mid(start, end - start);
            QString lower = keyword.toLower();
            if (lowerCaseKeywords.find(lower) == lowerCaseKeywords.end())
            {
                lowerCaseKeywords.insert(lower);
                keywords.insert("'" + keyword + "'");
            }
            //
            begin = end + 5;
        }
    }
    //
    CWizStdStringArray arr;
    arr.assign(keywords.begin(), keywords.end());
    //
    CString ret;
    ::WizStringArrayToText(arr, ret, ",");
    return ret;
}


void WizDocumentWebView::onEditorLoadFinished(bool ok)
{
    if (!ok)
        return;
    //
    QString resPath = Utils::WizPathResolve::resourcesPath() + "files/";
    QString editorPath = resPath + "wizeditor/";
    QString lang = "zh-cn";
    QString userGUID = m_dbMgr.db().getUserGuid();
    QString userAlias = m_dbMgr.db().getUserAlias();
    //
    const WIZDOCUMENTDATAEX& doc = view()->note();
    bool ignoreTable = doc.strURL.startsWith("http");
    //
    QString noteType = getNoteType();
    QString keywords = getHighlightKeywords();
    // Initialize rich text editor
    QString strCode = "(async function(){\n";
    strCode += WizFormatString6("await WizEditorInit(\"%1\", \"%2\", \"%3\", \"%4\", %5, \"%6\", false);",
                                       editorPath, lang, userGUID, userAlias,
                                       ignoreTable ? "true" : "false",
                                       noteType);
    if (m_currentEditorMode == modeEditor) {
        // Open rich text editor when doc is in edit mode
        strCode += "WizEditor.on();";
    } else {
        // Close rich text editor when doc is in read mode
        if (keywords.isEmpty()) {
            strCode += "WizEditor.off();";
        } else {
            // When user open document in search results, highlit key words
            QString strCodeMarkdown = QString("WizEditor.off(null, function(){\n\
                WizReader.highlight.on([%1]);\nconsole.log('highlight');\n\
            });").arg(keywords);
            QString strCodeCommon = "await new Promise( (resolve, reject) => {"
                "WizEditor.off(null, () => {"
                    "resolve();"
                "});"
            "});";
            // It's hard to determine when markdown document has rendered.
            // So, keywords highlight in scroll bar is only available for 
            // common document. 
            if (noteType == "common") {
                strCode += strCodeCommon;
            } else {
                strCode += strCodeMarkdown;
            }
        }
    }
    strCode += "\n})()";
    page()->runJavaScript(strCode, [=](const QVariant &v) {
        if (!keywords.isEmpty()) {
            if (noteType == "common") {
                //FIXME: This is not the best way, it needs the keyword in search line
                QString word = keywords.split(",").first()
                                    .section("'", 0, 0, QString::SectionSkipEmpty);
                findText(word);
            }
        }
    });
    // Notify all plugins
    JSPluginManager::instance().handleDocumentChanged();
}

/**
 * @brief handle click event on note page hyperlink
 * @param url
 * @param navigationType
 * @param isMainFrame
 * @param page
 */
void WizDocumentWebView::onEditorLinkClicked(QUrl url, QWebEnginePage::NavigationType navigationType, bool isMainFrame, WizWebEnginePage* page)
{
    if (!isMainFrame)
        return;
    //
    //page->stopCurrentNavigation(); // this will make document page become blank, so it was commented
    //
    if (isInternalUrl(url))
    {
        QString strUrl = url.toString();
        switch (GetWizUrlType(strUrl)) {
        case WizUrl_Document:
            viewDocumentByUrl(strUrl);
            return;
        case WizUrl_Attachment:
            if (!isEditing())
            {
                viewAttachmentByUrl(view()->note().strKbGUID, strUrl);
            }
            return;
        default:
            qDebug() << QString("%1 is a wiz internal url , but we can not identify it");
            return;
        }
    }
    else
    {
        QString strUrl = url.toString();
        if (strUrl.left(12) == "http://file/")
        {
            strUrl.replace(0, 12, "file:/");
        }

        qDebug() << "Open url " << strUrl;
        QDesktopServices::openUrl(strUrl);
        return;
    }
}

bool WizDocumentWebView::isInternalUrl(const QUrl& url)
{
    return url.scheme().toLower() == "wiz";
}

bool WizStringList2Map(const QStringList& list, QMap<QString, QString>& map)
{
    for (int i = 0; i < list.size(); i++) {
        int indx = list[i].indexOf("=");
        if (indx == -1) {
            return false;
        }

        qDebug() << "key: " << list[i].left(indx).toLower();
        qDebug() << "value: " << list[i].mid(indx + 1);

        map.insert(list[i].left(indx).toLower(), list[i].mid(indx + 1));
    }

    return true;
}

void WizDocumentWebView::viewDocumentByUrl(const QString& strUrl)
{
    if (strUrl.isEmpty())
        return;

    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    mainWindow->viewDocumentByWizKMURL(strUrl);
}

/**
 * @brief open attachment through wiz protocol on note page
 * @param strKbGUID
 * @param strUrl
 */
void WizDocumentWebView::viewAttachmentByUrl(const QString& strKbGUID, const QString& strUrl)
{
    if (strUrl.isEmpty())
        return;

    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    mainWindow->viewAttachmentByWizKMURL(strKbGUID, strUrl);
}

void getHtmlBodyStyle(const QString& strHtml, QString& strBodyStyle)
{
    QRegExp regh("<body ([^>]*)>", Qt::CaseInsensitive);
    if (regh.indexIn(strHtml) != -1)
    {
        strBodyStyle = regh.cap(1);
    }
}

/**
 * @brief save editting note
 * @param data note data
 * @param force weather edit force
 * @param callback callback function after save, 
 */
void WizDocumentWebView::saveEditingViewDocument(const WIZDOCUMENTDATA &data, bool force, std::function<void(const QVariant &)> callback)
{
    //FIXME: remove me, just for find a image losses bug.
    Q_ASSERT(!data.strGUID.isEmpty());

    // check note permission
    if (!m_dbMgr.db(data.strKbGUID).canEditDocument(data)) {
        callback(QVariant(false));
        return;
    }

    WIZDOCUMENTDATA doc = data;

    isModified([=](bool modified) {
        if (!force)
        {
            if (!modified)
            {
                // return sucess if not force save and no change 
                callback(QVariant(true));
                return;
            }
        }
        // reset save flag?
        setModified(false);

        QString strFileName = m_mapFile.value(data.strGUID);
        QString strScript = "WizEditor.getContentHtml()";

        TOLOG("saving note...");
        page()->runJavaScript(strScript, [=](const QVariant& ret){
            bool succeeded = false;
            if (ret.type() == QVariant::String)
            {
                QString html = ret.toString();
                if (!html.isEmpty())
                {
                    succeeded = true;
                    m_currentNoteHtml = html;
                    WizSaveUnicodeTextToUtf8File(m_strNoteHtmlFileName, m_currentNoteHtml);
                    emit currentHtmlChanged();
                    Q_EMIT saveDocumentRequested(doc, html, strFileName, 0);
                    TOLOG("save note done...");
                }
            }

            if (!succeeded)
            {
                TOLOG("save note failed, html is empty");
                QString message = tr("Failed to save note.");
                WizMessageBox::warning(this, tr("Error"), message);
            }

            callback(QVariant(succeeded));
        });
    });

}

/**
 * @brief save note under read pattern
 * @param data note data
 * @param force save force
 * @param callback callback after failure or sucess 

 */
void WizDocumentWebView::saveReadingViewDocument(const WIZDOCUMENTDATA &data, bool force, std::function<void(const QVariant &)> callback)
{
    Q_UNUSED(force);
    const WIZDOCUMENTDATA doc = data;

    QString strScript = QString("WizReader.closeDocument();");
    page()->runJavaScript(strScript, [=](const QVariant& vRet) {
        // get return note content
        QString strHtml = vRet.toString();
        // has made change?
        if (!strHtml.isEmpty())
        {
            if (!doc.strGUID.isEmpty())
            {   // get save file name
                QString strFileName = m_mapFile.value(doc.strGUID);
                if (!strFileName.isEmpty())
                {
                    Q_EMIT saveDocumentRequested(doc, strHtml, strFileName, 0);
                }
            }
        }

        callback(true);
    });
}

void WizDocumentWebView::on_insertCodeHtml_requset(QString strOldHtml)
{
    QString strHtml = strOldHtml;
    if (WizGetBodyContentFromHtml(strHtml, false))
    {
        QString name("wiz_code_highlight.css");
        QString strSrcCssFileName = Utils::WizPathResolve::resourcesPath() + "files/code/" + name;
        QString strDestCssFileName = noteResourcesPath() + name;

        if (!strDestCssFileName.isEmpty() && QFile::exists(strDestCssFileName))
        {
            QFile::remove(strDestCssFileName);
        }
        if (!QFile::copy(strSrcCssFileName, strDestCssFileName))
        {
            QMessageBox::critical(this, tr("Error"), tr("Can't copy style files"));
            return;
        }
        //
        QString link = "index_files/" + name;
        //
        page()->runJavaScript(QString("WizAddCssForCode('%1');").arg(link));
        //
        editorCommandExecuteInsertHtml(strHtml, true);
    }
}


//#define DEBUG_EDITOR

/**
 * @brief Get all scripts and style files needed for rich text editor
 * 
 * @param files 
 */
void WizDocumentWebView::getAllEditorScriptAndStyleFileName(std::map<QString, QString>& files)
{
    QString strResourcePath = Utils::WizPathResolve::resourcesPath();
    QString strHtmlEditorPath = strResourcePath + "files/wizeditor/";
    QString strWebEnginePath = strResourcePath + "files/webengine/";
    //
#ifdef DEBUG_EDITOR
    QString strEditorJS = "http://192.168.1.73:8080/libs/wizEditor/wizEditorForMac.js";
    QString strInit = QUrl::fromLocalFile(strHtmlEditorPath + "editorHelper.js").toString();
#else
    QString strEditorJS = QUrl::fromLocalFile(strHtmlEditorPath + "wizEditorForMac.js").toString();
    QString strWebChannelJS = QUrl::fromLocalFile(strWebEnginePath + "wizwebchannel.js").toString();
    QString strInit = QUrl::fromLocalFile(strHtmlEditorPath + "editorHelper.js").toString();
#endif
    //
    files.clear();
    files[strEditorJS] = "";
    files[strWebChannelJS] = "";
    files[strInit] = "";
    //
    /*
     *
     * load note gradually, not need now
    QString tempCss = QUrl::fromLocalFile(strHtmlEditorPath + "tempeditorstyle.css").toString();
    QString tempCssLoadOnly = QUrl::fromLocalFile(strHtmlEditorPath + "tempeditorstyle_loadonly.css").toString();
    //
    files[tempCss] = "wiz_unsave_style";
    files[tempCssLoadOnly] = "wiz_style_for_load";
    */

}



void WizDocumentWebView::insertScriptAndStyleCore(QString& strHtml, const std::map<QString, QString>& files)
{
    Q_ASSERT(!files.empty());
    for (std::map<QString, QString>::const_iterator it = files.begin(); it != files.end(); it++)
    {
        QString url = it->first;
        QString strFileName = QUrl(url).toLocalFile();
        QString name = it->second;
        //
#ifndef DEBUG_EDITOR
        if (!strFileName.isEmpty())
        {
            Q_ASSERT(WizPathFileExists(strFileName));
        }
#endif
        //
        QString strExt = Utils::WizMisc::extractFileExt(strFileName);
        if (strExt.isEmpty())
        {
            strExt = Utils::WizMisc::extractFileExt(url);
        }
        //
        if (0 == strExt.compare(".css", Qt::CaseInsensitive))
        {
            if (name.isEmpty()) {
                name = "wiz_inner_style";
            }
            //
            QString strTag = WizFormatString2(
                        "<link rel=\"stylesheet\" type=\"text/css\" href=\"%1\" name=\"%2\" wiz_style=\"unsave\" charset=\"utf-8\">",
                        url, name);
            //
            if (strHtml.indexOf(strTag) == -1)
            {
                WizHTMLAppendTextInHead(strTag, strHtml);
            }
        }
        else if (0 == strExt.compare(".js", Qt::CaseInsensitive))
        {
            if (name.isEmpty()) {
                name = "wiz_inner_script";
            }
            QString	strTag = WizFormatString2(
                    "<script type=\"text/javascript\" src=\"%1\" name=\"%2\" wiz_style=\"unsave\" charset=\"utf-8\"></script>",
                    url, name);
            //
            if (strHtml.indexOf(strTag) == -1)
            {
                WizHTMLAppendTextInHead(strTag, strHtml);
            }
        }
        else
        {
            Q_ASSERT(false);
        }
    }
    //
    QString strTemplateJsFileName = ::Utils::WizPathResolve::wizTemplateJsFilePath();
    if (QFileInfo(strTemplateJsFileName).exists())
    {
        strTemplateJsFileName = QUrl::fromLocalFile(strTemplateJsFileName).toString();
        QString strTag = QString(
            "<script type=\"text/javascript\" src=\"%1\" wiz_style=\"unsave\" charset=\"utf-8\"></script>")
            .arg(strTemplateJsFileName);
        
        WizHTMLAppendTextInHead(strTag, strHtml);
    }
}


void WizDocumentWebView::addDefaultScriptsToDocumentHtml(QString htmlFileName)
{
    QString strHtml;
    bool ret = WizLoadUnicodeTextFromFile(htmlFileName, strHtml);
    if (!ret) {
        // hide client and show error
        return;
    }

    m_currentNoteHtml = strHtml;
    emit currentHtmlChanged();

    std::map<QString, QString> files;
    getAllEditorScriptAndStyleFileName(files);
    insertScriptAndStyleCore(strHtml, files);

    replaceDefaultCss(strHtml);

    ::WizSaveUnicodeTextToUtf8File(htmlFileName, strHtml, true);

}


/**
 * @brief read text and insert style table and insert rich editter script from note file, then load changed html to page
 * @param editorMode
 */
void WizDocumentWebView::loadDocumentInWeb(WizEditorMode editorMode)
{
    QString strGUID = view()->note().strGUID;
    QString strFileName = m_mapFile.value(strGUID);
    if (strFileName.isEmpty()) {
        return;
    }
    //
    QString strHtml;
    bool ret = WizLoadUnicodeTextFromFile(strFileName, strHtml);
    if (!ret) {
        // hide client and show error
        return;
    }

    addDefaultScriptsToDocumentHtml(strFileName);

    WizEditorMode oldMode = m_currentEditorMode;
    m_currentEditorMode = editorMode;
    if (oldMode != m_currentEditorMode)
    {
        if (m_currentEditorMode == modeEditor) {
            Q_EMIT focusIn();
        } else {
            Q_EMIT focusOut();
        }

        view()->titleBar()->setEditorMode(m_currentEditorMode);
    }

    m_strNoteHtmlFileName = strFileName;
    load(QUrl::fromLocalFile(strFileName));

    onNoteLoadFinished();
}

void WizDocumentWebView::onNoteLoadFinished()
{
    WizGlobal::instance()->emitViewNoteLoaded(view(), view()->note(), true);
}

/*!
    Set the \a mode of editor and try to save document before switching mode.
    Editor will try to save document.
*/
void WizDocumentWebView::setEditorMode(WizEditorMode mode)
{
    if (m_currentEditorMode == mode)
        return;

    bool editing = mode == modeEditor;

    // show editor toolbar properly
    if (!editing && hasFocus()) {
        Q_EMIT focusOut();
    }

    if (editing && hasFocus()) {
        Q_EMIT focusIn();
    }

    WIZDOCUMENTDATA docData;
    WizDatabase& db = m_dbMgr.db(view()->note().strKbGUID);
    if (!db.documentFromGuid(view()->note().strGUID, docData))
        return;

    trySaveDocument(docData, false, [=](const QVariant&){
        enableEditor(editing);
        if (editing) {
            setFocus(Qt::MouseFocusReason);
            editorFocus();
        }
    });

    m_currentEditorMode = mode;

    if (m_currentEditorMode == modeEditor) {
        m_timerAutoSave.start();
    } else {
        m_timerAutoSave.stop();
    }
}

/**
 * @brief save note
 * @param data note data
 * @param force save force
 * @param callback callback if fail
 */
void WizDocumentWebView::trySaveDocument(const WIZDOCUMENTDATA& data, bool force, std::function<void(const QVariant &)> callback)
{
    if (!view()->noteLoaded())  //encrypting note & has been loaded
    {
        // callback after fail
        callback(QVariant(false));
        return;
    }

    if (m_currentEditorMode == modeEditor)
    {
        // save note under editable pattern
        saveEditingViewDocument(data, force, callback);
    }
    else
    {
        // save note under read pattern
        saveReadingViewDocument(data, force, callback);
    }
}

void WizDocumentWebView::editorCommandQueryCommandValue(const QString& strCommand, std::function<void(const QString& value)> callback)
{
    QString script = "document.queryCommandValue('" + strCommand +"');";
    page()->runJavaScript(script, [=](const QVariant& ret){
        //
        callback(ret.toString());
        //
    });
}

void WizDocumentWebView::editorCommandQueryCommandState(const QString& strCommand, std::function<void(int state)> callback)
{
    QString script = "document.queryCommandState('" + strCommand +"');";
    page()->runJavaScript(script, [=](const QVariant& ret){
        //
        callback(ret.toInt());
        //
    });
}

/*
 * Execute command and also save status to undostack.
 * All commands execute from client which may modify document MUST invoke this
 * instead of use frame's evaluateJavascript.
 */
void WizDocumentWebView::editorCommandExecuteCommand(const QString& strCommand,
                                                      const QString& arg1 /* = QString() */,
                                                      const QString& arg2 /* = QString() */,
                                                      const QString& arg3 /* = QString() */)
{
    QString strExec = QString("WizEditor.execCommand('%1'").arg(strCommand);
    if (!arg1.isEmpty()) {
        strExec += ", " + arg1;
    }

    if (!arg2.isEmpty()) {
        strExec += ", " + arg2;
    }

    if (!arg3.isEmpty()) {
        strExec += ", " + arg3;
    }

    strExec += ");";

    qDebug() << strExec;

    page()->runJavaScript(strExec);
    //
    setModified(true);
}


bool WizDocumentWebView::editorCommandQueryMobileFileReceiverState()
{
    return m_app.userSettings().receiveMobileFile();
}

void WizDocumentWebView::editorCommandExecuteParagraph(const QString& strType)
{
    WizGetAnalyzer().logAction("editorParagraph");
    editorCommandExecuteCommand("formatBlock", "false", "'" + strType + "'");
}

void WizDocumentWebView::editorCommandExecuteInsertHtml(const QString& strHtml, bool bNotSerialize)
{
    QString s = bNotSerialize ? "true" : "false";
    //
    QString base64Html = WizStringToBase64(strHtml);
    base64Html.replace("\r", "");
    base64Html.replace("\n", "");
    QString code = QString("WizEditor.insertB64Html('%1')").arg(base64Html);
    //
    page()->runJavaScript(code);
    //editorCommandExecuteCommand("insertHtml", s, "'" + strHtml + "'");
}


void WizDocumentWebView::editorCommandExecutePastePlainText()
{
    QClipboard* clip = QApplication::clipboard();
    if (!clip)
        return;
    const QMimeData* data = clip->mimeData();
    if (!data)
        return;
    QString text = data->text();

    QString base64Text = WizStringToBase64(text);
    QString js = QString("WizEditor.pasteB64('', '%1')").arg(base64Text);
    page()->runJavaScript(js);
}

void WizDocumentWebView::editorCommandExecuteIndent()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("indent");
    editorCommandExecuteCommand("indent");
}

void WizDocumentWebView::editorCommandExecuteOutdent()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("outdent");
    editorCommandExecuteCommand("outdent");
}

void WizDocumentWebView::editorCommandExecuteLinkInsert()
{
    if (!m_editorInsertLinkForm) {
        m_editorInsertLinkForm = new WizEditorInsertLinkForm(window());
        connect(m_editorInsertLinkForm, SIGNAL(accepted()), SLOT(on_editorCommandExecuteLinkInsert_accepted()));
    }
    //
    page()->runJavaScript("WizEditor.link.getCurrentLink();", [=](const QVariant& vLink){
        //
        QString strUrl = vLink.toString();
        if (strUrl.isEmpty()) {
            strUrl = "http://";
        }
        //
        m_editorInsertLinkForm->setUrl(strUrl);

        m_editorInsertLinkForm->exec();

        WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
        analyzer.logAction("linkInsert");
        //
    });
}

void WizDocumentWebView::on_editorCommandExecuteLinkInsert_accepted()
{
    // append http if not exist
    QString strUrl = m_editorInsertLinkForm->getUrl();
    if (strUrl.isEmpty() || strUrl == "http://") {
        //
        QString code = "WizEditor.link.removeSelectedLink();";
        page()->runJavaScript(code);
        //
    } else {
        QUrl url(strUrl);
        if (url.scheme().isEmpty())
        {
            strUrl = "http://" + strUrl;
        }
        else
        {
            strUrl = url.toString();
        }
        //
        QString code = QString("WizEditor.link.setCurrentLink('%1');").arg(strUrl);
        //
        page()->runJavaScript(code);
    }
}

void WizDocumentWebView::editorCommandExecuteLinkRemove()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("unlink");
    editorCommandExecuteCommand("unlink");
}

void WizDocumentWebView::editorCommandExecuteFindReplace()
{
    if (!m_searchReplaceWidget)
    {
        m_searchReplaceWidget = new WizSearchReplaceWidget(this);
        //
        connect(m_searchReplaceWidget, SIGNAL(findPre(QString,bool)), SLOT(findPre(QString,bool)));
        connect(m_searchReplaceWidget, SIGNAL(findNext(QString,bool)), SLOT(findNext(QString,bool)));
        connect(m_searchReplaceWidget, SIGNAL(replaceCurrent(QString,QString)), SLOT(replaceCurrent(QString,QString)));
        connect(m_searchReplaceWidget, SIGNAL(replaceAndFindNext(QString,QString,bool)), SLOT(replaceAndFindNext(QString,QString,bool)));
        connect(m_searchReplaceWidget, SIGNAL(replaceAll(QString,QString,bool)), SLOT(replaceAll(QString,QString,bool)));
    }

    QRect rect = geometry();
    QPoint leftTop = pos();
    if (parentWidget())
        leftTop = parentWidget()->mapToGlobal(leftTop);
    rect.moveTo(leftTop);
    m_searchReplaceWidget->showInEditor(rect);
    m_searchReplaceWidget->setSourceText(selectedText());

    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("findReplace");

}

void WizDocumentWebView::innerFindText(QString text, bool next, bool matchCase)
{
    bool back = !next;
    QString code = QString("find('%1', %2, %3, true)").arg(text)
            .arg(back ? "true" : "false")
            .arg(matchCase ? "true" : "false");
    page()->runJavaScript(code);

}

static QString strOldSearchText = "";
static bool strOldCase = false;
void WizDocumentWebView::findPre(QString strTxt, bool bCasesensitive)
{
    //FIXME:  there is a problem here, HighlightAllOccurrences can not be used togethor with find one.
    if (strOldSearchText != strTxt || strOldCase != bCasesensitive)
    {
        // clear highlight
        //findText("");
        strOldSearchText = strTxt;
        strOldCase = bCasesensitive;
    }
    QWebEnginePage::FindFlags options;
    options |= QWebEnginePage::FindBackward;
    if (bCasesensitive)
    {
        options |= QWebEnginePage::FindCaseSensitively;
    }
    //
    findText(strTxt, options);
    //innerFindText(strTxt, false, bCasesensitive);
}

void WizDocumentWebView::findNext(QString strTxt, bool bCasesensitive)
{
    if (strOldSearchText != strTxt || strOldCase != bCasesensitive)
    {
        //findText("", 0);
        strOldSearchText = strTxt;
        strOldCase = bCasesensitive;
    }
    QWebEnginePage::FindFlags options;
    if (bCasesensitive)
    {
        options |= QWebEnginePage::FindCaseSensitively;
    }
    //
    findText(strTxt, options);
    //innerFindText(strTxt, true, bCasesensitive);
}

void WizDocumentWebView::replaceCurrent(QString strSource, QString strTarget)
{
    QString strExec = QString("WizEditor.replace('%1', '%2', true)").arg(strSource).arg(strTarget);
    page()->runJavaScript(strExec);
}

void WizDocumentWebView::replaceAndFindNext(QString strSource, QString strTarget, bool bCasesensitive)
{
    QString strExec = QString("WizEditor.replace('%1', '%2', %3)").arg(strSource).arg(strTarget).arg(bCasesensitive ? "true" : "false");
    page()->runJavaScript(strExec);
}

void WizDocumentWebView::replaceAll(QString strSource, QString strTarget, bool bCasesensitive)
{
    QString strExec = QString("WizEditor.replaceAll('%1', '%2', %3)").arg(strSource).arg(strTarget).arg(bCasesensitive ? "true" : "false");
    page()->runJavaScript(strExec);
    setModified(true);
}

void WizDocumentWebView::editorCommandExecuteFontFamily(const QString& strFamily)
{
    WizGetAnalyzer().logAction(QString("editorSetFontFamily : %1").arg(strFamily));
    editorCommandExecuteCommand("fontName", "false", "'" + strFamily + "'");
}

void WizDocumentWebView::editorCommandExecuteFontSize(const QString& strSize)
{
    WizGetAnalyzer().logAction(QString("editorSetFontSize : %1pt").arg(strSize));
    //
    CString strStyle = WizFormatString1("{\\\"font-size\\\" : \\\"%1pt\\\"}", strSize);
    CString strScript = WizFormatString1("WizEditor.modifySelectionDom(JSON.parse(\"%1\"))", strStyle);

    page()->runJavaScript(strScript);
    setModified(true);
}

void WizDocumentWebView::editorCommandExecuteBackColor(const QColor& color)
{
    if (color == QColor(Qt::transparent)) {
       //editorCommandExecuteCommand("backColor", "false", "'default'");
    }
    else {
        editorCommandExecuteCommand("backColor", "false", "'" + color.name() + "'");
    }
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("backColor");
}

void WizDocumentWebView::editorCommandExecuteForeColor(const QColor& color)
{
    if (color == QColor(Qt::transparent)) {
        //editorCommandExecuteCommand("foreColor", "false", "'default'");
    }
    else {
        editorCommandExecuteCommand("foreColor", "false", "'" + color.name() + "'");
    }
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("foreColor");
}

void WizDocumentWebView::editorCommandExecuteBold()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("bold");
    editorCommandExecuteCommand("bold");
}

void WizDocumentWebView::editorCommandExecuteItalic()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("italic");
    editorCommandExecuteCommand("italic");
}

void WizDocumentWebView::editorCommandExecuteUnderLine()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("underline");
    editorCommandExecuteCommand("underline");
}

void WizDocumentWebView::editorCommandExecuteStrikeThrough()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("strikethrough");
    editorCommandExecuteCommand("strikethrough");
}

void WizDocumentWebView::editorCommandExecuteSubScript()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("subscript");
    editorCommandExecuteCommand("subscript");
}

void WizDocumentWebView::editorCommandExecuteSuperScript()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("superscript");
    editorCommandExecuteCommand("superscript");
}

void WizDocumentWebView::editorCommandExecuteJustifyLeft()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("justifyLeft");
    editorCommandExecuteCommand("JustifyLeft");
}

void WizDocumentWebView::editorCommandExecuteJustifyRight()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("justifyRight");
    editorCommandExecuteCommand("JustifyRight");
}

void WizDocumentWebView::editorCommandExecuteJustifyCenter()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("justifyCenter");
    editorCommandExecuteCommand("JustifyCenter");
}

void WizDocumentWebView::editorCommandExecuteJustifyJustify()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("justifyJustify");
    editorCommandExecuteCommand("justify", "'justify'");
}

void WizDocumentWebView::editorCommandExecuteInsertOrderedList()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("insertOrderedList");
    editorCommandExecuteCommand("insertOrderedList");
}

void WizDocumentWebView::editorCommandExecuteInsertUnorderedList()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("insertUnorderedList");
    editorCommandExecuteCommand("insertUnorderedList");
}

void WizDocumentWebView::editorCommandExecuteTableInsert(int row, int col)
{
    QString code = QString("WizEditor.table.insertTable(%1, %2);").arg(col).arg(row);
    //
    page()->runJavaScript(code);
}


void WizDocumentWebView::on_editorCommandExecuteScreenShot_imageAccepted(QPixmap pix)
{
    QObject *ssSender = qobject_cast<QObject*>(sender());
    if (ssSender)
        delete ssSender;

    setWindowVisibleOnScreenShot(true);

    if (pix.isNull())
        return;

    CString strFileName = noteResourcesPath() + WizIntToStr(WizGetTickCount()) + ".png";
    if (!pix.save(strFileName)) {
        TOLOG("ERROR: Can't save clipboard image to file");
        return;
    }

    insertImage(strFileName);
}

void WizDocumentWebView::on_editorCommandExecuteScreenShot_finished()
{
    QObject *ssSender = qobject_cast<QObject*>(sender());
    if (ssSender)
        delete ssSender;

    setWindowVisibleOnScreenShot(true);
}

void WizDocumentWebView::editorCommandExecuteInsertHorizontal()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("insertHorizontal");
    editorCommandExecuteCommand("InsertHorizontalRule");
}

void WizDocumentWebView::editorCommandExecuteInsertCheckList()
{
    QString strExec = "WizEditor.todo.setTodo();";
    page()->runJavaScript(strExec);

    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("insertCheckList");
}

void WizDocumentWebView::editorCommandExecuteInsertImage()
{
    static QString initPath = QDir::homePath();
    QStringList strImgFileList = QFileDialog::getOpenFileNames(0, tr("Image File"), initPath, tr("Images (*.png *.bmp *.gif *.jpg)"));
    if (strImgFileList.isEmpty())
        return;
    //
    QString strImagePath = noteResourcesPath();

    CWizStdStringArray files;
    foreach (QString strImgFile, strImgFileList)
    {
        QString destImageFileName = strImagePath + ::WizGenGUIDLowerCaseLetterOnly()
                + Utils::WizMisc::extractFileExt(strImgFile);
        //
        if (QFile::copy(strImgFile, destImageFileName))
        {
            files.push_back(QUrl::fromLocalFile(destImageFileName).toString());
        }
        //
        initPath = Utils::WizMisc::extractFilePath(strImgFile);
    }
    //
    CString param;
    WizStringArrayToText(files, param, "*");
    //WizEditor.img.insertByPath('./img01.jpg');
    QString script = QString("WizEditor.img.insertByPath('%1');").arg(param);
    page()->runJavaScript(script);

    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("insertImage");
}


void WizDocumentWebView::editorCommandExecuteStartMarkup()
{
    QString js = QString("WizEditor.marker.start();");
    page()->runJavaScript(js);
}


void WizDocumentWebView::editorCommandExecuteStopMarkup()
{
    QString js = QString("WizEditor.marker.quit();");
    page()->runJavaScript(js);
}


void WizDocumentWebView::editorExecJs(QString js)
{
    page()->runJavaScript(js);
}

void WizDocumentWebView::onViewMindMap(bool on)
{
    if (on) {
        QString title = view()->note().strTitle;
        QString js = QString("WizEditor.outline.showMinder(`%1`);").arg(title.replace("`", "\\`"));
        editorExecJs(js);
    } else {
        QString js = "WizEditor.outline.hideMinder();";
        editorExecJs(js);
    }
}

void WizDocumentWebView::editorCommandExecuteInsertPainter()
{
    view()->changeType("svgpainter");
    QString js = QString("WizEditor.createSvg()");
    page()->runJavaScript(js);
}


void WizDocumentWebView::editorCommandExecuteInsertDate()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("insertDate");
    //
    QString date = QDate::currentDate().toString(Qt::DefaultLocaleLongDate);
    editorCommandExecuteInsertHtml(date, false);
}

void WizDocumentWebView::editorCommandExecuteInsertTime()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("insertTime");
    //
    QString time = QTime::currentTime().toString(Qt::DefaultLocaleLongDate);
    editorCommandExecuteInsertHtml(time, false);
}

void WizDocumentWebView::editorCommandExecuteRemoveFormat()
{
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("removeFormat");
    editorCommandExecuteCommand("removeFormat");
}

void WizDocumentWebView::editorCommandExecuteFormatPainterOn(bool multi)
{
    QString script = QString("WizEditor.formatPainter.on(%1);").arg(multi ? "true" : "false");
    //
    page()->runJavaScript(script, [=] (const QVariant& vRet) {
        if (vRet.type() == QVariant::Bool) {
            if (!vRet.toBool()) {
                //
                TOLOG("Can't stsart format painter");

            }
        }

    });
    //
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("formatPainter");
}

void WizDocumentWebView::editorCommandExecuteFormatPainterOff()
{
    QString script = QString("WizEditor.formatPainter.off();");
    //
    page()->runJavaScript(script, [=] (const QVariant& vRet) {
        if (vRet.type() == QVariant::Bool) {
            if (!vRet.toBool()) {
                //
                TOLOG("Can't stsart format painter");

            }
        }

    });
    //
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("formatPainter");
}

void WizDocumentWebView::editorCommandExecuteInsertCode()
{
    page()->runJavaScript("WizEditor.code.insertCode();");
    //
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("insertCode");
}

void WizDocumentWebView::editorCommandExecuteMobileImage(bool bReceiveImage)
{
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    if (bReceiveImage && m_app.userSettings().needShowMobileFileReceiverUserGuide())
    {
        mainWindow->showMobileFileReceiverUserGuide();
    }

    m_app.userSettings().setReceiveMobileFile(bReceiveImage);
    mainWindow->setMobileFileReceiverEnable(bReceiveImage);

    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("mobileImage");
}

void WizDocumentWebView::editorCommandExecuteScreenShot()
{
    WizScreenShotHelper* helper = new WizScreenShotHelper();

    connect(helper, SIGNAL(screenShotCaptured(QPixmap)),
            SLOT(on_editorCommandExecuteScreenShot_imageAccepted(QPixmap)));
    connect(helper, SIGNAL(shotScreenQuit()), SLOT(on_editorCommandExecuteScreenShot_finished()));

    setWindowVisibleOnScreenShot(false);
    QTimer::singleShot(200, helper, SLOT(startScreenShot()));

    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    analyzer.logAction("screenShot");
}


void WizDocumentWebView::saveAsPDF()
{
    CString strTitle = view()->note().strTitle;
    WizMakeValidFileNameNoPath(strTitle);
    static QString strInitPath = QDir::homePath();
    QString strInitFileName = Utils::WizMisc::addBackslash2(strInitPath) + Utils::WizMisc::extractFileTitle(strTitle);
    //
    QString strFileName = QFileDialog::getSaveFileName(this, QString(),
                                                       strInitFileName,
                                                       tr("PDF Files (*.pdf)"));
    //
    if (strFileName.isEmpty())
        return;
    //
    strInitPath = Utils::WizMisc::extractFilePath(strFileName);
    //
    if (::WizPathFileExists(strFileName))
    {
        ::WizDeleteFile(strFileName);
    }
    //
    QPrinter::Unit marginUnit =  (QPrinter::Unit)m_app.userSettings().printMarginUnit();
    double marginTop = m_app.userSettings().printMarginValue(wizPositionTop);
    double marginBottom = m_app.userSettings().printMarginValue(wizPositionBottom);
    double marginLeft = m_app.userSettings().printMarginValue(wizPositionLeft);
    double marginRight = m_app.userSettings().printMarginValue(wizPositionRight);
    QMarginsF margins(marginLeft, marginTop, marginRight, marginBottom);
    //
    const QPageLayout layout = QPageLayout(QPageSize(QPageSize::A4), QPageLayout::Portrait, margins);
    //
    page()->printToPdf(strFileName, layout);
}

void WizDocumentWebView::saveAsMarkdown()
{
    // prepare file and dir
    CString strTitle = view()->note().strTitle;
    WizMakeValidFileNameNoPath(strTitle);
    static QString strInitPath = QDir::homePath();
    QString strInitFileName = Utils::WizMisc::addBackslash2(strInitPath) + strTitle;
    // destination file address
    QString strIndexFileName = QFileDialog::getSaveFileName(this, QString(),
                                                       strInitFileName,
                                                       tr("Markdown Files (*.md)"));
    saveAsMarkdown(strIndexFileName);
}

void WizDocumentWebView::saveAsMarkdown(QString& strIndexFileName, bool bSaveResource)
{
    Q_UNUSED(bSaveResource);
    if (strIndexFileName.isEmpty())
        return;
    //
    if (::WizPathFileExists(strIndexFileName))
    {
        ::WizDeleteFile(strIndexFileName);
    }
    // prepare note data
    const WIZDOCUMENTDATA& doc = view()->note();
    WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
    // 导出HTML和资源文件到目标文件地址
    if (!db.exportToHtmlFile(doc, strIndexFileName)) {
        return;
    }
    // cannot be standard markdown for inner contanined table, todo, picture
    page()->runJavaScript(QString("WizEditor.getMarkdownSrc({unEscapeHtml: true});"), [=](const QVariant& vModified){
        //
        QString source = vModified.toString();
        //
        QString fileTitle = Utils::WizMisc::extractFileTitle(strIndexFileName);

        QString strResFolder = fileTitle.toHtmlEscaped() + "_files/";
        source.replace("index_files/", strResFolder);
        //
        ::WizSaveUnicodeTextToUtf8File(strIndexFileName, source, false);
    });
}

void WizDocumentWebView::saveAsPlainMarkdown(QString& destFileName, std::function<void(QString fileName)> callback)
{
    if (destFileName.isEmpty())
        return;
    //
    if (::WizPathFileExists(destFileName))
    {
        ::WizDeleteFile(destFileName);
    }
    page()->runJavaScript(QString("WizEditor.getMarkdownSrc({unEscapeHtml: true});"), [=](const QVariant& vModified){
        //
        QString source = vModified.toString();
        //
        QString fileTitle = Utils::WizMisc::extractFileTitle(destFileName);
        //
        if(::WizSaveUnicodeTextToUtf8File(destFileName, source, false))
            callback(destFileName);
    });
}

void WizDocumentWebView::saveAsPlainText(QString& destFileName, std::function<void(QString fileName)> callback)
{
    // prepare note data
    WIZDOCUMENTDATA docData;
    WizDatabase& db = m_dbMgr.db(view()->note().strKbGUID);
    if (!db.documentFromGuid(view()->note().strGUID, docData))
        return;
    trySaveDocument(docData, false, [=](const QVariant&){
        //FIXME: cannot asure note temp file is newest
        // make sure current donot have file
        if (destFileName.isEmpty())
            return;
        //
        if (::WizPathFileExists(destFileName))
        {
            ::WizDeleteFile(destFileName);
        }
        // read html
        QString strHtml;
        QString strFileName = m_mapFile.value(docData.strGUID);
        bool ret = WizLoadUnicodeTextFromFile(strFileName, strHtml);
        if (!ret)
        {
            return;
        }
        // get pure text
        QTextDocument doc;
        doc.setHtml(strHtml);
        QString strText = doc.toPlainText(); //auto deHtmlEscaped
        strText.replace("&nbsp", " "); // older version exsits unnomarlly &ngsp, so cannot be jump by QTextDocument
        // write file 
        if(WizSaveUnicodeTextToUtf8File(destFileName, strText, false))
        {
            callback(destFileName);
        }
    });
}

/**
 * @brief save note be html, include resource file
 */
void WizDocumentWebView::saveAsHtml()
{
    CString strTitle = view()->note().strTitle;
    WizMakeValidFileNameNoPath(strTitle);
    strTitle = Utils::WizMisc::extractFileTitle(strTitle);
    //
    static QString strInitPath = QDir::homePath();
    QString strInitFileName = Utils::WizMisc::addBackslash2(strInitPath) + Utils::WizMisc::extractFileTitle(strTitle);
    //
    QString strIndexFileName = QFileDialog::getSaveFileName(this, QString(),
                                                       strInitFileName,
                                                       tr("Html Files (*.html)"));
    //
    if (strIndexFileName.isEmpty())
        return;
    //
    strInitPath = Utils::WizMisc::extractFilePath(strIndexFileName);
    //
    const WIZDOCUMENTDATA& doc = view()->note();
    WizDatabase& db = m_dbMgr.db(doc.strKbGUID);
    //
    if (!db.exportToHtmlFile(doc, strIndexFileName)) {
        return;
    }
    //
    if (WizIsMarkdownNote(doc))
    {
        QString strScript = QString("WizReader.getRenderDocument();");
        page()->runJavaScript(strScript, [=](const QVariant& vRet) {
            //
            QString strHtml = vRet.toString();
            //
            if (!strHtml.isEmpty())
            {
                QString fileTitle = Utils::WizMisc::extractFileTitle(strIndexFileName);
                QString resourcePath = fileTitle + "_files/";
                strHtml.replace("index_files/", resourcePath);
                //
                ::WizSaveUnicodeTextToUtf8File(strIndexFileName, strHtml, true);
            }
            //
        });

    }
}

/**
 * @brief save note be rendered html, exclude resource file
 * @param destFileName
 * @param callback
 */
void WizDocumentWebView::saveAsRenderedHtml(QString& destFileName, std::function<void(QString fileName)> callback)
{
    //FIXME: should trimmed Wiz's Script and Style.
    //Script: name="wiz_inner_script", wiz_style="unsave", src="contains: odemirror"
    //Style: id="wiz_custom_css", id="wiz_tmp_style_reader_common", name="wiz_tmp_editor_style"
    //          id="wiz_tmp_style_reader_block_scroll", id="wiz_tmp_style_code_common", name="wiz_tmp_editor_style"
    //          id="wiz_tmp_style_code_reader"
    //Tags: wiz_tmp_tag
    // not need remove those staff, for need rendered html page
    page()->toHtml([=](const QString& strHtml){
        //
        if(::WizSaveUnicodeTextToUtf8File(destFileName, strHtml, false))
            callback(destFileName);
    });
}

/**
 * @brief is note content modified
 * @param callback callback to judge wether saved succefully
 */
void WizDocumentWebView::isModified(std::function<void(bool modified)> callback)
{
    if (m_bContentsChanged)
    {
        callback(true);
        return;
    }
    // judge modify through native js edit function
    page()->runJavaScript(QString("WizEditor.isModified();"), [=](const QVariant& vModified){
        //
        callback(vModified.toBool());

    });
}

/**
 * @brief set note content changed
 * @param b changed?
 */
void WizDocumentWebView::setModified(bool b)
{
    m_bContentsChanged = b;
    page()->runJavaScript("WizEditor.setUnModified()");
}

void WizDocumentWebView::undo()
{
    page()->runJavaScript("WizEditor.undo()");
}

void WizDocumentWebView::redo()
{
    page()->runJavaScript("WizEditor.redo()");
}

QString WizDocumentWebView::getUserGuid()
{
    return m_dbMgr.db().getUserGuid();
}


QString WizDocumentWebView::getUserAvatarFilePath()
{
    int size = 24;
    QString strFileName;
    QString strUserID = m_dbMgr.db().getUserId();
    if (WizAvatarHost::customSizeAvatar(strUserID, size, size, strFileName))
        return strFileName;


    return QString();
}

QString WizDocumentWebView::getUserAlias()
{
    QString strKbGUID = view()->note().strKbGUID;
    return m_dbMgr.db(strKbGUID).getUserAlias();
}


bool WizDocumentWebView::isPersonalDocument()
{
    QString strKbGUID = view()->note().strKbGUID;
    QString dbKbGUID = m_dbMgr.db().kbGUID();
    return strKbGUID.isEmpty() || (strKbGUID == dbKbGUID);
}

QString WizDocumentWebView::getCurrentNoteHtml()
{
    return m_currentNoteHtml;
}


void copyFileToFolder(const QString& strFileFolder, const QString& strIndexFile, \
                         const QStringList& strResourceList)
{
    //copy index file
    QString strFolderIndex = strFileFolder + "index.html";
    if (strIndexFile != strFolderIndex)
    {
        QFile::remove(strFolderIndex);
        QFile::copy(strIndexFile, strFolderIndex);
    }

    //copy resources to temp folder
    QString strResourcePath = strFileFolder + "index_files/";
    for (int i = 0; i < strResourceList.count(); i++)
    {
        if (QFile::exists(strResourceList.at(i)))
        {
            QFile::copy(strResourceList.at(i), strResourcePath + Utils::WizMisc::extractFileName(strResourceList.at(i)));
        }
    }
}

bool WizDocumentWebView::hasEditPermissionOnCurrentNote()
{
    WIZDOCUMENTDATA docData = view()->note();
    WizDatabase& db = m_dbMgr.db(docData.strKbGUID);
    return db.canEditDocument(docData) && !WizDatabase::isInDeletedItems(docData.strLocation);
}

void WizDocumentWebView::changeCurrentDocumentType(const QString &strType)
{
    WIZDOCUMENTDATA docData = view()->note();
    WizDatabase& db = m_dbMgr.db(docData.strKbGUID);
    docData.strType = strType;
    db.modifyDocumentInfoEx(docData);
}

bool WizDocumentWebView::checkListClickable()
{
    WizDocumentView* v = view();
    if (!m_dbMgr.db(v->note().strKbGUID).isGroup())
    {
        emit clickingTodoCallBack(false, false);
        return true;
    }

    if (v->checkListClickable())
    {
        emit clickingTodoCallBack(false, false);
        v->setStatusToEditingByCheckList();
        return true;
    }
    emit clickingTodoCallBack(true, true);
    return false;
}

bool WizDocumentWebView::shouldAddCustomCSS()
{
    const WIZDOCUMENTDATA& data = view()->note();
    // note from web page canot add self define style 
    bool styledNote = data.strURL.startsWith("http");
    if (styledNote)
        return false;

    bool isMarkdown = WizIsMarkdownNote(data) && !view()->isEditing();

    return !isMarkdown;
}

bool WizDocumentWebView::canRenderMarkdown()
{
    const WIZDOCUMENTDATA& doc = view()->note();

    if (view()->isEditing())
        return false;

    return WizIsMarkdownNote(doc);
}

bool WizDocumentWebView::canEditNote()
{
    return view()->isEditing();
}

// used in editor initialize
QString WizDocumentWebView::getLocalLanguage()
{
    QLocale locale;
    if (locale.language() == QLocale::Chinese)
    {
        if (locale.country() == QLocale::China)
            return "zh-cn";
        else if (locale.country() == QLocale::Taiwan)
            return "zh-tw";
    }

    return "en";
}

void WizDocumentWebView::onSelectionChange(const QString& currentStyle)
{
    Q_EMIT statusChanged(currentStyle);
}


void WizDocumentWebView::onClickedSvg(const QString& data)
{
    const WIZDOCUMENTDATA note = view()->note();
    //
    trySaveDocument(note, true, [=] (const QVariant) {
        //
        ::WizExecuteOnThread(WIZ_THREAD_MAIN, [=] {
            //
            editHandwritingNote(m_dbMgr, note, m_strNoteHtmlFileName, data, WizMainWindow::instance());
            addDefaultScriptsToDocumentHtml(m_strNoteHtmlFileName);
            load(QUrl::fromLocalFile(m_strNoteHtmlFileName));
        });
    });

}

void WizDocumentWebView::updateSvg(QString data)
{
    //
    data = data.replace("\\", "\\\\");
    QString js = QString("WizEditor.replaceSvg(`%1`)").arg(data);
    page()->runJavaScript(js);
    //
}


void WizDocumentWebView::saveCurrentNote()
{
    WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
        //
        onTimerAutoSaveTimout();
        //
    });
}

void WizDocumentWebView::onReturn()
{
    WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
        //
        tryResetTitle();
        //
    });
}


void WizDocumentWebView::doCopy()
{
    page()->triggerAction(QWebEnginePage::Copy, false);
}


void WizDocumentWebView::doPaste()
{
    WizExecuteOnThread(WIZ_THREAD_MAIN, [=]{
        //
        onPasteCommand();
        //
    });
}


void WizDocumentWebView::afterCopied()
{
    WizWebEnginePage::processCopiedData();
}

void WizDocumentWebView::onMarkerUndoStatusChanged(QString data)
{
    emit markerUndoStatusChanged(data);

}
void WizDocumentWebView::onMarkerInitiated(QString data)
{
    emit markerInitiated(data);
}

static std::map<QString, QWebEnginePage::WebAction> g_webActions;
void initWebActions(QWebEnginePage* page)
{
    if (!g_webActions.empty())
        return;
    //
    for (int action = QWebEnginePage::NoWebAction + 1;
         action < QWebEnginePage::WebActionCount;
         action++)
    {
        QWebEnginePage::WebAction a = (QWebEnginePage::WebAction)action;
        QAction* actionObj = page->action(a);
        //
        QString text = actionObj->text();
        g_webActions[text] = a;
        //
        text = text.replace("&", "");
        g_webActions[text] = a;
    }
}

QWebEnginePage::WebAction menuText2WebAction(QWebEnginePage* page, QString text)
{
    initWebActions(page);

    text = text.replace("&", "");
    auto it = g_webActions.find(text);
    if (it == g_webActions.end())
        return QWebEnginePage::NoWebAction;

    return it->second;
}

#define WIZEDITOR_ACTION_GOOGLE         QObject::tr("Use \"Google\" search")
#define WIZEDITOR_ACTION_BAIDU           QObject::tr("Use \"Baidu\" search")
#define WIZEDITOR_ACTION_CUT            QObject::tr("Cut")
#define WIZEDITOR_ACTION_COPY           QObject::tr("Copy")
#define WIZEDITOR_ACTION_PASTE          QObject::tr("Paste")
#define WIZEDITOR_ACTION_PASTE_PLAIN    QObject::tr("Paste plain text")
#define WIZEDITOR_ACTION_REMOVE_LINK    QObject::tr("Remove link")

void WizDocumentWebView::showContextMenu(const QPoint &pos)
{
    QWebEnginePage* page = this->page();
    if (!page)
        return;

    QMenu *menu = createStandardContextMenu();
    if (!menu)
        return;

    bool editing = isEditing();

    bool hasPasteMenu = false;
    bool hasLinkMenu = false;

    // get all actions of menu
    // FIXME: 不要使用这种遍历方法，速度有点慢，参考Qt浏览器实现示例
    QList<QAction*> actions = menu->actions();
    for (QAction* action : actions)
    {
        QWebEnginePage::WebAction a = menuText2WebAction(page, action->iconText());
        switch (a)
        {
        case QWebEnginePage::Copy:
            action->setText(QObject::tr("Copy"));
            break;
        case QWebEnginePage::Unselect:
            action->setText(QObject::tr("Unselect"));
            break;
        case QWebEnginePage::Back:
        case QWebEnginePage::Forward:
        case QWebEnginePage::Stop:
        case QWebEnginePage::Reload:
#if QT_VERSION >= 0x050600
        case QWebEnginePage::DownloadImageToDisk:
#endif
#if QT_VERSION >= 0x050800
        case QWebEnginePage::ViewSource:
#endif
            menu->removeAction(action);
            break;

        case QWebEnginePage::Paste:
            hasPasteMenu = true;
            break;
        case QWebEnginePage::OpenLinkInThisWindow:
        case QWebEnginePage::OpenLinkInNewWindow:
        case QWebEnginePage::OpenLinkInNewTab:
        case QWebEnginePage::DownloadLinkToDisk:
            menu->removeAction(action);
            hasLinkMenu = true;
            break;
        case QWebEnginePage::CopyLinkToClipboard:
            hasLinkMenu = true;
            break;
        default:
            break;
        }
    }

    if (!selectedText().isEmpty())
    {
        if (!menu->actions().isEmpty())
        {
            menu->addSeparator();
        }
        menu->addAction(WIZEDITOR_ACTION_GOOGLE, this, SLOT(on_editor_google_triggered()));
        menu->addAction(WIZEDITOR_ACTION_BAIDU, this, SLOT(on_editor_baidu_triggered()));
    }
    //
    if (editing)
    {
        if (!hasPasteMenu)
        {
            if (!menu->actions().isEmpty())
            {
                menu->addSeparator();
            }

            menu->addAction(WIZEDITOR_ACTION_PASTE, this, SLOT(on_editor_paste_triggered()));
            menu->addAction(WIZEDITOR_ACTION_PASTE_PLAIN, this, SLOT(on_editor_pastePlain_triggered()));
        }

        if (hasLinkMenu)
        {
            menu->addAction(WIZEDITOR_ACTION_REMOVE_LINK, this, SLOT(on_editor_removeLink_triggered()));
        }
    }

    if (menu->actions().isEmpty())
        return;

    menu->popup(pos);

    WizGetAnalyzer().logAction("editorContextMenu");
}
