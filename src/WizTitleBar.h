﻿#ifndef CORE_TITLEBAR_H
#define CORE_TITLEBAR_H

#include <QWidget>
#include <QIcon>
#include "share/WizObject.h"

class QString;
class QMenu;

struct WIZDOCUMENTDATA;
class WizDatabase;
class WizTagListWidget;
class WizNoteInfoForm;
class WizDocumentWebEngine;
class WizDocumentWebView;
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
    void setEditor(WizDocumentWebView* editor);

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
    void onTagButtonClicked();
    void onShareButtonClicked();
    void onAttachButtonClicked();
    void onHistoryButtonClicked();
    void onInfoButtonClicked();

    void onEmailActionClicked();
    void onShareActionClicked();

    void onCommentsButtonClicked();
    void onCommentPageLoaded(bool ok);
    void onViewNoteLoaded(WizDocumentView* view, const WIZDOCUMENTDATAEX& note, bool bOk);

    void on_commentTokenAcquired(QString token);
    void on_commentCountAcquired(QString GUID, int count);

    void onEditorChanged();
    void onEditorFocusIn();
    void onEditorFocusOut();

    //
    void updateTagButtonStatus();
    void updateAttachButtonStatus();
    void updateInfoButtonStatus();
    void updateCommentsButtonStatus();

    void showCoachingTips();
    //
    void onTitleEditFinished();

    void loadErrorPage();
signals:
    void notifyBar_link_clicked(const QString& link);
    void loadComment_request(const QString& url);
    void viewNoteInSeparateWindow_request();
    void viewNoteInExternalEditor_request(QString& Name, QString& ProgramFile,
                                QString& Arguments, int TextEditor, int UTF8Encoding);
private:
    void showInfoBar();
    void showEditorBar();
    void setTagBarVisible(bool visible);
    QMenu* createEditorMenu();
    //
    WizDocumentWebView* m_editor;
    WizExplorerApp& m_app;

    WizTitleEdit* m_editTitle;
    WizTagBar* m_tagBar;
    WizInfoBar* m_infoBar;
    WizNotifyBar* m_notifyBar;
    WizEditorToolBar* m_editorBar;

    WizRoundCellButton* m_editBtn;
    WizCellButton* m_separateBtn;
    WizCellButton* m_tagBtn;    
//    CellButton* m_emailBtn;
    WizCellButton* m_shareBtn;
    WizCellButton* m_attachBtn;
//    CellButton* m_historyBtn;
    WizCellButton* m_infoBtn;    

    QMenu* m_shareMenu;

    WizCellButton* m_commentsBtn;

    WizCommentManager* m_commentManager;

    WizTagListWidget* m_tags;
    WizAttachmentListWidget* m_attachments;
    WizNoteInfoForm* m_info;

    QString m_strWebchannelUrl;
    WizAnimateAction* m_editButtonAnimation;
};


#endif // CORE_TITLEBAR_H
