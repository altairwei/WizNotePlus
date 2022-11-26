#include "CollaborationDocView.h"

#include <QWebEnginePage>
#include <QVBoxLayout>
#include <QWebChannel>
#include <QJsonDocument>
#include <QEventLoop>
#include <QApplication>
#include <QMessageBox>

#include "share/WizWebEngineView.h"
#include "share/WizThreads.h"
#include "share/WizMisc.h"
#include "utils/WizLogger.h"
#include "sync/WizToken.h"
#include "sync/WizApiEntry.h"
#include "sync/WizKMSync.h"
#include "database/WizDatabaseManager.h"
#include "WizDef.h"
#include "WizMainWindow.h"
#include "WizTitleBar.h"
#include "WizCellButton.h"
#include "WizTitleEdit.h"

CollaborationDocView::CollaborationDocView(const WIZDOCUMENTDATAEX &doc, WizExplorerApp &app, QWidget *parent)
    : AbstractDocumentView(parent)
    , m_doc(doc)
    , m_title(new CollaborationTitleBar(app, this))
    , m_editor(new CollaborationEditor(app))
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_userSettings(app.userSettings())
    , m_mode(WizEditorMode::modeReader)
{
    // set layout
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 5, 0, 0);
    layout->setSpacing(5);
    layout->addWidget(m_title);
    layout->addWidget(m_editor);
    this->setLayout(layout);

    connect(m_title, &CollaborationTitleBar::editButtonClicked,
            this, &CollaborationDocView::handleEditButtonClicked);
    connect(m_editor->page(), &QWebEnginePage::windowCloseRequested,
            this, &CollaborationDocView::handleWindowCloseRequested);
    connect(m_editor->page(), &QWebEnginePage::titleChanged,
            this, &AbstractTabPage::titleChanged);
    connect(m_editor, &CollaborationEditor::titleChanged,
            this, &CollaborationDocView::handleNoteTitleChanged);
    connect(m_editor, &CollaborationEditor::abstractChanged,
            this, &CollaborationDocView::handleNoteAbstractChanged);

    connect(m_editor, &QWebEngineView::loadStarted,
            this, [this]() {
                if (!m_editor->isEditorLoaded()) {
                    m_title->editButton()->setEnabled(false);
                    m_title->startEditButtonAnimation();
                }
            });
    connect(m_editor, &CollaborationEditor::editorLoaded,
            this, [this](const QString &docGuid) {
                m_title->editButton()->setEnabled(true);
                if (m_mode == modeEditor && m_editor->isEditorLoaded())
                    m_title->editButton()->setState(WizToolButton::Checked);
                m_title->stopEditButtonAnimation();
            });
    connect(m_editor, &CollaborationEditor::noteCreated,
            this, &CollaborationDocView::handleNoteCreated, Qt::QueuedConnection);

    connect(&m_dbMgr, &WizDatabaseManager::documentDeleted,
            this, &CollaborationDocView::handleNoteDeleted);
}

CollaborationDocView::~CollaborationDocView()
{

}

QString CollaborationDocView::Title()
{
    return m_doc.strTitle;
}

void CollaborationDocView::RequestClose(bool force /*= false*/)
{
    if (force) {
        handleWindowCloseRequested();
    } else {
        waitForSave();
        m_editor->triggerPageAction(QWebEnginePage::RequestClose);
    }
}

void CollaborationDocView::handleWindowCloseRequested()
{
    emit pageCloseRequested();
}

void CollaborationDocView::trySaveDocument(std::function<void(const QVariant &)> callback)
{
    m_editor->page()->runJavaScript("wizEditor.getTitle()",
        [&, callback](const QVariant& vRet){
            if (vRet.type() == QVariant::String) {
                WIZDOCUMENTDATA data;
                WizDatabase& db = m_dbMgr.db(note().strKbGUID);
                if (db.documentFromGuid(note().strGUID, data)) {
                    if (!db.canEditDocument(data)) {
                        callback(QVariant(false));
                        return;
                    }

                    QString newTitle = vRet.toString();
                    if (newTitle != data.strTitle) {
                        data.strTitle = newTitle;
                        data.tDataModified = WizGetCurrentTime();
                        db.modifyDocumentInfo(data);
                    }

                    callback(QVariant(true));

                } else {
                    callback(QVariant(false));
                }
            }
        }
    );
}

void CollaborationDocView::handleEditButtonClicked()
{
    auto oldMode = m_mode;
    m_mode = oldMode == modeReader ? modeEditor : modeReader;
    m_editor->setEditorMode(m_mode);

    if (oldMode == modeEditor) {
        trySaveDocument([] (const QVariant &) {});
    }
}

void CollaborationDocView::setEditorMode(WizEditorMode mode)
{
    if (m_mode == mode)
        return;
    m_editor->setEditorMode(mode);
}

void CollaborationDocView::loadDocument()
{
    m_editor->loadDocument(m_doc);
}

void CollaborationDocView::createDocument(const WIZTAGDATA& tag)
{
    m_tag = tag;
    if (m_doc.strKbGUID.isEmpty())
        m_doc.strKbGUID = m_dbMgr.db().kbGUID();
    m_editor->createDocument(m_doc.strKbGUID);
    m_mode = modeEditor;
}

void CollaborationDocView::handleNoteCreated(const QString &docGuid, const QString &title)
{
    WizDatabase& pdb = WizDatabaseManager::instance()->db();
    WizKMSyncThread sync(pdb, false);
    sync.start(QThread::TimeCriticalPriority);

    bool done = false;
    connect(&sync, &WizKMSyncThread::syncFinished,
        this, [&] (int nErrorCode, bool isNetworkError,
                   const QString& strErrorMesssage, bool isBackground) {
            if (nErrorCode != 0)
                QMessageBox::critical(this,
                    tr("Sync failed"), strErrorMesssage);

            WIZDOCUMENTDATA data;
            WizDatabase& db = m_dbMgr.db(m_doc.strKbGUID);
            if (db.documentFromGuid(docGuid, data)) {
                data.strTitle = title.isEmpty() ? data.strTitle : title;
                data.tDataModified = WizGetCurrentTime();
                data.strLocation = m_doc.strLocation;
                db.modifyDocumentInfo(data);
                m_doc = data;

                if (!m_tag.strGUID.isEmpty()) {
                    WizDocument doc(db, m_doc);
                    doc.addTag(m_tag);
                }
            }

            done = true;
        }
    );

    sync.startSyncAll();

    while(!done)
        QApplication::processEvents();

    sync.waitForDone();
}

void CollaborationDocView::handleNoteDeleted(const WIZDOCUMENTDATA& data)
{
    if (note().strGUID != data.strGUID)
        return;

    RequestClose();
}

void CollaborationDocView::waitForSave()
{
    bool done = false;
    trySaveDocument([&done](const QVariant& ret){
        done = true;
    });

    while (!done)
    {
        QApplication::processEvents();
    }
}

void CollaborationDocView::handleNoteTitleChanged(const QString &docGuid, const QString &title)
{
    if (m_doc.strGUID != docGuid)
        return;

    trySaveDocument([] (const QVariant &) {});
}

void CollaborationDocView::handleNoteAbstractChanged(const QString &docGuid, const QString &abstract)
{
    WizDatabase& db = m_dbMgr.db(m_doc.strKbGUID);
    db.updateDocumentAbstract(docGuid, abstract);
}

//////////////////////////////////////////////////////////////////////////

CollaborationEditor::CollaborationEditor(WizExplorerApp &app, QWidget *parent)
    : AbstractDocumentEditor(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_editorLoaded(false)
{
    // create default web view
    WizWebEngineInjectObjectCollection objects = {
        {"wizQt", this},
        {"WizChromeBrowser", this},
        {"ReactNativeWebView", this}
    };
    auto profile = createWebEngineProfile(objects, this);
    auto webPage = new WizWebEnginePage(objects, profile, this);
    setPage(webPage);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

CollaborationEditor::~CollaborationEditor()
{
    // The class itself was registered to web channel, so we
    // need to deregister it before deletion avoiding to dispatch
    // pageCloseRequested() signal via QWebChannel.
    page()->webChannel()->deregisterObject(this);
}

void CollaborationEditor::loadDocument(const WIZDOCUMENTDATAEX &doc)
{
    auto &db = m_dbMgr.db(doc.strKbGUID);
    WIZUSERINFO info;
    if (!db.getUserInfo(info))
        return;

    QString liveEditorUrl = WizCommonApiEntry::noteplusUrl(
        doc.strKbGUID, doc.strGUID, info.strUserGUID, info.strDisplayName);
    load(liveEditorUrl);
}

void CollaborationEditor::createDocument(const QString &kbGUID)
{
    auto &db = m_dbMgr.db(kbGUID);
    WIZUSERINFO info;
    if (!db.getUserInfo(info))
        return;

    QString liveEditorUrl = WizCommonApiEntry::noteplusUrl(
        kbGUID, "create", info.strUserGUID, info.strDisplayName);
    load(liveEditorUrl);
}

void CollaborationEditor::setEditorMode(WizEditorMode mode)
{
    QString code = "wizEditor.setReadOnly(%1)";
    if (m_editorLoaded)
        page()->runJavaScript(code.arg(
            mode == modeReader ? "true" : "false"));
}

void CollaborationEditor::isModified(std::function<void(bool modified)> callback)
{

}

void CollaborationEditor::onTitleEdited(QString strTitle)
{

}

void CollaborationEditor::Execute(const QString &method,
    const QVariant &arg1, const QVariant &arg2,
    const QVariant &arg3, const QVariant &arg4)
{
    if (method == "GetToken")
        GetToken(arg1.toString());
    else if (method == "ChangeCommandStatus")
    {
        auto arg2json = QJsonDocument::fromJson(arg2.toByteArray());
        OnCommandStatusChanged(arg1.toString(), arg2json.object());
    }
    else if (method == "CreateNote")
        OnCreateNote(arg1.toString(), arg2.toString());
    else if (method == "OnEditorLoad")
        OnEditorLoad(arg1.toString());
    else if (method == "ChangeModifiedStatus")
    {
        auto arg2json = QJsonDocument::fromJson(arg2.toByteArray());
        OnModifiedStatusChanged(arg1.toString(), arg2json.object());
    }
    else if (method == "ChangeRemoteUser")
    {
        auto arg2json = QJsonDocument::fromJson(arg2.toByteArray());
        OnRemoteUserChanged(arg1.toString(), arg2json.object());
    }
    else if (method == "ChangeTitle")
        OnTitleChanged(arg1.toString(), arg2.toString());
    else if (method == "ChangeAbstract")
        OnAbstractChanged(arg1.toString(), arg2.toString());
    else
        TOLOG1("Unknown method invoked: %1", method);
}

void CollaborationEditor::postMessage(const QString &message)
{
    auto msg = QJsonDocument::fromJson(message.toUtf8()).object();
    auto type = msg["type"].toString();
    if (type == "getToken")
        GetToken(msg["callback"].toString());
    else if (type == "onCommandStatusChanged")
        OnCommandStatusChanged(
            msg["docGuid"].toString(), msg["commandStatus"].toObject());
    else if (type == "onCreateNote")
        OnCreateNote(msg["docGuid"].toString(), msg["title"].toString());
    else if (type == "onEditorLoad")
        OnEditorLoad(msg["docGuid"].toString());
    else if (type == "onModifiedStatusChanged")
        OnModifiedStatusChanged(
            msg["docGuid"].toString(), msg["modifiedStatus"].toObject());
    else if (type == "onRemoteUserChanged")
        OnRemoteUserChanged(
            msg["docGuid"].toString(), msg["users"].toObject());
    else if (type == "onTitleChanged")
        OnTitleChanged(msg["docGuid"].toString(), msg["title"].toString());
    else if (type == "onAbstractChanged")
        OnAbstractChanged(msg["docGuid"].toString(), msg["abstract"].toString());
    else if (type == "onFilePreview")
        OnFilePreview(msg["docGuid"].toString(), msg["previewUrl"].toString(),
                      msg["downloadUrl"].toString(), msg["fileName"].toString(),
                      msg["fileSize"].toInt(), msg["fileType"].toString());
    else
        TOLOG1("Unknown method invoked: %1", type);

}

void CollaborationEditor::GetToken(const QString &func)
{
    QString functionName(func);
    WizExecuteOnThread(WIZ_THREAD_NETWORK, [=] {
        QString strToken = WizToken::token();
        if (strToken.isEmpty()) return;
        WizExecuteOnThread(WIZ_THREAD_MAIN, [=] {
            QString strExec = functionName + QString("('%1')").arg(strToken);
            page()->runJavaScript(strExec);
        });
    });
}

void CollaborationEditor::OnCommandStatusChanged(const QString &docGuid, const QJsonObject &commandStatus)
{

}

void CollaborationEditor::OnCreateNote(const QString &docGuid, const QString &title)
{
    Q_EMIT noteCreated(docGuid, title);
}

void CollaborationEditor::OnEditorLoad(const QString &docGuid)
{
    m_editorLoaded = true;
    Q_EMIT editorLoaded(docGuid);
}

void CollaborationEditor::OnModifiedStatusChanged(const QString &docGuid, const QJsonObject &modifiedStatus)
{
    QJsonDocument doc(modifiedStatus);
    Q_EMIT modifiedStatusChanged(
                modifiedStatus["isModified"].toBool(),
                modifiedStatus["isSaved"].toBool());
}

void CollaborationEditor::OnRemoteUserChanged(const QString &docGuid, const QJsonObject &users)
{

}

void CollaborationEditor::OnTitleChanged(const QString &docGuid, const QString &title)
{
    Q_EMIT titleChanged(docGuid, title);
}

void CollaborationEditor::OnAbstractChanged(const QString &docGuid, const QString &abstract)
{
    Q_EMIT abstractChanged(docGuid, abstract);
}

void CollaborationEditor::OnFilePreview(const QString &docGuid, const QString &previewUrl, const QString &downloadUrl,
                                        const QString &fileName, const int fileSize, const QString &fileType)
{

}
