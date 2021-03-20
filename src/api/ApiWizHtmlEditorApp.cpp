#include "ApiWizHtmlEditorApp.h"
#include "gui/documentviewer/WizDocumentWebView.h"
#include "gui/documentviewer/WizDocumentView.h"

#include "database/WizDatabaseManager.h"
#include "database/WizDatabase.h"
#include "share/WizMisc.h"
#include "sync/WizAvatarHost.h"

ApiWizHtmlEditorApp::ApiWizHtmlEditorApp(WizDocumentWebView* webView, QObject *parent)
    : QObject(parent)
    , m_documentWebView(webView)
{

}

QString ApiWizHtmlEditorApp::getUserGuid()
{
    return m_documentWebView->getUserGuid();
}

QString ApiWizHtmlEditorApp::getUserAvatarFilePath()
{
    return m_documentWebView->getUserAvatarFilePath();
}

QString ApiWizHtmlEditorApp::getUserAlias()
{
    return m_documentWebView->getUserAlias();
}

bool ApiWizHtmlEditorApp::isPersonalDocument()
{
    return m_documentWebView->isPersonalDocument();
}

QString ApiWizHtmlEditorApp::getCurrentNoteHtml()
{
    return m_documentWebView->getCurrentNoteHtml();
}

bool ApiWizHtmlEditorApp::hasEditPermissionOnCurrentNote()
{
    return m_documentWebView->hasEditPermissionOnCurrentNote();
}

void ApiWizHtmlEditorApp::changeCurrentDocumentType(const QString &strType)
{
    m_documentWebView->changeCurrentDocumentType(strType);
}

bool ApiWizHtmlEditorApp::checkListClickable()
{
    return m_documentWebView->checkListClickable();
}

bool ApiWizHtmlEditorApp::shouldAddCustomCSS()
{
    return m_documentWebView->shouldAddCustomCSS();
}

bool ApiWizHtmlEditorApp::canRenderMarkdown()
{
    return m_documentWebView->canRenderMarkdown();
}

bool ApiWizHtmlEditorApp::canEditNote()
{
    return m_documentWebView->canEditNote();
}

QString ApiWizHtmlEditorApp::getLocalLanguage()
{
    return m_documentWebView->getLocalLanguage();
}

void ApiWizHtmlEditorApp::OnSelectionChange(const QString& currentStyle)
{
    m_documentWebView->OnSelectionChange(currentStyle);
}

void ApiWizHtmlEditorApp::saveCurrentNote()
{
    m_documentWebView->saveCurrentNote();
}

void ApiWizHtmlEditorApp::setModified(bool b)
{
    m_documentWebView->setModified(b);
}

void ApiWizHtmlEditorApp::onNoteLoadFinished()
{
    m_documentWebView->onNoteLoadFinished();
}

void ApiWizHtmlEditorApp::onReturn(){
    m_documentWebView->onReturn();
}

void ApiWizHtmlEditorApp::doPaste()
{
    m_documentWebView->doPaste();
}