#include "ApiWizExplorerApp.h"

#include "WizMainWindow.h"
#include "share/WizCommonUI.h"

#include "ApiWizExplorerWindow.h"
#include "ApiWizCategoryCtrl.h"
#include "ApiWizDocumentListCtrl.h"
#include "ApiWizDatabase.h"

ApiWizExplorerApp::ApiWizExplorerApp(WizMainWindow* mw, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mw)
{
    m_window = new ApiWizExplorerWindow(mw, this);
    m_categoryCtrl = new ApiWizCategoryCtrl(m_mainWindow->CategoryView(), this);
    m_docListCtrl = new ApiWizDocumentListCtrl(m_mainWindow->documentList(),this);
    m_database = new ApiWizDatabase(m_mainWindow->DatabaseManagerEx(), this);
    m_commonUI = new WizCommonUI(this);
}

/** now just return WizExplorerApp, but should return REAL WizExplorerWindow */
QObject* ApiWizExplorerApp::Window()
{
    return m_window;
}

/** return WizCategoryView's interface */
QObject* ApiWizExplorerApp::CategoryCtrl()
{
    return m_categoryCtrl;
}

QObject* ApiWizExplorerApp::DocumentsCtrl()
{
    return m_docListCtrl;
}

QObject* ApiWizExplorerApp::CommonUI()
{
    return m_commonUI;
}

QObject* ApiWizExplorerApp::DatabaseManager()
{
    return m_database;
}

QObject* ApiWizExplorerApp::Database()
{
    return m_database->Database();
}

QObject* ApiWizExplorerApp::CreateWizObject(const QString& strObjectID)
{
    return m_mainWindow->CreateWizObject(strObjectID);
}

void ApiWizExplorerApp::SetSavingDocument(bool saving)
{
    m_mainWindow->SetSavingDocument(saving);
}

void ApiWizExplorerApp::ProcessClipboardBeforePaste(const QVariantMap& data)
{
    m_mainWindow->ProcessClipboardBeforePaste(data);
}

QObject* ApiWizExplorerApp::GetGroupDatabase(const QString &kbGUID)
{
    return m_database->GetGroupDatabase(kbGUID);
}

void ApiWizExplorerApp::ShowBubbleNotification(const QString &strTitle, const QString &strInfo)
{
    m_mainWindow->showBubbleNotification(strTitle, strInfo);
}

QString ApiWizExplorerApp::TranslateString(const QString& string)
{
    return m_mainWindow->TranslateString(string);
}

void ApiWizExplorerApp::OpenURLInDefaultBrowser(const QString& strUrl)
{
    m_mainWindow->OpenURLInDefaultBrowser(strUrl);
}

void ApiWizExplorerApp::GetToken(const QString& strFunctionName)
{
    m_mainWindow->GetToken(strFunctionName);
}

void ApiWizExplorerApp::SetDialogResult(int nResult)
{
    m_mainWindow->SetDialogResult(nResult);
}

void ApiWizExplorerApp::AppStoreIAP()
{
    m_mainWindow->AppStoreIAP();
}

void ApiWizExplorerApp::copyLink(const QString& link)
{
    m_mainWindow->copyLink(link);
}

void ApiWizExplorerApp::onClickedImage(const QString& src, const QString& list)
{
    m_mainWindow->onClickedImage(src, list);
}

QString ApiWizExplorerApp::Locale()
{
    return m_mainWindow->userSettings().locale();
}