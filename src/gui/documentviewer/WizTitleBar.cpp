#include "WizTitleBar.h"

#include <QVBoxLayout>
#include <QUrl>
#include <QMenu>
#include <QNetworkConfigurationManager>
#include <QMessageBox>
#include <QSplitter>
#include <QList>
#include <QLabel>
#include <QToolBar>
#include <QAction>
#include <QVariant>
#include <QMap>
#include <QShortcut>
#include <QPushButton>

#include "share/jsoncpp/json/json.h"

#include "widgets/WizTagBar.h"

#include "WizCellButton.h"
#include "WizInfoBar.h"
#include "WizNotifyBar.h"
#include "gui/documentviewer/WizTitleEdit.h"
#include "gui/documentviewer/WizEditorToolBar.h"
#include "gui/documentviewer/WizDocumentView.h"
#include "gui/documentviewer/WizDocumentWebView.h"
#include "gui/documentviewer/WizNoteInfoForm.h"
#include "gui/documentviewer/AbstractDocumentView.h"
#include "WizTagListWidget.h"
#include "WizAttachmentListWidget.h"
#include "WizNoteStyle.h"
#include "share/WizMisc.h"
#include "database/WizDatabase.h"
#include "share/WizSettings.h"
#include "share/WizAnimateAction.h"
#include "share/WizAnalyzer.h"
#include "share/WizGlobal.h"
#include "share/WizThreads.h"
#include "sync/WizApiEntry.h"
#include "utils/WizStyleHelper.h"
#include "utils/WizPathResolve.h"
#include "widgets/WizLocalProgressWebView.h"
#include "widgets/WizTipsWidget.h"
#include "widgets/WizExternalEditorSettingDialog.h"

#include "WizMessageCompleter.h"
#include "WizOEMSettings.h"
#include "WizMainWindow.h"
#include "share/WizSettings.h"
#include "share/WizWebEngineView.h"

#include "core/WizCommentManager.h"

#include "jsplugin/JSPluginManager.h"
#include "jsplugin/JSPluginSpec.h"
#include "jsplugin/JSPlugin.h"

#define WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_LINK QObject::tr("Share by Link")
#define WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_EMAIL QObject::tr("Share by Email")

QString getOptionKey()
{
#ifdef Q_OS_MAC
    return "⌥";
#else
    return "Alt+";
#endif
}

WizTitleBar::WizTitleBar(WizExplorerApp& app, QWidget *parent)
    : QWidget(parent)
    , m_app(app)
    , m_documentToolBar(new QToolBar(this))
    , m_editTitle(new WizTitleEdit(this))
    , m_tagBar(new WizTagBar(app, this))
    , m_notifyBar(new WizNotifyBar(this))
    , m_editor(nullptr)
    , m_tags(nullptr)
    , m_info(nullptr)
    , m_attachments(nullptr)
    , m_editButtonAnimation(nullptr)
    , m_commentManager(new WizCommentManager(this))
{
    setObjectName("document-title-bar");

    // 标题栏输入框
    m_editTitle->setCompleter(new WizMessageCompleter(m_editTitle));
    int nTitleHeight = Utils::WizStyleHelper::titleEditorHeight();
    m_editTitle->setFixedHeight(nTitleHeight);
    m_editTitle->setAlignment(Qt::AlignVCenter);
    m_editTitle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QString strTheme = Utils::WizStyleHelper::themeName();

    // 添加垂直布局<工具栏+状态栏？>
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    // 编辑按钮
    QSize iconSize = QSize(Utils::WizStyleHelper::titleIconHeight(), Utils::WizStyleHelper::titleIconHeight());
    m_editBtn = new WizEditButton(this);
    m_editBtn->setFixedHeight(nTitleHeight);
    QString shortcut = ::WizGetShortcut("EditNote", "Alt+R");
    // FIXME: why shortcut for WizEditButton not work?
    //m_editBtn->setShortcut(QKeySequence::fromString(shortcut));
    m_editBtn->setStatefulIcon(::WizLoadSkinIcon(strTheme, "document_lock", iconSize), WizToolButton::Normal);
    m_editBtn->setStatefulText(tr("Edit"), tr("Switch to Editing View  %1").arg(shortcut), WizToolButton::Normal);
    m_editBtn->setStatefulIcon(::WizLoadSkinIcon(strTheme, "document_unlock", iconSize), WizToolButton::Checked);
    m_editBtn->setStatefulText(tr("Read") , tr("Switch to Reading View  %1").arg(shortcut), WizToolButton::Checked);
    // 准备外置编辑器菜单
    QMenu* extEditorMenu = createEditorMenu();
    m_editBtn->setMenu(extEditorMenu);
    m_editBtn->setPopupMode(QToolButton::MenuButtonPopup);
    connect(m_editBtn, SIGNAL(clicked()), SLOT(onEditButtonClicked()));
    auto edit_shortcut = new QShortcut(shortcut, this);
    connect(edit_shortcut, &QShortcut::activated, this, &WizTitleBar::onEditButtonClicked);

    m_mindmapBtn = new WizToolButton(this, WizToolButton::ImageOnly);
    m_mindmapBtn->setCheckable(true);
    m_mindmapBtn->setChecked(false);
    m_mindmapBtn->setFixedHeight(nTitleHeight);
    m_mindmapBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "outline_mindmap", iconSize), tr("View mindmap"));
    connect(m_mindmapBtn, SIGNAL(clicked()), SLOT(onViewMindMapClicked()));

    // 分离窗口浏览笔记
    m_separateBtn = new WizToolButton(this, WizToolButton::ImageOnly);
    m_separateBtn->setFixedHeight(nTitleHeight);
    QString separateShortcut = ::WizGetShortcut("EditNoteSeparate", "");
    m_separateBtn->setShortcut(QKeySequence::fromString(separateShortcut));
    m_separateBtn->setIcon(::WizLoadSkinIcon(strTheme, "document_use_separate", iconSize));
    m_separateBtn->setToolTip(tr("View note in seperate window  %1").arg(separateShortcut));
    connect(m_separateBtn, SIGNAL(clicked()), SLOT(onSeparateButtonClicked()));

    // 标签按钮
    m_tagBtn = new WizToolButton(this, WizToolButton::ImageOnly);
    m_tagBtn->setFixedHeight(nTitleHeight);
    QString tagsShortcut = ::WizGetShortcut("EditNoteTags", "");
    m_tagBtn->setShortcut(QKeySequence::fromString(tagsShortcut));
    m_tagBtn->setCheckable(true);
    m_tagBtn->setIcon(::WizLoadSkinIcon(strTheme, "document_tag", iconSize));
    m_tagBtn->setToolTip(tr("View and add tags  %1").arg(tagsShortcut));
    connect(m_tagBtn, SIGNAL(clicked()), SLOT(onTagButtonClicked()));

    // 分享按钮
    m_shareBtn = new WizToolButton(this,  WizToolButton::ImageOnly | WizToolButton::WithMenu);
    m_shareBtn->setFixedHeight(nTitleHeight);
    m_shareMenu = new QMenu(m_shareBtn);
    QAction *defaultAc = m_shareMenu->addAction(WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_LINK, this, SLOT(onShareActionClicked()));
    m_shareMenu->addAction(WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_EMAIL, this, SLOT(onEmailActionClicked()));
    QString shareShortcut = ::WizGetShortcut("EditShare", "");
    defaultAc->setShortcut(QKeySequence::fromString(shareShortcut));
    defaultAc->setIcon(::WizLoadSkinIcon(strTheme, "document_share", iconSize));
    defaultAc->setToolTip(tr("Share note  %1").arg(shareShortcut));
    connect(m_shareBtn, SIGNAL(clicked()), SLOT(onShareButtonClicked()));
    WizOEMSettings oemSettings(m_app.databaseManager().db().getAccountPath());
    m_shareBtn->setVisible(!oemSettings.isHideShare());
    m_shareBtn->setPopupMode(QToolButton::MenuButtonPopup);
    m_shareBtn->setMenu(m_shareMenu);
    m_shareBtn->setDefaultAction(defaultAc);

    //隐藏历史版本按钮，给以后增加提醒按钮保留位置
//    WizCellButton* historyBtn = new WizCellButton(WizCellButton::ImageOnly, this);
//    historyBtn->setFixedHeight(nTitleHeight);
//    QString historyShortcut = ::WizGetShortcut("EditNoteHistory", "Alt+5");
//    historyBtn->setShortcut(QKeySequence::fromString(historyShortcut));
//    historyBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_history"), tr("View and recover note's history (Alt + 5)"));
//    connect(historyBtn, SIGNAL(clicked()), SLOT(onHistoryButtonClicked()));

//    m_emailBtn = new CellButton(CellButton::ImageOnly, this);
//    m_emailBtn->setFixedHeight(nTitleHeight);
//    QString emailShortcut = ::WizGetShortcut("EditNoteEmail", "Alt+6");
//    m_emailBtn->setShortcut(QKeySequence::fromString(emailShortcut));
//    m_emailBtn->setNormalIcon(::WizLoadSkinIcon(strTheme, "document_email"), tr("Share document by email (Alt + 6)"));
//    connect(m_emailBtn, SIGNAL(clicked()), SLOT(onEmailButtonClicked()));
//    m_emailBtn->setVisible(!oemSettings.isHideShareByEmail());

    // 笔记信息按钮
    m_infoBtn = new WizToolButton(this, WizToolButton::ImageOnly);
    m_infoBtn->setFixedHeight(nTitleHeight);
    QString infoShortcut = ::WizGetShortcut("EditNoteInfo", "");
    m_infoBtn->setCheckable(true);
    m_infoBtn->setShortcut(QKeySequence::fromString(infoShortcut));
    m_infoBtn->setIcon(::WizLoadSkinIcon(strTheme, "document_info", iconSize));
    m_infoBtn->setToolTip(tr("View and modify note's info  %1").arg(infoShortcut));
    connect(m_infoBtn, SIGNAL(clicked()), SLOT(onInfoButtonClicked()));

    // 附件按钮
    m_attachBtn = new WizToolButton(this, WizToolButton::WithCountInfo);
    m_attachBtn->setFixedHeight(nTitleHeight);
    QString attachmentShortcut = ::WizGetShortcut("EditNoteAttachments", "");
    m_attachBtn->setCheckable(true);
    m_attachBtn->setShortcut(QKeySequence::fromString(attachmentShortcut));
    m_attachBtn->setIcon(::WizLoadSkinIcon(strTheme, "document_attachment", iconSize));
    m_attachBtn->setToolTip(tr("Add attachments  %1").arg(attachmentShortcut));
    connect(m_attachBtn, SIGNAL(clicked()), SLOT(onAttachButtonClicked()));

    // comments
    m_commentsBtn = new WizToolButton(this, WizToolButton::WithCountInfo);
    m_commentsBtn->setFixedHeight(nTitleHeight);
    QString commentShortcut = ::WizGetShortcut("ShowComment", "Alt+C");
    m_commentsBtn->setShortcut(QKeySequence::fromString(commentShortcut));
    m_commentsBtn->setCheckable(true);
    m_commentsBtn->setIcon(::WizLoadSkinIcon(strTheme, "comments", iconSize));
    m_commentsBtn->setToolTip(tr("Add comments  %1").arg(commentShortcut));
    connect(m_commentsBtn, SIGNAL(clicked()), SLOT(onCommentsButtonClicked()));
    connect(WizGlobal::instance(), SIGNAL(viewNoteLoaded(WizDocumentView*,const WIZDOCUMENTDATAEX&,bool)),
            SLOT(onViewNoteLoaded(WizDocumentView*,const WIZDOCUMENTDATAEX&,bool)));

    m_pageZoomBtn = new WizToolButton(this, WizToolButton::ImageOnly);
    m_pageZoomBtn->setFixedHeight(nTitleHeight);
    m_pageZoomBtn->setCheckable(true);
    m_pageZoomBtn->setIcon(::WizLoadSkinIcon(strTheme, "document_zoom", iconSize));
    m_pageZoomBtn->setToolTip(tr("Show page zoom widget"));
    connect(m_pageZoomBtn, &QToolButton::toggled,
            this, &WizTitleBar::onPageZoomButtonToggled);

    // 标题工具栏
    m_documentToolBar->setIconSize(iconSize);
    m_documentToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    m_documentToolBar->setMovable(false);
    m_documentToolBar->addWidget(m_editTitle);
    m_documentToolBar->addWidget(m_editBtn);
    m_documentToolBar->addWidget(new WizFixedSpacer(QSize(7, 1), m_documentToolBar));
    m_mindmapAction = m_documentToolBar->addWidget(m_mindmapBtn);
    m_documentToolBar->addWidget(m_separateBtn);
    m_documentToolBar->addWidget(m_tagBtn);
    m_documentToolBar->addWidget(m_shareBtn);
    m_documentToolBar->addWidget(m_infoBtn);
    m_documentToolBar->addWidget(m_attachBtn);
    m_documentToolBar->addWidget(m_commentsBtn);
    m_documentToolBar->addWidget(m_pageZoomBtn);

    // 标题工具栏布局
    /*
    QHBoxLayout* layoutInfo2 = new QHBoxLayout();
    layoutInfo2->setContentsMargins(0, 0, 0, 0);
    layoutInfo2->setSpacing(0);
    layoutInfo2->addWidget(m_editTitle);
    layoutInfo2->addWidget(m_editBtn);
    //layoutInfo2->addWidget(extEditorButton); // 外置编辑器按钮
    layoutInfo2->addSpacing(::WizSmartScaleUI(7));
    layoutInfo2->addWidget(m_separateBtn);
    layoutInfo2->addWidget(m_tagBtn);
    layoutInfo2->addWidget(m_shareBtn);
    //隐藏历史版本按钮，给以后增加提醒按钮保留位置
    //layoutInfo2->addWidget(historyBtn);
//    layoutInfo2->addWidget(m_emailBtn);
    layoutInfo2->addWidget(m_infoBtn);
    layoutInfo2->addWidget(m_attachBtn);
    layoutInfo2->addWidget(m_commentsBtn);
    */

    initPlugins();

    // 笔记状态信息布局
    QVBoxLayout* layoutInfo1 = new QVBoxLayout();
    layoutInfo1->setContentsMargins(0, 0, 0, 0);
    layoutInfo1->setSpacing(0);
    //layoutInfo1->addLayout(layoutInfo2); // 将标题工具栏放入整体布局中
    layoutInfo1->addWidget(m_documentToolBar);
    layoutInfo1->addWidget(m_tagBar);
    layoutInfo1->addWidget(m_notifyBar);

    layout->addLayout(layoutInfo1);
    //layout->addLayout(layoutInfo4);

    layout->addStretch();
    connect(m_notifyBar, SIGNAL(labelLink_clicked(QString)), SIGNAL(notifyBar_link_clicked(QString)));
    connect(m_tagBar, SIGNAL(widgetStatusChanged()), SLOT(updateTagButtonStatus()));

    connect(m_commentManager, SIGNAL(tokenAcquired(QString)),
            SLOT(on_commentTokenAcquired(QString)));
    connect(m_commentManager, SIGNAL(commentCountAcquired(QString,int)),
            SLOT(on_commentCountAcquired(QString,int)));
}

/*!
    Init plugins' tool button on document tool bar.
*/
void WizTitleBar::initPlugins()
{
    int nTitleHeight = Utils::WizStyleHelper::titleEditorHeight();
    JSPluginManager &jsPluginMgr = JSPluginManager::instance();

    auto menus = jsPluginMgr.modulesByModuleType("Menu");
    foreach (auto menuData, menus) {
        if (menuData->spec()->buttonLocation() != "Document")
            continue;
        auto btn = new WizToolButton(this, WizToolButton::ImageOnly | WizToolButton::WithMenu);
        btn->setFixedHeight(nTitleHeight);
        btn->setPopupMode(QToolButton::MenuButtonPopup);
        QMenu *m = new QMenu(btn);
        foreach (int i, menuData->spec()->actionIndexes()) {
            auto parent = menuData->parentPlugin();
            auto acm = parent->module(i);
            QAction *ac = jsPluginMgr.createPluginAction(btn, acm);
            m->addAction(ac);
            if (acm->spec()->slotType() == "DocumentSidebar") {
                ac->setCheckable(true);
                connect(ac, &QAction::triggered, this, [this, ac] (bool checked) {
                    Q_EMIT pluginSidebarRequest(ac, checked);
                });
            } else {
                connect(ac, &QAction::triggered, this,
                    [this, btn, ac] (bool checked) {
                        QRect rc = btn->rect();
                        QPoint pt = btn->mapToGlobal(QPoint(rc.width()/2, rc.height()));
                        Q_EMIT pluginPopupRequest(ac, pt);
                    }
                );
            }

        }

        btn->setMenu(m);
        auto acs = m->actions();
        if (!acs.isEmpty()) {
            btn->setDefaultAction(acs.first());
            m_documentToolBar->addWidget(btn);
        }
    }

    QList<JSPluginModule *> modules = jsPluginMgr.modulesByKeyValue("ModuleType", "Action");
    foreach (auto moduleData, modules) {
        if (moduleData->spec()->buttonLocation() != "Document")
            continue;
        auto btn = new WizToolButton(this, WizToolButton::ImageOnly);
        QAction *ac = m_documentToolBar->addWidget(btn);
        JSPluginManager::initPluginAction(ac, moduleData);
        btn->setDefaultAction(ac);
        btn->setFixedHeight(nTitleHeight);
        QString slotType = moduleData->spec()->slotType();
        if (slotType == "DocumentSidebar") {
            ac->setCheckable(true);
            connect(ac, &QAction::triggered, this, [this, ac] (bool checked) {
                Q_EMIT pluginSidebarRequest(ac, checked);
            });

        } else if (slotType == "SelectorWindow") {
            connect(btn, &QToolButton::triggered, this, &WizTitleBar::handlePluginPopup);
        }
    }

    connect(this, &WizTitleBar::pluginPopupRequest,
            &jsPluginMgr, &JSPluginManager::handlePluginPopupRequest);
    connect(this, &WizTitleBar::launchPluginEditorRequest,
            &jsPluginMgr, &JSPluginManager::handlePluginEditorRequest);
}

// FIXME: urgly impl
WizDocumentView* WizTitleBar::noteView()
{
    QWidget* pParent = parentWidget();
    while (pParent) {
        WizDocumentView* view = dynamic_cast<WizDocumentView*>(pParent);
        if (view)
            return view;

        pParent = pParent->parentWidget();
    }

    Q_ASSERT(0);
    return 0;
}

void WizTitleBar::setLocked(bool bReadOnly, int nReason, bool bIsGroup)
{
    m_notifyBar->showPermissionNotify(nReason);
    m_editTitle->setReadOnly(bReadOnly);
    m_editBtn->setEnabled(!bReadOnly);

    if (nReason == WizNotifyBar::Deleted)
    {
        m_tagBtn->setEnabled(false);
        m_commentsBtn->setEnabled(false);
        foreach (QAction* action , m_shareMenu->actions())
        {
            action->setEnabled(false);
        }
    }
    else
    {
        WizOEMSettings oemSettings(m_app.databaseManager().db().getAccountPath());
        m_tagBtn->setVisible(bIsGroup ? false : true);
        m_tagBtn->setEnabled(bIsGroup ? false : true);
        m_commentsBtn->setEnabled(true);
        foreach (QAction* action , m_shareMenu->actions())
        {
            if (action->text() == WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_EMAIL)
            {
                action->setEnabled(!oemSettings.isHideShareByEmail());
                action->setVisible(!oemSettings.isHideShareByEmail());
            }
            else if (action->text() == WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_LINK)
            {
                action->setEnabled(!oemSettings.isHideShare());
                action->setVisible(!oemSettings.isHideShare());
            }
        }
    }
}

QMenu* WizTitleBar::createEditorMenu()
{
    QMenu* editorMenu = new QMenu(this);
    // Check mode
    if (noteView()->editorMode() == modeEditor) {
        // Editing mode only allow one action.
        editorMenu->addAction(tr("Discard changes"), this, &WizTitleBar::handleDiscardChanges);
        return editorMenu;
    }
    // External editor option
    editorMenu->addAction(tr("Editor Options"), this, SLOT(onEditorOptionSelected()));
    editorMenu->addSeparator();

    // Load JSPlugin type editor
    JSPluginManager &jsPluginMgr = JSPluginManager::instance();
    QList<JSPluginModule *> modules = jsPluginMgr.modulesByKeyValue("ModuleType", "Editor");
    for (auto module : modules) {
        QAction *editorAction = jsPluginMgr.createPluginAction(this, module);
        connect(editorAction, &QAction::triggered,
            this, &WizTitleBar::handlePluginEditorActionTriggered);
        editorMenu->addAction(editorAction);
    }

    // Reading External editor settings
    QSettings* extEditorSettings = new QSettings(
                Utils::WizPathResolve::dataStorePath() + "externalEditor.ini", QSettings::IniFormat);
    QStringList groups = extEditorSettings->childGroups();
    for (QString& editorIndex : groups) {
        extEditorSettings->beginGroup(editorIndex);
        // Prepare metadata of external editor
        QMap<QString, QVariant> data;
        data["Name"] = extEditorSettings->value("Name");
        data["ProgramFile"] = extEditorSettings->value("ProgramFile");
        data["Arguments"] = extEditorSettings->value("Arguments", "%1");
        data["TextEditor"] = extEditorSettings->value("TextEditor", 0);
        data["UTF8Encoding"] = extEditorSettings->value("UTF8Encoding", 0);
        data["OpenShortCut"] = extEditorSettings->value("OpenShortCut", "");
        // Create actions
        QAction* editorAction = editorMenu->addAction(
            data.value("Name").toString(), this, SLOT(onExternalEditorMenuSelected()));
        QVariant var(data);
        editorAction->setData(var);

        QString shortcut = data["OpenShortCut"].toString();
        if (!shortcut.isEmpty()) {
            editorAction->setShortcut(QKeySequence::fromString(shortcut));
        }

        extEditorSettings->endGroup();
    }

    return editorMenu;
}

void WizTitleBar::handlePluginEditorActionTriggered()
{
    QAction *ac = qobject_cast<QAction *>(sender());
    if (!ac)
        return;

    QString moduleGuid = ac->data().toString();
    if (moduleGuid.isEmpty())
        return;

    const WIZDOCUMENTDATA &doc = noteView()->note();
    emit launchPluginEditorRequest(doc, moduleGuid);
}

void WizTitleBar::setEditor(AbstractDocumentEditor* editor)
{
    Q_ASSERT(!m_editor);
    m_editor = editor;
}

void WizTitleBar::setBackgroundColor(QColor color)
{
    QPalette pal = m_editTitle->palette();
    pal.setColor(QPalette::Window, color);
    m_editTitle->setPalette(pal);
}

void WizTitleBar::updateTagButtonStatus()
{
    if (m_tagBar && m_tagBtn)
    {
        m_tagBtn->setChecked(m_tagBar->isVisible());
        m_tagBtn->setState(m_tagBar->isVisible() ? WizToolButton::Checked : WizToolButton::Normal);
    }
}

void WizTitleBar::updateAttachButtonStatus()
{
    if (m_attachments && m_attachBtn)
    {
        m_attachBtn->setChecked(m_attachments->isVisible());
        m_attachBtn->setState(m_attachments->isVisible() ? WizCellButton::Checked : WizCellButton::Normal);
    }
}

void WizTitleBar::updateInfoButtonStatus()
{
    if (m_info && m_infoBtn)
    {
        m_infoBtn->setChecked(m_info->isVisible());
        m_infoBtn->setState(m_info->isVisible() ? WizToolButton::Checked : WizToolButton::Normal);
    }
}

void WizTitleBar::updateCommentsButtonStatus()
{
    if (m_commentsBtn && noteView()->commentWidget())
    {
        m_commentsBtn->setChecked(noteView()->commentWidget()->isVisible());
        m_commentsBtn->setState(noteView()->commentWidget()->isVisible() ? WizCellButton::Checked : WizCellButton::Normal);
    }
}

void WizTitleBar::onTitleEditFinished()
{
    m_editTitle->onTitleEditingFinished();
}

void WizTitleBar::loadErrorPage()
{
    QWebEngineView* comments = noteView()->commentView();
    QString strFileName = Utils::WizPathResolve::resourcesPath() + "files/errorpage/load_fail_comments.html";
    QString strHtml;
    ::WizLoadUnicodeTextFromFile(strFileName, strHtml);
    // clear old url
    comments->load(QUrl());
    QUrl url = QUrl::fromLocalFile(strFileName);
    comments->load(url);
}

void WizTitleBar::setTagBarVisible(bool visible)
{
    m_tagBar->setVisible(visible);
}


void WizTitleBar::onEditorChanged()
{
    updateEditButton(noteView()->editorMode());
}

void WizTitleBar::setNote(const WIZDOCUMENTDATA& data, WizEditorMode editorMode, bool locked)
{
    updateInfo(data);
    setEditorMode(editorMode);

    WizDatabase& db = m_app.databaseManager().db(data.strKbGUID);
    bool isGroup = db.isGroup();
    int nTagCount = db.getDocumentTagCount(data.strGUID);
    setTagBarVisible(!isGroup && nTagCount > 0);
    if (!isGroup)
    {
        m_tagBar->setDocument(data);
    }

    m_mindmapAction->setVisible(data.strType == "outline");
    m_mindmapBtn->setChecked(false);
}

void WizTitleBar::updateInfo(const WIZDOCUMENTDATA& doc)
{
    m_editTitle->setText(doc.strTitle);
    m_attachBtn->setCount(doc.nAttachmentCount);
}

void WizTitleBar::setEditorMode(WizEditorMode editorMode)
{
    m_editTitle->setReadOnly(editorMode == modeReader);
    m_editBtn->setState(editorMode == modeEditor ? WizEditButton::Checked : WizEditButton::Normal);
    // Refresh button side menu
    m_editBtn->menu()->deleteLater();
    QMenu* extEditorMenu = createEditorMenu();
    m_editBtn->setMenu(extEditorMenu);
}

void WizTitleBar::setEditButtonEnabled(bool enable)
{
    m_editBtn->setEnabled(enable);
}

void WizTitleBar::updateEditButton(WizEditorMode editorMode)
{
    m_editor->isModified([=](bool modified) {
        if (modified){
            m_editBtn->setState(WizEditButton::Badge);
        } else {
            m_editBtn->setState(editorMode == modeEditor ? WizEditButton::Checked : WizEditButton::Normal);
        }
    });
}

void WizTitleBar::resetTitle(const QString& strTitle)
{
    m_editTitle->resetTitle(strTitle);

}

void WizTitleBar::clearAndSetPlaceHolderText(const QString& text)
{
    m_editTitle->setPlaceholderText(text);
    m_editTitle->clear();
}

void WizTitleBar::clearPlaceHolderText()
{
    m_editTitle->setPlaceholderText("");
}

#define TITLEBARTIPSCHECKED        "TitleBarTipsChecked"

void WizTitleBar::showCoachingTips()
{
    bool showTips = false;
    if (WizMainWindow* mainWindow = WizMainWindow::instance())
    {
        showTips = mainWindow->userSettings().get(TITLEBARTIPSCHECKED).toInt() == 0;
    }

    if (showTips && !WizTipsWidget::isTipsExists(TITLEBARTIPSCHECKED))
    {
        WizTipsWidget* widget = new WizTipsWidget(TITLEBARTIPSCHECKED, this);
        connect(m_editBtn, SIGNAL(clicked(bool)), widget, SLOT(on_targetWidgetClicked()));
        widget->setAttribute(Qt::WA_DeleteOnClose, true);
        widget->setText(tr("Switch to reading mode"), tr("In reading mode, the note can not be "
                                                         "edited and markdown note can be redered."));
        widget->setSizeHint(QSize(290, 82));
        widget->setButtonVisible(false);
        widget->bindCloseFunction([](){
            if (WizMainWindow* mainWindow = WizMainWindow::instance())
            {
                mainWindow->userSettings().set(TITLEBARTIPSCHECKED, "1");
            }
        });
        //
        widget->bindTargetWidget(m_editBtn, 0, -2);
        QTimer::singleShot(500, widget, SLOT(on_showRequest()));
    }
}

void WizTitleBar::startEditButtonAnimation()
{
    if (!m_editButtonAnimation)
    {
        m_editButtonAnimation = new WizAnimateAction(this);
        m_editButtonAnimation->setToolButton(m_editBtn);
        m_editButtonAnimation->setSingleIcons("editButtonProcessing");
    }
    m_editButtonAnimation->startPlay();
}

void WizTitleBar::stopEditButtonAnimation()
{
    if (!m_editButtonAnimation)
        return;
    if (m_editButtonAnimation->isPlaying())
    {
        m_editButtonAnimation->stopPlay();
    }
}

void WizTitleBar::applyButtonStateForSeparateWindow(bool inSeparateWindow)
{
    m_separateBtn->setVisible(!inSeparateWindow);
}

/**
 * @brief Switch editor mode when button clicked.
 *
 * FIXME: use signal-slot instead.
 */
void WizTitleBar::onEditButtonClicked()
{
    // Switch editor mode
    noteView()->setEditorMode(noteView()->editorMode() == modeEditor ? modeReader: modeEditor);
    //
    WizAnalyzer& analyzer = WizAnalyzer::getAnalyzer();
    if (noteView()->isEditing())
    {
        analyzer.logAction("editNote");
    }
    else
    {
        analyzer.logAction("viewNote");
    }
}

/**
 * @brief 弹出编辑器选项对话框
 */
void WizTitleBar::onEditorOptionSelected()
{
    WizExternalEditorSettingDialog* editorSetting = new WizExternalEditorSettingDialog(this);
    connect(editorSetting, &WizExternalEditorSettingDialog::settingChanged, this, [=]{
        QMenu* m = createEditorMenu();
        m_editBtn->setMenu(m);
    });
    editorSetting->setAttribute(Qt::WA_DeleteOnClose);
    editorSetting->exec();
}

/**
 * @brief 发送外置编辑器请求信号
 */
void WizTitleBar::onExternalEditorMenuSelected()
{
    QAction* action = qobject_cast<QAction*>(sender());
    QMap<QString, QVariant> data;
    data = action->data().toMap();
    QString Name = data.value("Name").toString();
    QString ProgramFile = data.value("ProgramFile").toString();
    QString Arguments = data.value("Arguments").toString();
    int TextEditor = data.value("TextEditor").toInt();
    int UTF8Encoding = data.value("UTF8Encoding").toInt();

    emit viewNoteInExternalEditor_request(Name, ProgramFile, Arguments, TextEditor, UTF8Encoding);

}

void WizTitleBar::handleDiscardChanges()
{
    // Confirm
    QMessageBox::StandardButton res = QMessageBox::question(this,
        tr("Discard changes"), tr("Do you really want to discard changes ?"));
    if (res == QMessageBox::Yes) {
        emit discardChangesRequest();
    } else {
        return;
    }
}

void WizTitleBar::onSeparateButtonClicked()
{
    WizGetAnalyzer().logAction("titleBarViewInSeperateWindow");

    emit viewNoteInSeparateWindow_request();
}

void WizTitleBar::onTagButtonClicked()
{
    setTagBarVisible(!m_tagBar->isVisible());

    WizGetAnalyzer().logAction("showTags");
}

void WizTitleBar::onViewMindMapClicked()
{
    bool on = m_mindmapBtn->isChecked();
    //m_mindmapBtn->setChecked(on);
    emit onViewMindMap(on);
    m_mindmapBtn->setState(on ? WizCellButton::Checked : WizCellButton::Normal);
}

void WizTitleBar::handlePluginPopup(QAction* ac)
{
    QWidget *btn = m_documentToolBar->widgetForAction(ac);

    QRect rc = btn->rect();
    QPoint pt = btn->mapToGlobal(QPoint(rc.width()/2, rc.height()));

    Q_EMIT pluginPopupRequest(ac, pt);
}

QAction* actionFromMenu(QMenu* menu, const QString& text)
{
    QList<QAction*> actionList = menu->actions();
    for (QAction* action : actionList)
    {
        if (action->text() == text)
            return action;
    }
    return nullptr;
}

void WizTitleBar::onPageZoomButtonToggled(bool checked)
{
    QToolButton* btn = qobject_cast<QToolButton *>(sender());
    if (!btn) return;

    QPoint topLeft = mapToGlobal(btn->pos());
    QRect location(topLeft, btn->size());

    Q_EMIT showPageZoomWidgetRequested(checked, location);
}

void WizTitleBar::onPageZoomWidgetClosed()
{
    m_pageZoomBtn->setChecked(false);
    m_pageZoomBtn->setState(WizToolButton::Normal);
}

void WizTitleBar::onShareButtonClicked()
{
    if (m_shareMenu)
    {
        const WIZDOCUMENTDATA& doc = noteView()->note();

        QAction* actionLink = actionFromMenu(m_shareMenu, WIZACTION_TITLEBAR_SHARE_DOCUMENT_BY_LINK);
        if (actionLink)
        {
            actionLink->setVisible(true);
            if (m_app.databaseManager().db(doc.strKbGUID).isGroup())
            {
                WIZGROUPDATA group;
                m_app.databaseManager().db().getGroupData(doc.strKbGUID, group);
                if (!group.isBiz())
                {
                    actionLink->setVisible(false);
                }
            }
        }

        //QPoint pos = m_shareBtn->mapToGlobal(m_shareBtn->rect().bottomLeft());
        //m_shareMenu->popup(pos);
    }
}

void WizTitleBar::onEmailActionClicked()
{
    WizGetAnalyzer().logAction("shareByEmail");

    Q_EMIT shareNoteByEmailRequest();
}

void WizTitleBar::onShareActionClicked()
{
    WizGetAnalyzer().logAction("shareByLink");

    Q_EMIT shareNoteByLinkRequest();
}

void WizTitleBar::onAttachButtonClicked()
{
    if (!m_attachments) {
        m_attachments = new WizAttachmentListWidget(topLevelWidget());
        connect(m_attachments, SIGNAL(widgetStatusChanged()), SLOT(updateAttachButtonStatus()));
    }


    if (m_attachments->setDocument(noteView()->note()))
    {
        QRect rc = m_attachBtn->rect();
        QPoint pt = m_attachBtn->mapToGlobal(QPoint(rc.width()/2, rc.height()));
        m_attachments->showAtPoint(pt);
        updateAttachButtonStatus();
    }

    WizGetAnalyzer().logAction("showAttachments");
}

void WizTitleBar::onHistoryButtonClicked()
{
    const WIZDOCUMENTDATA& doc = noteView()->note();

    WizShowDocumentHistory(doc, noteView());

    WizGetAnalyzer().logAction("showHistory");
}


void WizTitleBar::onInfoButtonClicked()
{
    if (!m_info) {
        m_info = new WizNoteInfoForm(topLevelWidget());
        connect(m_info, SIGNAL(widgetStatusChanged()), SLOT(updateInfoButtonStatus()));
    }

    m_info->setDocument(noteView()->note());
    //
    noteView()->wordCount([=](const QString& json){
        //
        Json::Value d;
        Json::Reader reader;
        if (!reader.parse(json.toUtf8().constData(), d))
            return;

        try {
            int nWords = d["nWords"].asInt();
            int nChars = d["nChars"].asInt();
            int nCharsWithSpace = d["nCharsWithSpace"].asInt();
            int nNonAsianWords = d["nNonAsianWords"].asInt();
            int nAsianChars = d["nAsianChars"].asInt();
            //
            m_info->setWordCount(nWords, nChars, nCharsWithSpace, nNonAsianWords, nAsianChars);


        } catch (...) {

        }
    });

    QRect rc = m_infoBtn->rect();
    QPoint pt = m_infoBtn->mapToGlobal(QPoint(rc.width()/2, rc.height()));
    m_info->showAtPoint(pt);
    updateInfoButtonStatus();

    WizGetAnalyzer().logAction("showNoteInfo");
}

bool isNetworkAccessible()
{
    return true;
    //QNetworkConfigurationManager man;
    //return man.isOnline();
}

#define COMMENT_FRAME_WIDTH 315

void WizTitleBar::onCommentsButtonClicked()
{
    QWebEngineView* comments = noteView()->commentView();

    WizDocumentView* view = noteView();
    if (!view)
        return;

    WizLocalProgressWebView* commentWidget = view->commentWidget();
    connect(commentWidget, SIGNAL(widgetStatusChanged()), this,
            SLOT(updateCommentsButtonStatus()), Qt::UniqueConnection);


    if (commentWidget->isVisible()) {
        commentWidget->hide();

        WizGetAnalyzer().logAction("hideComments");

        return;
    }

    WizGetAnalyzer().logAction("showComments");

    if (isNetworkAccessible()) {
        QSplitter* splitter = qobject_cast<QSplitter*>(commentWidget->parentWidget());
        Q_ASSERT(splitter);
        QList<int> li = splitter->sizes();
        Q_ASSERT(li.size() == 2);
        QList<int> lin;
        lin.push_back(li.value(0) - COMMENT_FRAME_WIDTH);
        lin.push_back(li.value(1) + COMMENT_FRAME_WIDTH);
        splitter->setSizes(lin);
        commentWidget->show();

        m_commentManager->queryCommentCount(view->note().strKbGUID, view->note().strGUID, true);
    } else {
        m_commentsBtn->setEnabled(false);
    }
}

void WizTitleBar::onCommentPageLoaded(bool ok)
{
    WizLocalProgressWebView* commentWidget = noteView()->commentWidget();

    if (!ok)
    {
        qDebug() << "Wow, load comment page failed! " << commentWidget->web()->url();
        //失败的时候会造成死循环
        //loadErrorPage();
        commentWidget->show();
    }
}

void WizTitleBar::onViewNoteLoaded(WizDocumentView* view, const WIZDOCUMENTDATAEX& note, bool bOk)
{
    if (!bOk)
        return;

    if (view != noteView()) {
        return;
    }

    m_commentsBtn->setCount(0);
    m_commentManager->queryCommentCount(note.strKbGUID, note.strGUID, true);
}

void WizTitleBar::on_commentTokenAcquired(QString token)
{
    WizDocumentView* view = noteView();
    if (view)
    {
        WizLocalProgressWebView* commentWidget = view->commentWidget();
        if (commentWidget && commentWidget->isVisible())
        {
            if (token.isEmpty())
            {
                qDebug() << "Wow, query token= failed!";
                loadErrorPage();
            }
            else
            {
                WIZDOCUMENTDATA note = view->note();

                QString js = QString("updateCmt('%1','%2','%3')").arg(token).arg(note.strKbGUID).arg(note.strGUID);
#ifdef QT_DEBUG
                qDebug() << js;
#endif
                commentWidget->web()->page()->runJavaScript(js, [=](const QVariant& vRet){
                    if (!vRet.toBool())
                    {
                        QString commentUrlTemplate = m_commentManager->getCommentUrlTemplate();
                        if (!commentUrlTemplate.isEmpty())
                        {
                            QString strUrl = commentUrlTemplate;
                            strUrl.replace("{token}", token);
                            strUrl.replace("{kbGuid}", note.strKbGUID);
                            strUrl.replace("{documentGuid}", note.strGUID);
                            commentWidget->web()->load(strUrl);
                        }
                    }
                });
            }
        }
    }
}

void WizTitleBar::on_commentCountAcquired(QString GUID, int count)
{
    WizDocumentView* view = noteView();
    if (view && view->note().strGUID == GUID)
    {
        m_commentsBtn->setCount(count);
    }
}

void WizTitleBar::showMessageTips(Qt::TextFormat format, const QString& strInfo)
{
    m_notifyBar->showMessageTips(format, strInfo);
}

void WizTitleBar::hideMessageTips(bool useAnimation)
{
    m_notifyBar->hideMessageTips(useAnimation);
}

WizTitleEdit* WizTitleBar::getTitleEdit()
{
    return m_editTitle;
}

//////////////////////////////////////////////////////////////////

CollaborationTitleBar::CollaborationTitleBar(WizExplorerApp& app, QWidget *parent)
    : QWidget(parent)
    , m_app(app)
    , m_documentToolBar(new QToolBar(this))
    , m_editButtonAnimation(nullptr)
{
    setObjectName("collaboration-title-bar");

    int nTitleHeight = Utils::WizStyleHelper::titleEditorHeight();
    QString strTheme = Utils::WizStyleHelper::themeName();

    // Edit button
    QSize iconSize = QSize(Utils::WizStyleHelper::titleIconHeight(), Utils::WizStyleHelper::titleIconHeight());
    m_editBtn = new WizEditButton(this);
    m_editBtn->setFixedHeight(nTitleHeight);
    QString shortcut = ::WizGetShortcut("EditNote", "Alt+R");
    m_editBtn->setStatefulIcon(::WizLoadSkinIcon(strTheme, "document_lock", iconSize), WizToolButton::Normal);
    m_editBtn->setStatefulText(tr("Edit"), tr("Switch to Editing View  %1").arg(shortcut), WizToolButton::Normal);
    m_editBtn->setStatefulIcon(::WizLoadSkinIcon(strTheme, "document_unlock", iconSize), WizToolButton::Checked);
    m_editBtn->setStatefulText(tr("Read") , tr("Switch to Reading View  %1").arg(shortcut), WizToolButton::Checked);
    connect(m_editBtn, &WizToolButton::clicked, this, &CollaborationTitleBar::editButtonClicked);
    m_editBtn->setState(WizToolButton::Normal);

    //m_editBtn->setShortcut(QKeySequence::fromString(shortcut));
    // Title bar toolbar
    m_documentToolBar->setIconSize(iconSize);
    m_documentToolBar->setContextMenuPolicy(Qt::PreventContextMenu);
    m_documentToolBar->setMovable(false);
    QWidget* spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_documentToolBar->addWidget(spacer);
    m_documentToolBar->addWidget(m_editBtn);
    m_documentToolBar->addWidget(new WizFixedSpacer(QSize(10, 1)));

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);
    layout->addWidget(m_documentToolBar);

}

void CollaborationTitleBar::startEditButtonAnimation()
{
    if (!m_editButtonAnimation)
    {
        m_editButtonAnimation = new WizAnimateAction(this);
        m_editButtonAnimation->setToolButton(m_editBtn);
        m_editButtonAnimation->setSingleIcons("editButtonProcessing");
    }
    m_editButtonAnimation->startPlay();
}


void CollaborationTitleBar::stopEditButtonAnimation()
{
    if (!m_editButtonAnimation)
        return;
    if (m_editButtonAnimation->isPlaying())
    {
        m_editButtonAnimation->stopPlay();
    }
}



