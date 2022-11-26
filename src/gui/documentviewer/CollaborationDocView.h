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
class QJsonObject;

class CollaborationDocView : public AbstractDocumentView
{
    Q_OBJECT

public:
    CollaborationDocView(const WIZDOCUMENTDATAEX &doc, WizExplorerApp &app, QWidget *parent = nullptr);
    CollaborationDocView(WizExplorerApp &app, QWidget *parent = nullptr)
        : CollaborationDocView({}, app, parent) { };
    ~CollaborationDocView();

    void RequestClose(bool force = false) override;
    QString Title() override;
    virtual void setEditorMode(WizEditorMode mode) override;
    WizEditorMode editorMode() const override { return m_mode; }
    const WIZDOCUMENTDATAEX& note() const override { return m_doc; }

    void createDocument(const WIZTAGDATA& tag);
    void loadDocument();
    void waitForSave();
    void trySaveDocument(std::function<void(const QVariant &)> callback);

public slots:
    void handleWindowCloseRequested();
    void handleEditButtonClicked();
    void handleNoteCreated(const QString &docGuid, const QString &title);
    void handleNoteDeleted(const WIZDOCUMENTDATA& data);
    void handleNoteTitleChanged(const QString &docGuid, const QString &title);
    void handleNoteAbstractChanged(const QString &docGuid, const QString &abstract);

private:
    WIZDOCUMENTDATAEX m_doc;
    WIZTAGDATA m_tag;
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
    void createDocument(const QString &kbGUID);

    Q_INVOKABLE void GetToken(const QString &func);
    Q_INVOKABLE void OnCommandStatusChanged(const QString &docGuid, const QJsonObject &commandStatus);
    Q_INVOKABLE void OnCreateNote(const QString &docGuid, const QString &title);
    Q_INVOKABLE void OnEditorLoad(const QString &docGuid);
    Q_INVOKABLE void OnModifiedStatusChanged(const QString &docGuid, const QJsonObject &modifiedStatus);
    Q_INVOKABLE void OnRemoteUserChanged(const QString &docGuid, const QJsonObject &users);
    Q_INVOKABLE void OnTitleChanged(const QString &docGuid, const QString &title);
    Q_INVOKABLE void OnAbstractChanged(const QString &docGuid, const QString &abstract);
    Q_INVOKABLE void OnFilePreview(const QString &docGuid, const QString &previewUrl, const QString &downloadUrl,
                                   const QString &fileName, const int fileSize, const QString &fileType);
    Q_INVOKABLE void Execute(const QString &method, const QVariant &arg1, const QVariant &arg2, const QVariant &arg3, const QVariant &arg4);
    Q_INVOKABLE void postMessage(const QString &msg);

    void setEditorMode(WizEditorMode mode) override;
    void isModified(std::function<void(bool modified)> callback) override;
    bool isEditorLoaded() { return m_editorLoaded; }

public Q_SLOTS:
    void onTitleEdited(QString strTitle) override;

Q_SIGNALS:
    void editorLoaded(const QString &docGuid);
    void noteCreated(const QString &docGuid, const QString &title);
    void titleChanged(const QString &docGuid, const QString &title);
    void abstractChanged(const QString &docGuid, const QString &abstract);
    void modifiedStatusChanged(bool isModified, bool isSaved);

private:
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;
    bool m_editorLoaded;
};

#endif // GUI_DOCUMENTVIEWER_COLLABORATIONDOCVIEW_H
