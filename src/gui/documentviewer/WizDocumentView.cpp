#include "WizDocumentView.h"

#include <QLineEdit>
#include <QLabel>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QApplication>
#include <QDesktopWidget>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QAction>

#include "share/WizGlobal.h"
#include "share/WizObjectDataDownloader.h"
#include "database/WizDatabaseManager.h"
#include "share/WizSettings.h"
#include "share/WizUIHelper.h"
#include "sync/WizToken.h"
#include "sync/WizApiEntry.h"
#include "sync/WizKMServer.h"
#include "widgets/WizScrollBar.h"
#include "widgets/WizLocalProgressWebView.h"
#include "widgets/WizSegmentedButton.h"
#include "widgets/WizTipsWidget.h"
#include "widgets/WizEmailShareDialog.h"
#include "WizNoteStyle.h"
#include "WizDocumentTransitionView.h"
#include "gui/documentviewer/WizDocumentWebView.h"
#include "WizButton.h"
#include "WizUserCipherForm.h"
#include "WizNotifyBar.h"
#include "WizMainWindow.h"
#include "gui/documentviewer/WizEditorToolBar.h"
#include "gui/documentviewer/WizDocumentWebView.h"
#include "gui/documentviewer/WizDocumentEditStatus.h"
#include "gui/documentviewer/WizTitleBar.h"
#include "share/WizThreads.h"
#include "share/WizWebEngineView.h"
#include "share/WizEnc.h"
#include "share/WizMisc.h"
#include "jsplugin/JSPluginWidgets.h"
#include "jsplugin/JSPluginManager.h"
#include "jsplugin/JSPlugin.h"
#include "utils/WizPathResolve.h"
#include "WizInfoBar.h"
#include "WizTitleEdit.h"


#define DOCUMENT_STATUS_NOSTATUS            0x0000
#define DOCUMENT_STATUS_GROUP                 0x0001
#define DOCUMENT_STATUS_OFFLINE               0x0002
#define DOCUMENT_STATUS_FIRSTTIMEVIEW     0x0004
#define DOCUMENT_STATUS_EDITBYOTHERS   0x0008
#define DOCUMENT_STATUS_NEWVERSIONFOUNDED      0x0010
#define DOCUMENT_STATUS_PERSONAL           0x0020
#define DOCUMENT_STATUS_ON_EDITREQUEST       0x0040
#define DOCUMENT_STATUS_ON_CHECKLIST       0x0080

WizDocumentView::WizDocumentView(WizExplorerApp& app, QWidget* parent)
    : AbstractDocumentView(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_userSettings(app.userSettings())
    , m_commentWidget(new WizLocalProgressWebView(app.mainWindow()))
    , m_title(new WizTitleBar(app, this))
    , m_passwordView(new WizUserCipherForm(app, this))
    , m_defaultViewMode(app.userSettings().noteViewMode())
    , m_transitionView(new WizDocumentTransitionView(this))
    , m_bLocked(false)
    , m_editorMode(modeReader)
    , m_noteLoaded(false)
    , m_editStatusSyncThread(new WizDocumentEditStatusSyncThread(this))
    , m_editStatus(0)
    , m_sizeHint(QSize(200, 1))
    , m_comments(nullptr)
    , m_pluginSidebar(nullptr)
    , m_editorBar(new WizEditorToolBar(app, this))
    , m_infoBar(new WizInfoBar(app, this))
{
    // docView is the widget to view/edit document actually.
    QVBoxLayout* layoutDoc = new QVBoxLayout();
    layoutDoc->setContentsMargins(0, 0, 0, 0);
    layoutDoc->setSpacing(0);
    m_docView = new QWidget(this);
    m_docView->setLayout(layoutDoc);

    // Control different page via stack widget
    m_stack = new QStackedWidget(this);

    // This page will ask password to view encrypted note
    connect(m_passwordView, SIGNAL(cipherCheckRequest()), SLOT(onCipherCheckRequest()));

    m_blankView = new QWidget(this);

    m_stack->addWidget(m_docView);
    m_stack->addWidget(m_passwordView);
    m_stack->addWidget(m_transitionView);
    m_stack->addWidget(m_blankView);
    m_stack->setCurrentWidget(m_blankView);
    m_stack->setBackgroundRole(QPalette::HighlightedText);

    // Setup comment widget
    m_comments = m_commentWidget->web();
    WizWebEngineInjectObjectCollection objects = {{"WizExplorerApp", WizGlobal::mainWindow()->publicAPIsObject()}};
    auto profile = createWebEngineProfile(objects, this);
    auto webPage = new WizWebEnginePage(objects, profile, m_comments);
    m_comments->setPage(webPage);
    m_comments->settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, true);
    m_comments->setAcceptDrops(false);
    connect(m_comments, SIGNAL(loadFinishedEx(bool)), m_title, SLOT(onCommentPageLoaded(bool)));
    connect(m_commentWidget, SIGNAL(widgetStatusChanged()), SLOT(on_commentWidget_statusChanged()));
    m_commentWidget->hide();

    // Editor Components
    QWidget* wgtEditor = new QWidget(m_docView);
    wgtEditor->setObjectName("editor-container");
    m_web = new WizDocumentWebView(app, wgtEditor);
    m_title->setEditor(m_web);
    m_editorBar->layout()->setAlignment(Qt::AlignVCenter);
    m_editorBar->setDelegate(m_web);
    connect(m_web, &WizDocumentWebView::focusIn, this, &WizDocumentView::onEditorFocusIn);
    connect(m_web, &WizDocumentWebView::focusOut, this, &WizDocumentView::onEditorFocusOut);
    connect(m_comments->getPage(), &WizWebEnginePage::linkClicked,
            m_web, &WizDocumentWebView::onEditorLinkClicked);

    QVBoxLayout* layoutEditor = new QVBoxLayout(wgtEditor);
    layoutEditor->setSpacing(0);
    layoutEditor->setContentsMargins(0, 5, 0, 0);

    QVBoxLayout* layoutTitle = new QVBoxLayout;
    layoutTitle->setContentsMargins(14, 0, 14, 6);
    layoutTitle->setSpacing(0);
    layoutTitle->addWidget(m_title);
    layoutTitle->addWidget(m_infoBar);
    layoutTitle->addWidget(m_editorBar);
    layoutTitle->addStretch();

    layoutEditor->addLayout(layoutTitle);
    layoutEditor->addWidget(m_web);
    layoutEditor->setStretchFactor(layoutTitle, 0);
    layoutEditor->setStretchFactor(m_web, 1);
    m_editorBar->hide();

    m_splitter = new WizSplitter(this);
    m_splitter->addWidget(wgtEditor);
    m_splitter->addWidget(m_commentWidget);
    m_splitter->setOrientation(Qt::Horizontal);

    layoutDoc->addWidget(m_splitter);

    QVBoxLayout* layoutMain = new QVBoxLayout(this);
    layoutMain->setContentsMargins(0, 0, 0, 0);
    setLayout(layoutMain);
    layoutMain->addWidget(m_stack);

    // 设置下载器
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    m_downloaderHost = mainWindow->downloaderHost();
    connect(m_downloaderHost, SIGNAL(downloadDone(const WIZOBJECTDATA&, bool)),
            SLOT(on_download_finished(const WIZOBJECTDATA&, bool)));

    connect(&m_dbMgr, SIGNAL(documentModified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)),
            SLOT(on_document_modified(const WIZDOCUMENTDATA&, const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(documentDataModified(const WIZDOCUMENTDATA&)),
            SLOT(on_document_data_modified(const WIZDOCUMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(attachmentCreated(const WIZDOCUMENTATTACHMENTDATA&)),
            SLOT(on_attachment_created(const WIZDOCUMENTATTACHMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(attachmentDeleted(const WIZDOCUMENTATTACHMENTDATA&)),
            SLOT(on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA&)));

    connect(&m_dbMgr, SIGNAL(documentUploaded(QString,QString)),
            m_editStatusSyncThread, SLOT(documentUploaded(QString,QString)));
    
    connect(&m_dbMgr, &WizDatabaseManager::documentDeleted, this, &WizDocumentView::on_document_deleted);

    // 连接浏览笔记请求信号
    connect(WizGlobal::instance(), SIGNAL(viewNoteRequested(WizDocumentView*,const WIZDOCUMENTDATAEX&,bool)),
            SLOT(onViewNoteRequested(WizDocumentView*,const WIZDOCUMENTDATAEX&,bool)));

    connect(WizGlobal::instance(), SIGNAL(viewNoteLoaded(WizDocumentView*,WIZDOCUMENTDATAEX,bool)),
            SLOT(onViewNoteLoaded(WizDocumentView*,const WIZDOCUMENTDATAEX&,bool)));

    connect(WizGlobal::instance(), SIGNAL(closeNoteRequested(WizDocumentView*)),
            SLOT(onCloseNoteRequested(WizDocumentView*)));

    connect(m_web, SIGNAL(focusIn()), SLOT(on_webView_focus_changed()));
    connect(m_web->page(), &QWebEnginePage::windowCloseRequested, this, &WizDocumentView::handleWindowCloseRequest);

    connect(m_title, SIGNAL(notifyBar_link_clicked(QString)), SLOT(on_notifyBar_link_clicked(QString)));
    connect(m_title, SIGNAL(loadComment_request(QString)), SLOT(on_loadComment_request(QString)), Qt::QueuedConnection);
    connect(m_title, SIGNAL(viewNoteInExternalEditor_request(QString&,QString&,QString&,int,int)), SLOT(on_viewNoteInExternalEditor_request(QString&,QString&,QString&,int,int)));
    connect(m_title, &WizTitleBar::discardChangesRequest,
            this, &WizDocumentView::handleDiscardChangesRequest);
    connect(m_title, &WizTitleBar::pluginSidebarRequest,
            this, &WizDocumentView::handlePluginSidebarRequest);
    connect(m_title, &WizTitleBar::shareNoteByEmailRequest,
            this, &WizDocumentView::shareNoteByEmail);
    connect(m_title, &WizTitleBar::shareNoteByLinkRequest,
            this, &WizDocumentView::shareNoteByLink);
    connect(m_title->getTitleEdit(), &WizTitleEdit::returnPressed,
            this, &WizDocumentView::onTitleReturnPressed);
    connect(m_title->getTitleEdit(), &WizTitleEdit::newTitleRequest,
            this, &WizDocumentView::onTitleEditingFinished);
    connect(m_title, &WizTitleBar::showPageZoomWidgetRequested,
            m_web, &WizWebEngineView::handleShowZoomWidgetRequest);
    connect(m_web, &WizDocumentWebView::zoomWidgetFinished,
            m_title, &WizTitleBar::onPageZoomWidgetClosed);

    // 编辑状态同步线程
    m_editStatusSyncThread->start(QThread::IdlePriority);

    m_editStatusChecker = new WizDocumentStatusChecker(this);
    connect(this, SIGNAL(checkDocumentEditStatusRequest(QString,QString)), m_editStatusChecker,
            SLOT(checkEditStatus(QString,QString)));
    connect(this, SIGNAL(stopCheckDocumentEditStatusRequest(QString,QString)),
            m_editStatusChecker, SLOT(stopCheckStatus(QString,QString)));
    connect(m_editStatusChecker, SIGNAL(checkEditStatusFinished(QString,bool)), \
            SLOT(on_checkEditStatus_finished(QString,bool)));
    connect(m_editStatusChecker, SIGNAL(checkTimeOut(QString)), \
            SLOT(on_checkEditStatus_timeout(QString)));
    connect(m_editStatusChecker, SIGNAL(documentEditingByOthers(QString,QStringList)), \
            SLOT(on_documentEditingByOthers(QString,QStringList)));
    connect(m_editStatusChecker, SIGNAL(checkDocumentChangedFinished(QString,bool)), \
            SLOT(on_checkDocumentChanged_finished(QString,bool)));

    QThread* checkThread = new QThread(this);
    connect(checkThread, SIGNAL(started()), m_editStatusChecker, SLOT(initialise()));
    connect(checkThread, SIGNAL(finished()), m_editStatusChecker, SLOT(clearTimers()));
    m_editStatusChecker->moveToThread(checkThread);
    checkThread->start();

}

WizDocumentView::~WizDocumentView()
{
    if (m_editStatusChecker)
       delete m_editStatusChecker;
}

QSize WizDocumentView::sizeHint() const
{
    return m_sizeHint;
}

void WizDocumentView::setSizeHint(QSize size)
{
    m_sizeHint = size;
}

void WizDocumentView::waitForDone()
{
    m_editStatusChecker->thread()->quit();
    m_editStatusSyncThread->waitForDone();

    bool done = false;
    m_web->trySaveDocument(m_note, false, [=, &done](const QVariant& ret){
        done = true;
    });

    while (!done)
    {
        QApplication::processEvents();
    }
}

void WizDocumentView::waitForSave()
{
    bool done = false;
    m_web->trySaveDocument(m_note, false, [=, &done](const QVariant& ret){
        done = true;
    });

    while (!done)
    {
        QApplication::processEvents();
    }
}

void WizDocumentView::waitForThread()
{
    m_editStatusChecker->thread()->quit();
    m_editStatusSyncThread->waitForDone();
}

void WizDocumentView::RequestClose(bool force /*= false*/)
{
    if (force) {
        // Force to close when Chromium process no response.
        handleWindowCloseRequest();
    } else {
        // We can not runJavaScript after RequestClose.
        waitForSave();
        m_web->triggerPageAction(QWebEnginePage::RequestClose);
    }
}

void WizDocumentView::handleWindowCloseRequest()
{
    // We Should exit all thread after user confirmed to close page.
    waitForThread();
    emit pageCloseRequested();
}

void WizDocumentView::handlePluginSidebarRequest(QAction *ac, bool checked)
{
    QString moduleGuid = ac->data().toString();
    if (moduleGuid.isEmpty())
        return;

    JSPluginManager &jsPluginMgr = JSPluginManager::instance();
    JSPluginModule *moduleData = jsPluginMgr.moduleByGUID(moduleGuid);
    if (!moduleData)
        return;

    static QPointer<QAction> lastAc = nullptr;
    if (m_pluginSidebar) {
        auto old = m_pluginSidebar->module()->spec()->guid();
        if (moduleData->spec()->guid() != old) {
            m_pluginSidebar->deleteLater();
            m_pluginSidebar = nullptr;
            if (lastAc) lastAc->setChecked(false);
        }
    }

    if (!m_pluginSidebar) {
        m_pluginSidebar = new JSPluginDocSidebar(m_app, moduleData, this);
        if (moduleData->spec()->sidebarLocation() == "Left")
            m_splitter->insertWidget(0, m_pluginSidebar);
        else
            m_splitter->addWidget(m_pluginSidebar);
        connect(m_web, &WizDocumentWebView::currentHtmlChanged,
                m_pluginSidebar, &JSPluginDocSidebar::documentHtmlChanged);
        connect(m_web, &WizDocumentWebView::loadFinishedEx,
                m_pluginSidebar, &JSPluginDocSidebar::editorLoadFinished);
    }

    if (checked) {
        moduleData->emitShowEvent();
        m_pluginSidebar->setVisible(true);
    } else {
        m_pluginSidebar->setVisible(false);
    }

    lastAc = ac;
}

WizWebEngineView*WizDocumentView::commentView() const
{
    return m_commentWidget->web();
}

void WizDocumentView::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
}

void WizDocumentView::resizeEvent(QResizeEvent* ev)
{
    QWidget::resizeEvent(ev);
    qDebug() << "oldSize: " << ev->oldSize() << ", newSize: " << ev->size();

    m_editorBar->adjustButtonPosition();
}

/**
 * @brief 处理“浏览笔记请求”信号，
 * @param view 文档视图
 * @param doc 文档数据
 * @param forceEditing 是否强制编辑
 */
void WizDocumentView::onViewNoteRequested(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool forceEditing)
{
    // 只处理指定为本视图的信号
    if (view != this)
        return;

    if (doc.tCreated.secsTo(QDateTime::currentDateTime()) <= 1) {
        // 新建笔记
        viewNote(doc, forceEditing);
        m_title->clearAndSetPlaceHolderText(doc.strTitle);
    } else {
        // 已有笔记
        m_title->clearPlaceHolderText();
        viewNote(doc, forceEditing);
    }
}

void WizDocumentView::onViewNoteLoaded(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool bOk)
{
}

bool WizDocumentView::reload()
{
    bool ret = m_dbMgr.db(m_note.strKbGUID).documentFromGuid(m_note.strGUID, m_note);
    m_title->updateInfo(note());
    m_infoBar->setDocument(note());
    emit titleChanged(m_note.strTitle);

    return ret;
}

void WizDocumentView::reloadNote()
{
    reload();
    m_web->reloadNoteData(note());
}


void WizDocumentView::initStat(const WIZDOCUMENTDATA& data, bool forceEdit)
{
    m_bLocked = false;
    int nLockReason = -1;

    if (m_dbMgr.db(data.strKbGUID).isGroup()) {
        if (!forceEdit) {
            nLockReason = WizNotifyBar::LockForGruop;
            m_bLocked = true;
        }
    } else if (!m_dbMgr.db(data.strKbGUID).canEditDocument(data)) {
        nLockReason = WizNotifyBar::PermissionLack;
        m_bLocked = true;
    } else if (WizDatabase::isInDeletedItems(data.strLocation)) {
        nLockReason = WizNotifyBar::Deleted;
        m_bLocked = true;
    }

    m_editorMode = modeReader;
    if (!m_bLocked) {
        if (forceEdit || m_defaultViewMode == viewmodeAlwaysEditing) {
            m_editorMode = modeEditor;
        }
    }

    emit m_web->canEditNoteChanged();

    bool bGroup = m_dbMgr.db(data.strKbGUID).isGroup();
    m_editStatus = m_editStatus & DOCUMENT_STATUS_PERSONAL;
    if (bGroup)
    {
        m_editStatus = m_editStatus | DOCUMENT_STATUS_GROUP | DOCUMENT_STATUS_FIRSTTIMEVIEW;
    }

    m_title->setLocked(m_bLocked, nLockReason, bGroup);
    //
    //   个人笔记检查笔记是否已经更新了
    //暂时禁止，等待服务器优化速度
    if (WizNotifyBar::LockForGruop == nLockReason)
    {
        startCheckDocumentEditStatus();
    }

    if (m_editorMode == modeEditor)
    {
        showCoachingTips();
    }
}

/**
 * @brief View note in current document view.
 * @param wizDoc Note meta data.
 * @param forceEdit Force to edit or not.
 */
void WizDocumentView::viewNote(const WIZDOCUMENTDATAEX& wizDoc, bool forceEdit)
{
    WIZDOCUMENTDATAEX dataTemp = wizDoc;

    m_web->trySaveDocument(m_note, false, [=](const QVariant& ret){

        WIZDOCUMENTDATAEX data = dataTemp;

        if (m_dbMgr.db(m_note.strKbGUID).isGroup())
        {
            stopDocumentEditingStatus();
        }

        m_noteLoaded = false;
        m_note = data;
        initStat(data, forceEdit);

        // download document if not exist
        WizDatabase& db = m_dbMgr.db(data.strKbGUID);
        QString strDocumentFileName = db.getDocumentFileName(data.strGUID);
        if (!db.isObjectDataDownloaded(data.strGUID, "document") || \
                !WizPathFileExists(strDocumentFileName))
        {
            m_web->clear();

            m_stack->setCurrentWidget(m_transitionView);
            downloadNoteFromServer(data);

            return;
        }

        // ask user cipher if needed
        data.nProtected = WizZiwReader::isEncryptedFile(strDocumentFileName) ? 1 : 0;
        if (data.nProtected || db.isEncryptAllData()) {
            //
            db.initCert(false);
            //
            if(!db.loadUserCert()) {
                return;
            }
            //
            if (db.getCertPassword().isEmpty()) {
                //
                if (db.isGroup()) {
                    m_passwordView->setPasswordText(tr("Encrypted group password"));
                } else {
                    m_passwordView->setPasswordText(tr("Personal notes certificate password"));
                }
                //

                m_passwordView->setHint(db.getCertPasswordHint());
                m_stack->setCurrentWidget(m_passwordView);
                m_passwordView->setCipherEditorFocus();

                return;
            }
        }
        //
        m_stack->setCurrentWidget(m_docView);

        // Emit notification of change regarding ownership of this note
        emit m_web->isPersonalDocumentChanged();
        emit m_web->hasEditPermissionOnCurrentNoteChanged();

        loadNote(data);
        WIZDOCUMENTDATA docData = data;
        docData.nReadCount ++;
        db.modifyDocumentReadCount(docData);
        docData.tAccessed = WizGetCurrentTime();
        db.modifyDocumentDateAccessed(docData);

        emit titleChanged(docData.strTitle);
    });
}

void WizDocumentView::reviewCurrentNote()
{
    Q_ASSERT(!m_note.strGUID.isEmpty());

    initStat(m_note, m_editorMode == modeEditor);

    // download document if not exist
    WizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
    QString strDocumentFileName = db.getDocumentFileName(m_note.strGUID);
    if (!db.isObjectDataDownloaded(m_note.strGUID, "document") || \
            !WizPathFileExists(strDocumentFileName))
    {
        downloadNoteFromServer(m_note);

        return;
    }

    // ask user cipher if needed
    if (m_note.nProtected) {
        if(!db.loadUserCert()) {
            return;
        }

        if (db.getCertPassword().isEmpty()) {
            m_passwordView->setHint(db.getCertPasswordHint());
            m_stack->setCurrentWidget(m_passwordView);
            m_passwordView->setCipherEditorFocus();

            return;
        }
    }

    if (m_stack->currentWidget() != m_docView) {
        m_stack->setCurrentWidget(m_docView);
    }
}

/*!
    Set the \a mode of editor and try to save document before switching mode.
    Editor will check and send group notes editing status.
*/
void WizDocumentView::setEditorMode(WizEditorMode mode)
{
    if (m_bLocked)
        return;

    if (m_editorMode == mode)
        return;

    bool edit = mode == modeEditor;
    bool read = !edit;

    // 检查文档的团队编辑状态
    bool isGroupNote = m_dbMgr.db(m_note.strKbGUID).isGroup();
    if (edit && isGroupNote)
    {
        m_title->startEditButtonAnimation();
        if (!checkDocumentEditable(false))
        {
            return;
        }
        // stop check document edit status while enter editing mode
        stopCheckDocumentEditStatus();
    }

    // 设置文档视图编辑器状态
    m_editorMode = mode;

    if (read)
    {
        m_editStatus = DOCUMENT_STATUS_NOSTATUS;

        // 保存标题，防止因多线程保存引起覆盖
        m_title->onTitleEditFinished();
        m_title->hideMessageTips(false);
    }

    if (mode == modeReader)
    {
        showInfoBar();
        m_editorBar->switchToNormalMode();
    }
    else
    {
        m_editorBar->switchToNormalMode();
        showEditorBar();
    }

    m_title->setEditorMode(mode);

    m_web->setEditorMode(mode);

    // 发送团队文档编辑状态
    if (isGroupNote)
    {
        if (m_editorMode == modeEditor)
        {
            sendDocumentEditingStatus();
        }
        else
        {
            stopDocumentEditingStatus();
            startCheckDocumentEditStatus();
        }
    }

    emit m_web->canEditNoteChanged();
}

void WizDocumentView::setDefaultViewMode(WizDocumentViewMode mode)
{
    m_defaultViewMode = mode;

    switch (m_defaultViewMode)
    {
    case viewmodeAlwaysEditing:
        setEditorMode(modeEditor);
        break;
    case viewmodeAlwaysReading:
        setEditorMode(modeReader);
        break;
    default:
        //Q_ASSERT(0);           //when set to the auto mode,do nonthing
        break;
    }
}

void WizDocumentView::settingsChanged()
{
    setDefaultViewMode(m_userSettings.noteViewMode());
}

void WizDocumentView::sendDocumentSavedSignal(const QString& strGUID, const QString& strKbGUID)
{
    WizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
    if (db.isGroup())
    {
        QString strUserAlias = db.getUserAlias();
        m_editStatusSyncThread->documentSaved(strUserAlias, strKbGUID, strGUID);
    }

    emit documentSaved(strGUID, this);
    //FIXME: Is it really necessary?
    emit titleChanged(m_note.strTitle);
}

void WizDocumentView::resetTitle(const QString& strTitle)
{
    m_title->resetTitle(strTitle);
}

bool WizDocumentView::checkListClickable()
{
    WizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
    if (db.canEditDocument(m_note))
    {
        //m_title->showMessageTips(Qt::PlainText, tr("Checking whether checklist is clickable..."));
        setCursor(Qt::WaitCursor);
        return checkDocumentEditable(true);
    }
    return false;
}

void WizDocumentView::setStatusToEditingByCheckList()
{
    stopCheckDocumentEditStatus();
    sendDocumentEditingStatus();
    m_title->showMessageTips(Qt::PlainText, tr("You have occupied this note by clicking checklist !  " \
                                               "Switch to other notes to free this note."));
}

void WizDocumentView::showCoachingTips()
{
    if (WizTipsWidget* tipWidget = m_editorBar->showCoachingTips())
    {
        connect(tipWidget, SIGNAL(finished()), m_title, SLOT(showCoachingTips()));
    }
    else
    {
        m_title->showCoachingTips();
    }
}

void WizDocumentView::wordCount(std::function<void(const QString&)> callback)
{
    m_web->page()->runJavaScript("WizReader.getWordCount()", [=](const QVariant& vRet){
        if (vRet.type() == QVariant::String) {
            QString json = vRet.toString();
            callback(json);
        }
    });
}

void WizDocumentView::setEditorFocus()
{
    m_web->setFocus(Qt::MouseFocusReason);
    m_web->editorFocus();
}


void WizDocumentView::on_attachment_created(const WIZDOCUMENTATTACHMENTDATA& attachment)
{
    if (attachment.strDocumentGUID != note().strGUID)
        return;

    reload();
}

void WizDocumentView::on_attachment_deleted(const WIZDOCUMENTATTACHMENTDATA& attachment)
{
    if (attachment.strDocumentGUID != note().strGUID)
        return;

    reload();
}

void WizDocumentView::on_checkEditStatus_finished(const QString& strGUID, bool editable)
{
//    qDebug() << "check eidt status finished , editable  : " << editable;
    if (strGUID == m_note.strGUID)
    {
        if (editable)
        {
            WizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
            WIZDOCUMENTDATA doc;
            db.documentFromGuid(strGUID, doc);
            if (db.canEditDocument(doc) && !WizDatabase::isInDeletedItems(doc.strLocation))
            {
                //            qDebug() << "document editable , hide message tips.";
                if (0 == (m_editStatus & DOCUMENT_STATUS_NEWVERSIONFOUNDED))
                {
                    m_title->hideMessageTips(true);
                }
                m_title->setEditButtonEnabled(true);
                m_bLocked = false;
            }
        }
        stopCheckDocumentAnimations();
    }
}

void WizDocumentView::loadNote(const WIZDOCUMENTDATAEX& doc)
{
    m_web->viewDocument(doc, m_editorMode);
    m_title->setNote(doc, m_editorMode, m_bLocked);
    m_infoBar->setDocument(doc);

    // save last
    m_note = doc;

    m_noteLoaded = true;

    if (m_editorMode == modeEditor && m_web->hasFocus())
    {
        sendDocumentEditingStatus();
    }
}

void WizDocumentView::downloadNoteFromServer(const WIZDOCUMENTDATA& note)
{
    connect(m_downloaderHost, SIGNAL(downloadProgress(QString,int,int)),
            m_transitionView, SLOT(onDownloadProgressChanged(QString,int,int)), Qt::UniqueConnection);
    m_downloaderHost->downloadDocument(note);
    m_transitionView->showAsMode(note.strGUID, WizDocumentTransitionView::Downloading);
}

void WizDocumentView::sendDocumentEditingStatus()
{
    WizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
    if (db.isGroup())
    {
        QString strUserAlias = db.getUserAlias();
        m_editStatusSyncThread->startEditingDocument(strUserAlias, m_note.strKbGUID, m_note.strGUID);
    }
}

void WizDocumentView::stopDocumentEditingStatus()
{
    WIZDOCUMENTDATA doc = m_note;
    if (m_dbMgr.db(doc.strKbGUID).documentFromGuid(doc.strGUID, doc))
    {
        bool bModified = doc.nVersion == -1;
        m_editStatusSyncThread->stopEditingDocument(doc.strKbGUID, doc.strGUID, bModified);
    }
}

void WizDocumentView::startCheckDocumentEditStatus()
{
    //首先检查笔记是否是待下载状态，如果一篇笔记一直打开，自动同步会把笔记状态设置为待下载，
    //但是其版本号是服务器的版本号，检查新版本无效果

    WizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
    if (!db.isDocumentDownloaded(m_note.strGUID))
    {
        on_checkDocumentChanged_finished(m_note.strGUID, true);
        m_title->stopEditButtonAnimation();
        return;
    }

    m_editStatus = DOCUMENT_STATUS_NOSTATUS;
    emit checkDocumentEditStatusRequest(m_note.strKbGUID, m_note.strGUID);
}

void WizDocumentView::stopCheckDocumentEditStatus()
{
    emit stopCheckDocumentEditStatusRequest(m_note.strKbGUID, m_note.strGUID);
}

bool WizDocumentView::checkDocumentEditable(bool checklist)
{
    QEventLoop loop;
    connect(m_editStatusChecker, SIGNAL(checkEditStatusFinished(QString,bool)), &loop, SLOT(quit()));
    connect(m_editStatusChecker, SIGNAL(checkTimeOut(QString)), &loop, SLOT(quit()));
    startCheckDocumentEditStatus();
    m_editStatus = m_editStatus | DOCUMENT_STATUS_ON_EDITREQUEST;
    if (checklist)
    {
        m_editStatus |= DOCUMENT_STATUS_ON_CHECKLIST;
    }
    loop.exec();
    //

    bool editByOther = m_editStatus & DOCUMENT_STATUS_EDITBYOTHERS;
    bool newVersion = m_editStatus & DOCUMENT_STATUS_NEWVERSIONFOUNDED;
    bool isOffLine = m_editStatus & DOCUMENT_STATUS_OFFLINE;
    return !(editByOther || newVersion) || isOffLine;
}

void WizDocumentView::stopCheckDocumentAnimations()
{
    if (cursor().shape() != Qt::ArrowCursor)
    {
        setCursor(Qt::ArrowCursor);
    }
    m_title->stopEditButtonAnimation();
}

void WizDocumentView::on_document_modified(const WIZDOCUMENTDATA& documentOld, const WIZDOCUMENTDATA& documentNew)
{
    Q_UNUSED(documentOld);

    if (note().strGUID != documentNew.strGUID)
        return;

    reload();
}

void WizDocumentView::onCloseNoteRequested(WizDocumentView *view)
{
    Q_UNUSED(view)

    m_stack->setCurrentWidget(m_blankView);
}

void WizDocumentView::onCipherCheckRequest()
{
    const WIZDOCUMENTDATA& noteData = note();
    WizDatabase& db = m_dbMgr.db(noteData.strKbGUID);
    //
    QString password = m_passwordView->userCipher();
    if (!db.verifyCertPassword(password))
    {
        m_passwordView->cipherError();
        WizMessageBox::information(qApp->activeWindow(), tr("WizNote"), tr("Invalid password."));

        return;
    }
    //
    m_passwordView->cipherCorrect();

    m_stack->setCurrentWidget(m_docView);
    loadNote(noteData);
}

void WizDocumentView::on_download_finished(const WIZOBJECTDATA &data, bool bSucceed)
{
    if (m_note.strKbGUID != data.strKbGUID
            || m_note.strGUID != data.strObjectGUID)
        return;

    if (!bSucceed)
    {
        m_transitionView->showAsMode(data.strObjectGUID, WizDocumentTransitionView::ErrorOccured);
        return;
    }

    if (m_editorMode == modeEditor)
        return;


    bool onEditRequest = m_editStatus & DOCUMENT_STATUS_ON_EDITREQUEST;
    if (onEditRequest)
    {
        if (m_editStatus & DOCUMENT_STATUS_ON_CHECKLIST)
        {
            onEditRequest = false;
        }
    }
    //
    bool forceEdit = onEditRequest ? true : false;

    viewNote(m_note, forceEdit);
}

void WizDocumentView::on_document_data_modified(const WIZDOCUMENTDATA& data)
{
    //verify m_noteLoaded before reload
    if (note().strGUID != data.strGUID || !m_noteLoaded)
        return;

    reloadNote();

    WizMainWindow::instance()->quickSyncKb(data.strKbGUID);
}



void WizDocumentView::on_document_data_changed(const QString& strGUID,
                                              WizDocumentView* viewer)
{
    if (viewer != this && strGUID == note().strGUID && m_editorMode == modeReader)
    {
        reloadNote();
    }
}

void WizDocumentView::on_document_deleted(const WIZDOCUMENTDATA& data)
{
    if (note().strGUID != data.strGUID || !m_noteLoaded)
        return;

    RequestClose();
}


void WizDocumentView::on_documentEditingByOthers(QString strGUID, QStringList editors)
{
    //
    //QString strCurrentUser = m_dbMgr.db(m_note.strKbGUID).GetUserAlias();
    //editors.removeAll(strCurrentUser);

    if (strGUID == m_note.strGUID)
    {
        if (!editors.isEmpty())
        {
            QString strEditor = editors.join(" , ");
            if (!strEditor.isEmpty())
            {
                WizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
                QString strUserAlias = db.getUserAlias();
                if (editors.count() == 1 && editors.first() == strUserAlias)
                {
                    qDebug() << "[EditStatus]:editing by myself.";
                    m_title->hideMessageTips(true);
                    m_title->setEditButtonEnabled(true);
                    m_editStatus = m_editStatus & ~DOCUMENT_STATUS_EDITBYOTHERS;
                }
                else
                {
                    m_title->showMessageTips(Qt::PlainText, QString(tr("%1 is currently editing this note. Note has been locked.")).arg(strEditor));
                    m_editStatus = m_editStatus | DOCUMENT_STATUS_EDITBYOTHERS;
                    if (m_note.nVersion == -1)
                    {
                        m_title->showMessageTips(Qt::PlainText, QString(tr("%1 is currently editing this note.")).arg(strEditor));
                        m_title->setEditButtonEnabled(true);
                    }
                    else
                    {
                        m_title->setEditButtonEnabled(false);
                    }
                }
            }
        }
        else
        {
            m_title->setEditButtonEnabled(!m_bLocked);
            m_editStatus = m_editStatus & ~DOCUMENT_STATUS_EDITBYOTHERS;
        }
    }
}

void WizDocumentView::on_checkEditStatus_timeout(const QString& strGUID)
{
    if (strGUID == m_note.strGUID)
    {
        if (m_editStatus & DOCUMENT_STATUS_ON_EDITREQUEST)
        {
            m_title->showMessageTips(Qt::RichText, tr("The current network in poor condition, you are <b> offline editing mode </b>."));
        }
        m_title->setEditButtonEnabled(true);
        m_editStatus = m_editStatus | DOCUMENT_STATUS_OFFLINE;
        m_bLocked = false;
        stopCheckDocumentAnimations();
    }
}

void WizDocumentView::on_checkDocumentChanged_finished(const QString& strGUID, bool changed)
{
    if (strGUID == m_note.strGUID)
    {       
        if (changed)
        {
            m_title->showMessageTips(Qt::RichText, QString(tr("New version on server avalible. <a href='%1'>Click to download new version.<a>")).arg(NOTIFYBAR_LABELLINK_DOWNLOAD));
            m_editStatus |= DOCUMENT_STATUS_NEWVERSIONFOUNDED;
        }
        else
        {
            m_bLocked = false;
            int nLockReason = -1;

            const WIZDOCUMENTDATA& doc = note();
            if (!m_dbMgr.db(doc.strKbGUID).canEditDocument(doc)) {
                nLockReason = WizNotifyBar::PermissionLack;
                m_bLocked = true;
            } else if (WizDatabase::isInDeletedItems(doc.strLocation)) {
                nLockReason = WizNotifyBar::Deleted;
                m_bLocked = true;
            }

            if (m_bLocked)
            {
                bool bGroup = m_dbMgr.db(doc.strKbGUID).isGroup();
                m_title->setLocked(m_bLocked, nLockReason, bGroup);
            }
            //
            setEditorMode(modeReader);

            m_editStatus &= ~DOCUMENT_STATUS_NEWVERSIONFOUNDED;
        }
        m_editStatus = m_editStatus & ~DOCUMENT_STATUS_FIRSTTIMEVIEW;
    }
}

void WizDocumentView::on_syncDatabase_request(const QString& strKbGUID)
{
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow *>(m_app.mainWindow());
    mainWindow->quickSyncKb(strKbGUID);
}

void WizDocumentView::on_webView_focus_changed()
{
    if (m_web->hasFocus())
    {
        sendDocumentEditingStatus();
    }
}

void WizDocumentView::on_notifyBar_link_clicked(const QString& link)
{
    if (link == NOTIFYBAR_LABELLINK_DOWNLOAD)
    {
        WizDatabase& db = m_dbMgr.db(m_note.strKbGUID);
        QString strDocumentFileName = db.getDocumentFileName(m_note.strGUID);
        WizDeleteFile(strDocumentFileName);

        downloadNoteFromServer(m_note);
    }
}

void WizDocumentView::on_viewNoteInExternalEditor_request(QString& Name, QString& ProgramFile,
                                                          QString& Arguments, int TextEditor, int UTF8Encoding)
{
    WizExternalEditorData editorData = {
        Name, ProgramFile, Arguments, TextEditor, UTF8Encoding
    };

    emit viewNoteInExternalEditorRequest(editorData, m_note);
}

void WizDocumentView::handleDiscardChangesRequest()
{
    // Change editor state. WizDocumentView::setEditorMode() involves 
    // the process of saving, so we change m_editorMode separately.
    m_editorMode = modeReader;

    m_editStatus = DOCUMENT_STATUS_NOSTATUS;
    m_title->setEditorMode(modeReader);

    if (m_editorMode == modeReader)
    {
        showInfoBar();
        m_editorBar->switchToNormalMode();
    }
    else
    {
        m_editorBar->switchToNormalMode();
        showEditorBar();
    }

    // Discard changes
    m_web->discardChanges();

    // Notify group status
    bool isGroupNote = m_dbMgr.db(m_note.strKbGUID).isGroup();
    if (isGroupNote) {
        stopDocumentEditingStatus();
        startCheckDocumentEditStatus();
    }

    emit m_web->canEditNoteChanged();
}

void WizDocumentView::on_loadComment_request(const QString& url)
{
    m_comments->load(url);
}

void WizDocumentView::on_commentWidget_statusChanged()
{    
    if (m_web->isInSeperateWindow())
    {
        int commentWidth = 271;
        int maxWidth = maximumWidth();
        if (!WizIsHighPixel())
        {
            if (qApp->desktop()->availableGeometry().width() < 1440)
            {
                maxWidth = 916;
            }
            maxWidth = m_commentWidget->isVisible() ? (maxWidth + commentWidth) : (maxWidth - commentWidth);
        }
        if (width() > 1000)
        {
            m_commentWidget->setFixedWidth(271);
        }
        else
        {
            m_commentWidget->setMinimumWidth(0);
            m_commentWidget->setMaximumWidth(500);
        }
        setMaximumWidth(maxWidth);
        setSizeHint(QSize(maxWidth, 1));

    }

    m_editorBar->adjustButtonPosition();
}

void WizDocumentView::shareNoteByEmail()
{
    getMailSender([=](QString sendTo){
        WizEmailShareDialog dlg(m_app);
        dlg.setNote(note(), sendTo);
        connect(&dlg, &WizEmailShareDialog::insertCommentToNoteRequest,
                m_web, &WizDocumentWebView::on_insertCommentToNote_request);
        dlg.exec();
    });
}

void WizDocumentView::getMailSender(std::function<void(QString)> callback)
{
    QString scriptFileName = Utils::WizPathResolve::resourcesPath() + "files/scripts/GetMailSender.js";
    QString code;
    ::WizLoadUnicodeTextFromFile(scriptFileName, code);

    m_web->page()->runJavaScript(code, [=](const QVariant& vRet){
        QString mailSender = vRet.toString();

        if (mailSender.isEmpty())
        {
            QRegExp rxlen("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b");
            rxlen.setCaseSensitivity(Qt::CaseInsensitive);
            rxlen.setPatternSyntax(QRegExp::RegExp);
            QString strTitle = note().strTitle;
            int pos = rxlen.indexIn(strTitle);
            if (pos > -1) {
                mailSender = rxlen.cap(0);
            }
        }

        callback(mailSender);
    });
}

void WizDocumentView::shareNoteByLink()
{
    auto &doc = note();
    emit shareDocumentByLinkRequest(doc.strKbGUID, doc.strGUID);
}


void WizDocumentView::onEditorFocusIn()
{
    showEditorBar();
}

void WizDocumentView::onEditorFocusOut()
{
    showEditorBar();
    if (!m_editorBar->hasFocus())
        showInfoBar();
}

void WizDocumentView::onTitleReturnPressed()
{
    setEditorFocus();
    web()->setFocus(Qt::MouseFocusReason);
    web()->editorFocus();
}

void WizDocumentView::onTitleEditingFinished(const QString &newTitle)
{
    WIZDOCUMENTDATA data;
    WizDatabase& db = WizDatabaseManager::instance()->db(note().strKbGUID);
    if (db.documentFromGuid(note().strGUID, data)) {
        if (!db.canEditDocument(data))
            return;

        if (newTitle != data.strTitle) {
            data.strTitle = newTitle;
            data.tDataModified = WizGetCurrentTime();
            db.modifyDocumentInfo(data);

            m_web->onTitleEdited(newTitle);
        }
    }
}

void WizDocumentView::showInfoBar()
{
    m_editorBar->hide();
    m_infoBar->show();
}

void WizDocumentView::showEditorBar()
{
    m_infoBar->hide();
    m_editorBar->show();
    m_editorBar->adjustButtonPosition();
}
