#ifndef GUI_DOCUMENTVIEWER_COLLABORATIONDOCVIEW_H
#define GUI_DOCUMENTVIEWER_COLLABORATIONDOCVIEW_H

#include "share/WizObject.h"
#include "gui/tabbrowser/AbstractTabPage.h"
#include "AbstractDocumentView.h"

class CollaborationEditor;
class WizExplorerApp;
class WizDatabaseManager;
class WizUserSettings;
class WizTitleBar;
class CollaborationTitleBar;

class CollaborationDocView : public AbstractDocumentView
{
    Q_OBJECT

public:
    CollaborationDocView(const WIZDOCUMENTDATAEX &doc, WizExplorerApp &app, QWidget *parent = nullptr);
    ~CollaborationDocView();

    void RequestClose(bool force = false) override;
    QString Title() override;
    virtual void setEditorMode(WizEditorMode mode) override;
    WizEditorMode editorMode() const override { return m_mode; }
    const WIZDOCUMENTDATAEX& note() const override { return m_doc; }

public slots:
    void handleWindowCloseRequested();
    void handleEditButtonClicked();

private:
    WIZDOCUMENTDATAEX m_doc;
    CollaborationTitleBar *m_title;
    CollaborationEditor *m_editor;
    WizExplorerApp &m_app;
    WizDatabaseManager &m_dbMgr;
    WizUserSettings &m_userSettings;
    WizEditorMode m_mode;
};

class CollaborationEditor : public AbstractDocumentEditor
{
    Q_OBJECT

public:
    CollaborationEditor(WizExplorerApp &app, QWidget *parent = nullptr);
    ~CollaborationEditor();
    void loadDocument(const WIZDOCUMENTDATAEX &doc);

    Q_INVOKABLE void GetToken(const QString &func);

    void setEditorMode(WizEditorMode mode) override;
    void isModified(std::function<void(bool modified)> callback) override;

public Q_SLOTS:
    void onTitleEdited(QString strTitle) override;

private:
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;
};

#endif // GUI_DOCUMENTVIEWER_COLLABORATIONDOCVIEW_H
