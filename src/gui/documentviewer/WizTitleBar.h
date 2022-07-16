#ifndef CORE_TITLEBAR_H
#define CORE_TITLEBAR_H

#include <QWidget>
#include <QIcon>
#include "share/WizObject.h"

class QString;
class QMenu;
class QToolBar;
class QPushButton;
struct WIZDOCUMENTDATA;
class WizDatabase;
class WizTagListWidget;
class WizNoteInfoForm;
class WizDocumentWebEngine;
class AbstractDocumentEditor;
class WizAttachmentListWidget;
class WizAnimateAction;
class WizExplorerApp;
class QNetworkReply;

class WizDocumentView;
class INoteView;
class WizCommentManager;

class WizTitleEdit;
class WizInfoBar;
class WizNotifyBar;
class WizEditorToolBar;
class WizCellButton;
class WizToolButton;
class WizEditButton;
class WizRoundCellButton;
class WizTagBar;

class WizTitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit WizTitleBar(WizExplorerApp& app, QWidget *parent);
    WizDocumentView* noteView();
    WizEditorToolBar* editorToolBar();
    void setLocked(bool bReadOnly, int nReason, bool bIsGroup);
    void showMessageTips(Qt::TextFormat format, const QString& strInfo);
    void hideMessageTips(bool useAnimation);
    void setEditor(AbstractDocumentEditor* editor);

    void setBackgroundColor(QColor color);

    void setNote(const WIZDOCUMENTDATA& data, WizEditorMode editorMode, bool locked);
    void updateInfo(const WIZDOCUMENTDATA& doc);
    void setEditorMode(WizEditorMode editorMode);
    void setEditButtonEnabled(bool enable);
    void updateEditButton(WizEditorMode editorMode);
    void resetTitle(const QString& strTitle);
    void clearAndSetPlaceHolderText(const QString& text);
    void clearPlaceHolderText();

    void startEditButtonAnimation();
    void stopEditButtonAnimation();

    void applyButtonStateForSeparateWindow(bool inSeparateWindow);

    WizTitleEdit* getTitleEdit();

public Q_SLOTS:
    void onEditButtonClicked();
    void onSeparateButtonClicked();
    void onExternalEditorMenuSelected();
    void onEditorOptionSelected();
    void handleDiscardChanges();
    void onTagButtonClicked();
    void onShareButtonClicked();
    void onAttachButtonClicked();
    void onHistoryButtonClicked();
    void onInfoButtonClicked();
    void onViewMindMapClicked();
    void onPageZoomButtonToggled(bool checked);
    void onPageZoomWidgetClosed();
    void handlePluginPopup(QAction*);

    void onEmailActionClicked();
    void onShareActionClicked();

    void onCommentsButtonClicked();
    void onCommentPageLoaded(bool ok);
    void onViewNoteLoaded(WizDocumentView* view, const WIZDOCUMENTDATAEX& note, bool bOk);

    void on_commentTokenAcquired(QString token);
    void on_commentCountAcquired(QString GUID, int count);

    void onEditorChanged();

    //
    void updateTagButtonStatus();
    void updateAttachButtonStatus();
    void updateInfoButtonStatus();
    void updateCommentsButtonStatus();

    void showCoachingTips();
    //
    void onTitleEditFinished();

    void loadErrorPage();

    void handlePluginEditorActionTriggered();

signals:
    void notifyBar_link_clicked(const QString& link);
    void loadComment_request(const QString& url);
    void viewNoteInSeparateWindow_request();
    void onViewMindMap(bool on);
    void viewNoteInExternalEditor_request(QString& Name, QString& ProgramFile,
                                QString& Arguments, int TextEditor, int UTF8Encoding);
    void discardChangesRequest();
    void launchPluginEditorRequest(const WIZDOCUMENTDATA &doc, const QString &guid);
    void pluginPopupRequest(QAction *ac, const QPoint &pos);
    void pluginSidebarRequest(QAction *ac, bool checked);
    void shareNoteByEmailRequest();
    void shareNoteByLinkRequest();
    void showPageZoomWidgetRequested(bool show, const QRect &btnLocation);

private:
    void setTagBarVisible(bool visible);
    void initPlugins();
    QMenu* createEditorMenu();

    AbstractDocumentEditor* m_editor;
    WizExplorerApp& m_app;

    QToolBar* m_documentToolBar;
    WizTitleEdit* m_editTitle;
    WizTagBar* m_tagBar;
    WizNotifyBar* m_notifyBar;

    WizEditButton* m_editBtn;
    WizToolButton* m_mindmapBtn;
    QAction* m_mindmapAction;
    WizToolButton* m_separateBtn;
    WizToolButton* m_tagBtn;
//    CellButton* m_emailBtn;
    WizToolButton* m_shareBtn;
    WizToolButton* m_attachBtn;
//    CellButton* m_historyBtn;
    WizToolButton* m_infoBtn;

    QMenu* m_shareMenu;

    WizToolButton* m_commentsBtn;
    WizToolButton* m_pageZoomBtn;

    WizCommentManager* m_commentManager;

    WizTagListWidget* m_tags;
    WizAttachmentListWidget* m_attachments;
    WizNoteInfoForm* m_info;

    QString m_strWebchannelUrl;
    WizAnimateAction* m_editButtonAnimation;
};

class CollaborationTitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit CollaborationTitleBar(WizExplorerApp& app, QWidget *parent = nullptr);
    WizEditButton* editButton() { return m_editBtn; }

    void startEditButtonAnimation();
    void stopEditButtonAnimation();

Q_SIGNALS:
    void editButtonClicked();

private:
    WizExplorerApp& m_app;
    QToolBar* m_documentToolBar;
    WizEditButton* m_editBtn;
    WizAnimateAction* m_editButtonAnimation;
};

#endif // CORE_TITLEBAR_H
