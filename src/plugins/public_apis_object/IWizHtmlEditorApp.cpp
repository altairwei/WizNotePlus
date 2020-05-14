#include "IWizHtmlEditorApp.h"
#include "WizDocumentWebView.h"
#include "WizDocumentView.h"

#include "share/WizDatabaseManager.h"
#include "share/WizDatabase.h"
#include "share/WizMisc.h"
#include "sync/WizAvatarHost.h"

IWizHtmlEditorApp::IWizHtmlEditorApp(WizDocumentWebView* webView, QObject *parent)
    : QObject(parent)
    , m_documentWebView(webView)
{

}

QString IWizHtmlEditorApp::getUserGuid()
{
    return m_documentWebView->getUserGuid();
}

QString IWizHtmlEditorApp::getUserAvatarFilePath()
{
    return m_documentWebView->getUserAvatarFilePath();
}

QString IWizHtmlEditorApp::getUserAlias()
{
    return m_documentWebView->getUserAlias();
}

bool IWizHtmlEditorApp::isPersonalDocument()
{
    return m_documentWebView->isPersonalDocument();
}

QString IWizHtmlEditorApp::getCurrentNoteHtml()
{
    return m_documentWebView->getCurrentNoteHtml();
}

bool IWizHtmlEditorApp::hasEditPermissionOnCurrentNote()
{
    return m_documentWebView->hasEditPermissionOnCurrentNote();
}

void IWizHtmlEditorApp::changeCurrentDocumentType(const QString &strType)
{
    m_documentWebView->changeCurrentDocumentType(strType);
}

bool IWizHtmlEditorApp::checkListClickable()
{
    return m_documentWebView->checkListClickable();
}

bool IWizHtmlEditorApp::shouldAddCustomCSS()
{
    return m_documentWebView->shouldAddCustomCSS();
}

bool IWizHtmlEditorApp::canRenderMarkdown()
{
    return m_documentWebView->canRenderMarkdown();
}

bool IWizHtmlEditorApp::canEditNote()
{
    return m_documentWebView->canEditNote();
}

QString IWizHtmlEditorApp::getLocalLanguage()
{
    return m_documentWebView->getLocalLanguage();
}

void IWizHtmlEditorApp::OnSelectionChange(const QString& currentStyle)
{
    m_documentWebView->OnSelectionChange(currentStyle);
}

void IWizHtmlEditorApp::saveCurrentNote()
{
    m_documentWebView->saveCurrentNote();
}

void IWizHtmlEditorApp::setModified(bool b)
{
    m_documentWebView->setModified(b);
}

void IWizHtmlEditorApp::onNoteLoadFinished()
{
    m_documentWebView->onNoteLoadFinished();
}

void IWizHtmlEditorApp::onReturn(){
    m_documentWebView->onReturn();
}

void IWizHtmlEditorApp::doPaste()
{
    m_documentWebView->doPaste();
}