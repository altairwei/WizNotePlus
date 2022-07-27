﻿#ifndef WIZACTIONS_H
#define WIZACTIONS_H

#include "WizDef.h"
#include "share/WizQtHelper.h"
#include <QAction>

class QAction;
class QMenuBar;
class QMenu;
class WizSettings;
class WizAnimateAction;
struct WIZACTION;
class QShortcut;

#define WIZACTION_GLOBAL_SYNC               "actionSync"
#define WIZACTION_GLOBAL_NEW_DOCUMENT       "actionNewNote"
#define WIZACTION_GLOBAL_GOFORWARD  "actionGoForward"
#define WIZACTION_GLOBAL_GOBACK  "actionGoBack"
#define WIZACTION_GLOBAL_SAVE_AS_PDF        "actionSaveAsPDF"
#define WIZACTION_GLOBAL_SAVE_AS_HTML        "actionSaveAsHtml"
#define WIZACTION_GLOBAL_SAVE_AS_MARKDOWN    "actionSaveAsMarkdown"
#define WIZACTION_GLOBAL_IMPORT_FILE            "actionImportFile"
#define WIZACTION_GLOBAL_EXPORT_FILE            "actionExportFile"
#define WIZACTION_GLOBAL_PRINT_MARGIN        "actionPrintMargin"
#define WIZACTION_GLOBAL_TOGGLE_CATEGORY    "actionViewToggleCategory"
#define WIZACTION_GLOBAL_SHOW_SUB_FOLDER_DOCUMENTS      "actionViewShowSubFolderDocuments"
#ifdef Q_OS_MAC
#   define WIZACTION_GLOBAL_CLIENT_FULLSCREEN  "actionViewToggleClientFullscreen"
#endif // Q_OS_MAC
#define WIZACTION_GLOBAL_TOGGLE_FULLSCREEN  "actionViewToggleFullscreen"

#define WIZACTION_EDITOR_UNDO               "actionEditingUndo"
#define WIZACTION_EDITOR_REDO               "actionEditingRedo"
#define WIZACTION_EDITOR_CUT                "actionEditingCut"
#define WIZACTION_EDITOR_COPY               "actionEditingCopy"
#define WIZACTION_EDITOR_PASTE              "actionEditingPaste"
#define WIZACTION_EDITOR_PASTE_PLAIN        "actionEditingPastePlain"
#define WIZACTION_EDITOR_DELETE             "actionEditingDelete"
#define WIZACTION_EDITOR_SELECT_ALL         "actionEditingSelectAll"
#define WIZACTION_EDITOR_FIND_REPLACE       "actionFindReplace"

#define WIZACTION_FORMAT_BOLD               "actionMenuFormatBold"
#define WIZACTION_FORMAT_ITALIC             "actionMenuFormatItalic"
#define WIZACTION_FORMAT_UNDERLINE          "actionMenuFormatUnderLine"
#define WIZACTION_FORMAT_STRIKETHROUGH      "actionMenuFormatStrikeThrough"
#define WIZACTION_FORMAT_SUBSCRIPT          "actionMenuFormatSubscript"
#define WIZACTION_FORMAT_SUPERSCRIPT        "actionMenuFormatSuperscript"
#define WIZACTION_FORMAT_UNORDEREDLIST      "actionMenuFormatInsertUnorderedList"
#define WIZACTION_FORMAT_ORDEREDLIST        "actionMenuFormatInsertOrderedList"
#define WIZACTION_FORMAT_JUSTIFYLEFT        "actionMenuFormatJustifyLeft"
#define WIZACTION_FORMAT_JUSTIFYRIGHT       "actionMenuFormatJustifyRight"
#define WIZACTION_FORMAT_JUSTIFYCENTER      "actionMenuFormatJustifyCenter"
#define WIZACTION_FORMAT_JUSTIFYJUSTIFY     "actionMenuFormatJustifyJustify"
#define WIZACTION_FORMAT_INDENT             "actionMenuFormatIndent"
#define WIZACTION_FORMAT_OUTDENT            "actionMenuFormatOutdent"
#define WIZACTION_FORMAT_INSERT_LINK        "actionMenuFormatInsertLink"
#define WIZACTION_FORMAT_INSERT_HORIZONTAL  "actionMenuFormatInsertHorizontal"
#define WIZACTION_FORMAT_INSERT_DATE        "actionMenuFormatInsertDate"
#define WIZACTION_FORMAT_INSERT_TIME        "actionMenuFormatInsertTime"
#define WIZACTION_FORMAT_REMOVE_FORMAT      "actionMenuFormatRemoveFormat"
#define WIZACTION_FORMAT_INSERT_CHECKLIST   "actionMenuFormatInsertCheckList"
#define WIZACTION_FORMAT_INSERT_CODE        "actionMenuFormatInsertCode"
#define WIZACTION_FORMAT_INSERT_IMAGE       "actionMenuFormatInsertImage"
#define WIZACTION_FORMAT_SCREEN_SHOT        "actionMenuFormatScreenShot"

#define WIZCATEGORY_OPTION_MESSAGECENTER    "actionCategoryMessageCenter"
#define WIZCATEGORY_OPTION_SHORTCUTS        "actionCategoryShortcuts"
#define WIZCATEGORY_OPTION_QUICKSEARCH      "actionCategoryQuickSearch"
#define WIZCATEGORY_OPTION_FOLDERS          "actionCategoryFolders"
#define WIZCATEGORY_OPTION_TAGS             "actionCategoryTags"
#define WIZCATEGORY_OPTION_BIZGROUPS        "actionCategoryBizGroups"
#define WIZCATEGORY_OPTION_PERSONALGROUPS   "actionCategoryPersonalGroups"
#define WIZCATEGORY_OPTION_THUMBNAILVIEW    "actionThumbnailView"
#define WIZCATEGORY_OPTION_SEARCHRESULTVIEW "actionSearchResultView"
#define WIZCATEGORY_OPTION_TWOLINEVIEW      "actionTwoLineView"
#define WIZCATEGORY_OPTION_ONELINEVIEW      "actionOneLineView"
#define WIZDOCUMENT_SORTBY_CREATEDTIME      "actionSortByCreatedTime"
#define WIZDOCUMENT_SORTBY_UPDATEDTIME      "actionSortByUpdatedTime"
#define WIZDOCUMENT_SORTBY_ACCESSTIME       "actionSortByAccessTime"
#define WIZDOCUMENT_SORTBY_TITLE            "actionSortByTitle"
#define WIZDOCUMENT_SORTBY_FOLDER           "actionSortByFolder"
#define WIZDOCUMENT_SORTBY_SIZE             "actionSortBySize"

#define WIZSKIN_RELOAD_STYLESHEET             "actionSkinReloadStyleSheet"

/**
//NOTE：因为Linux版本没有menubar，qaction在隐藏后快捷键无法触发，此处创建一个shortcut并与app的槽函数绑定
//作为程序的全局快捷键，禁用qaction时同时禁用shortcut
**/
class WizShortcutAction : public QAction
{
public:
    explicit WizShortcutAction(QObject* parent) : QAction(parent), m_shortcut(0){}
    WizShortcutAction(const QString &text, QObject* parent) : QAction(text, parent), m_shortcut(0){}
    WizShortcutAction(const QIcon &icon, const QString &text, QObject* parent) : QAction(icon, text, parent), m_shortcut(0){}

    void setShortcut(QShortcut* shortcut);
    void setShortcut(const QKeySequence &shortcut);
    void setEnabled(bool enable);
private:
    QShortcut* m_shortcut;
};


class WizActions : public QObject
{
    Q_OBJECT
public:
    WizActions(WizExplorerApp& app, QObject* parent);

private:
    QObject* m_parent;
    WizExplorerApp& m_app;

    std::map<QString, WizShortcutAction*> m_actions;
    WIZACTION* actionsData();
    WizShortcutAction* addAction(WIZACTION& action, bool bUseExtraShortcut);

public:
    void init(bool bUseExtraShortcut = false);
    WizShortcutAction* actionFromName(const QString& strActionName);
    void toggleActionText(const QString& strActionName);
    WizAnimateAction* animateActionFromName(const QString& strActionName);


    QMenu* toMenu(QWidget* parent, WizSettings& settings, const QString& strSection);
    void buildMenu(QMenu* pMenu, WizSettings& settings, const QString& strSection, bool bMenuBar);
    void buildMenuBar(QMenuBar* menuBar, const QString& strFileName, QMenu*& windowsMenu);
    void buildMenuList(QMenu* menu, const QString& strFileName, QMenu*& windowsMenu);
    //
Q_SIGNALS:
    void insertTableSelected(int row, int col);
};


#endif // WIZACTIONS_H
