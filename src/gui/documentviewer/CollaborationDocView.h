#ifndef GUI_DOCUMENTVIEWER_COLLABORATIONDOCVIEW_H
#define GUI_DOCUMENTVIEWER_COLLABORATIONDOCVIEW_H

#include "share/WizObject.h"
#include "gui/tabbrowser/AbstractTabPage.h"

class WizWebEngineView;
class WizExplorerApp;
class WizDatabaseManager;
class WizUserSettings;

class CollaborationDocView : public AbstractTabPage
{
    Q_OBJECT

public:
    CollaborationDocView(const WIZDOCUMENTDATAEX &doc, WizExplorerApp &app, QWidget *parent = nullptr);
    ~CollaborationDocView();

    void RequestClose(bool force = false) override;
    QString Title() override;

    Q_INVOKABLE void GetToken(const QString &func);

public slots:
    void handleWindowCloseRequested();

private:
    WIZDOCUMENTDATAEX m_doc;
    WizWebEngineView *m_webView;
    WizExplorerApp &m_app;
    WizDatabaseManager &m_dbMgr;
    WizUserSettings &m_userSettings;
};

#endif // GUI_DOCUMENTVIEWER_COLLABORATIONDOCVIEW_H
