#include "IWizExplorerApp.h"
#include "WizMainWindow.h"
#include "share/WizCommonUI.h"
#include "IWizExplorerWindow.h"
#include "IWizCategoryCtrl.h"
#include "IWizDocumentListCtrl.h"
#include "IWizDatabase.h"

IWizExplorerApp::IWizExplorerApp(WizMainWindow* mw, QObject* parent)
    : QObject(parent)
    , m_mainWindow(mw)
{
    m_window = new IWizExplorerWindow(mw, this);
    m_categoryCtrl = new IWizCategoryCtrl(m_mainWindow->CategoryView(), this);
    m_docListCtrl = new IWizDocumentListCtrl(m_mainWindow->documentList(),this);
    m_database = new IWizDatabase(m_mainWindow->DatabaseManagerEx(), this);
    m_commonUI = new WizCommonUI(this);
}

/** now just return WizExplorerApp, but should return REAL WizExplorerWindow */
QObject* IWizExplorerApp::Window()
{
    return m_window;
}

/** return WizCategoryView's interface */
QObject* IWizExplorerApp::CategoryCtrl()
{
    return m_categoryCtrl;
}

QObject* IWizExplorerApp::DocumentsCtrl()
{
    return m_docListCtrl;
}

QObject* IWizExplorerApp::CommonUI()
{
    return m_commonUI;
}

QObject* IWizExplorerApp::DatabaseManager()
{
    return m_database;
}

QObject* IWizExplorerApp::Database()
{
    return m_database->Database();
}

QObject* IWizExplorerApp::CreateWizObject(const QString& strObjectID)
{
    return m_mainWindow->CreateWizObject(strObjectID);
}

void IWizExplorerApp::SetSavingDocument(bool saving)
{
    m_mainWindow->SetSavingDocument(saving);
}

void IWizExplorerApp::ProcessClipboardBeforePaste(const QVariantMap& data)
{
    m_mainWindow->ProcessClipboardBeforePaste(data);
}

QString IWizExplorerApp::TranslateString(const QString& string)
{
    return m_mainWindow->TranslateString(string);
}

void IWizExplorerApp::OpenURLInDefaultBrowser(const QString& strUrl)
{
    m_mainWindow->OpenURLInDefaultBrowser(strUrl);
}

void IWizExplorerApp::GetToken(const QString& strFunctionName)
{
    m_mainWindow->GetToken(strFunctionName);
}

void IWizExplorerApp::SetDialogResult(int nResult)
{
    m_mainWindow->SetDialogResult(nResult);
}

void IWizExplorerApp::AppStoreIAP()
{
    m_mainWindow->AppStoreIAP();
}

void IWizExplorerApp::copyLink(const QString& link)
{
    m_mainWindow->copyLink(link);
}

void IWizExplorerApp::onClickedImage(const QString& src, const QString& list)
{
    m_mainWindow->onClickedImage(src, list);
}
