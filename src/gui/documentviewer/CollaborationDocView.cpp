#include "CollaborationDocView.h"

#include <QWebEnginePage>
#include <QVBoxLayout>
#include <QWebChannel>

#include "share/WizWebEngineView.h"
#include "share/WizThreads.h"
#include "share/WizMisc.h"
#include "sync/WizToken.h"
#include "sync/WizApiEntry.h"
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
    layout->setContentsMargins(0, 5, 0, 6);
    layout->setSpacing(0);
    this->setLayout(layout);
    layout->addWidget(m_title);
    layout->addStretch();
    layout->addWidget(m_editor);
    layout->addStretch();
    layout->setStretchFactor(m_title, 0);
    layout->setStretchFactor(m_editor, 1);

    m_editor->loadDocument(doc);
    m_editor->show();

    connect(m_title, &CollaborationTitleBar::editButtonClicked,
            this, &CollaborationDocView::handleEditButtonClicked);
    connect(m_editor->page(), &QWebEnginePage::windowCloseRequested,
            this, &CollaborationDocView::handleWindowCloseRequested);
    connect(m_editor->page(), &QWebEnginePage::titleChanged,
            this, &CollaborationDocView::titleChanged);
    connect(m_editor, &QWebEngineView::loadStarted,
            this, [this]() {
                m_title->editButton()->setEnabled(false);
                m_title->startEditButtonAnimation();
            });
    connect(m_editor, &QWebEngineView::loadFinished,
            this, [this](bool ok) {
                m_title->editButton()->setEnabled(ok);
                m_title->stopEditButtonAnimation();
            });

    if (doc.tCreated.secsTo(QDateTime::currentDateTime()) <= 1) {
        // 新建笔记
        //m_title->clearAndSetPlaceHolderText(doc.strTitle);
    } else {
        // 已有笔记
        //m_title->clearPlaceHolderText();
    }
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
        m_editor->triggerPageAction(QWebEnginePage::RequestClose);
    }
}

void CollaborationDocView::handleWindowCloseRequested()
{
    emit pageCloseRequested();
}

void CollaborationDocView::handleEditButtonClicked()
{
    auto oldMode = m_mode;
    m_mode = oldMode == modeReader ? modeEditor : modeReader;
    m_editor->setEditorMode(m_mode);

    if (oldMode == modeEditor) {
        m_editor->page()->runJavaScript("wizEditor.getTitle()",
            [&](const QVariant& vRet){
                if (vRet.type() == QVariant::String) {
                    WIZDOCUMENTDATA data;
                    WizDatabase& db = WizDatabaseManager::instance()->db(note().strKbGUID);
                    if (db.documentFromGuid(note().strGUID, data)) {
                        if (!db.canEditDocument(data))
                            return;
                        QString newTitle = vRet.toString();
                        if (newTitle != data.strTitle) {
                            data.strTitle = newTitle;
                            data.tDataModified = WizGetCurrentTime();
                            db.modifyDocumentInfo(data);
                        }
                    }
                }
            }
        );
    }
}

void CollaborationDocView::setEditorMode(WizEditorMode mode)
{
    if (m_mode == mode)
        return;
    m_editor->setEditorMode(mode);
}

//////////////////////////////////////////////////////////////////////////

CollaborationEditor::CollaborationEditor(WizExplorerApp &app, QWidget *parent)
    : AbstractDocumentEditor(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
{
    // create default web view
    WizWebEngineInjectObjectCollection objects = {{"wizQt", this}};
    auto profile = createWebEngineProfile(objects, this);
    auto webPage = new WizWebEnginePage(objects, profile, this);
    setPage(webPage);
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

void CollaborationEditor::setEditorMode(WizEditorMode mode)
{
    QString code = "wizEditor.setReadOnly(%1)";
    page()->runJavaScript(code.arg(
        mode == modeReader ? "true" : "false"));
}

void CollaborationEditor::isModified(std::function<void(bool modified)> callback)
{

}

void CollaborationEditor::onTitleEdited(QString strTitle)
{

}
