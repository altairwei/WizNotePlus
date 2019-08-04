#ifndef IWIZEXPLORERAPP_H
#define IWIZEXPLORERAPP_H

#include <QObject>

class WizMainWindow;
class WizCommonUI;
class IWizExplorerWindow;
class IWizCategoryCtrl;
class IWizDocumentListCtrl;
class IWizDatabase;

class IWizExplorerApp : public QObject
{
    Q_OBJECT

private:
    WizMainWindow* m_mainWindow;
    WizCommonUI* m_commonUI;
    IWizExplorerWindow* m_window;
    IWizCategoryCtrl* m_categoryCtrl;
    IWizDocumentListCtrl* m_docListCtrl;
    IWizDatabase* m_database;

public:
    IWizExplorerApp(WizMainWindow* mw, QObject* parent);

    //WizExplorerApp API:
    QObject* Window();
    Q_PROPERTY(QObject* Window READ Window NOTIFY WindowChanged)

    QObject* CategoryCtrl();
    Q_PROPERTY(QObject* CategoryCtrl READ CategoryCtrl NOTIFY CategoryCtrlChanged)

    QObject* DocumentsCtrl();
    Q_PROPERTY(QObject* DocumentsCtrl READ DocumentsCtrl NOTIFY DocumentsCtrlChanged)

    QObject* CommonUI();
    Q_PROPERTY(QObject* CommonUI READ CommonUI NOTIFY CommonUIChanged)

    QObject* DatabaseManager();
    Q_PROPERTY(QObject* DatabaseManager READ DatabaseManager NOTIFY DatabaseManagerChanged)

    QObject* Database();
    Q_PROPERTY(QObject* Database READ Database NOTIFY DatabaseChanged)

    Q_INVOKABLE QObject* CreateWizObject(const QString& strObjectID);
    Q_INVOKABLE void SetSavingDocument(bool saving);
    Q_INVOKABLE void ProcessClipboardBeforePaste(const QVariantMap& data);
    Q_INVOKABLE QString Locale();

    //NOTE: these functions would called by web page, do not delete
    Q_INVOKABLE QString TranslateString(const QString& string);
    Q_INVOKABLE void OpenURLInDefaultBrowser(const QString& strUrl);
    Q_INVOKABLE void GetToken(const QString& strFunctionName);
    Q_INVOKABLE void SetDialogResult(int nResult);
    Q_INVOKABLE void AppStoreIAP();
    Q_INVOKABLE void copyLink(const QString& link);
    Q_INVOKABLE void onClickedImage(const QString& src, const QString& list);

signals:
    void WindowChanged();
    void CategoryCtrlChanged();
    void DocumentsCtrlChanged();
    void DatabaseManagerChanged();
    void DatabaseChanged();
    void CommonUIChanged();
};

#endif // WIZEXPLORERAPP_H
