#include "JSPluginManager.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QToolBar>
#include <QToolButton>
#include <QAction>
#include <QStyle>
#include <QDir>
#include <QMovie>
#include <QVBoxLayout>
#include <QStyleFactory>
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <QNetworkReply>
#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QMessageBox>
#include <QJSEngine>
#include <QQmlEngine>

#include "sync/WizToken.h"
#include "WizMainWindow.h"
#include "share/WizMisc.h"
#include "share/WizGlobal.h"
#include "database/WizDatabaseManager.h"
#include "utils/WizPathResolve.h"
#include "widgets/WizLocalProgressWebView.h"
#include "html/WizHtmlTool.h"
#include "share/WizSettings.h"
#include "share/WizWebEngineView.h"
#include "share/WizMisc.h"
#include "gui/tabbrowser/WizWebsiteView.h"
#include "gui/tabbrowser/WizMainTabBrowser.h"
#include "JSPlugin.h"
#include "JSPluginSpec.h"
#include "JSPluginWidgets.h"

JSPluginManager::JSPluginManager()
    : QObject(nullptr)
    , m_mainWindow(nullptr)
{
    QStringList pluginScanPathList = Utils::WizPathResolve::pluginsAllPath();
    for (QString &path : pluginScanPathList) {
        loadPluginData(path);
    }
}

JSPluginManager::~JSPluginManager()
{
    for (auto htmlDialog : m_pluginHtmlDialogCollection) {
        delete htmlDialog;
    }
    m_pluginHtmlDialogCollection.clear();
    
    for (auto selectorWindow : m_pluginHtmlDialogCollection) {
        delete selectorWindow;
    }
    m_pluginHtmlDialogCollection.clear();

}

void JSPluginManager::loadPluginData(QString &pluginScanPath)
{
    CWizStdStringArray folders;
    WizEnumFolders(pluginScanPath, folders, 0);
    
    for (auto folder : folders) {
        if (!QDir(folder).exists("manifest.ini"))
            continue;
        JSPlugin* data = new JSPlugin(folder, this);
        if (data->isAvailable()) {
            m_pluginDataCollection.push_back(data);
            qDebug() << QString("Loaded plugin: %1").arg(data->spec()->name());
        } else {
            qWarning() << QString("Failed to load plugin: %1").arg(data->spec()->name());
            delete data;
        }
    }
}

QList<JSPluginModule *> JSPluginManager::modulesByButtonLocation(QString buttonLocation) const
{
    QList<JSPluginModule *> ret;
    for (JSPlugin *pluginData : m_pluginDataCollection) {
        for (JSPluginModule *moduleData : pluginData->modules()) {
            if (moduleData->spec()->buttonLocation() == buttonLocation) {
                ret.push_back(moduleData);
            }
        }
    }
    return ret;
}

QList<JSPluginModule *> JSPluginManager::modulesByModuleType(QString type) const
{
    QList<JSPluginModule *> ret;
    for (JSPlugin *pluginData : m_pluginDataCollection) {
        for (JSPluginModule *moduleData : pluginData->modules()) {
            if (moduleData->spec()->moduleType() == type) {
                ret.push_back(moduleData);
            }
        }
    }
    return ret;
}

QList<JSPluginModule *> JSPluginManager::modulesByKeyValue(QString key, QString value) const
{
    QList<JSPluginModule *> ret;
    for (JSPlugin *pluginData : m_pluginDataCollection) {
        QSettings *settings = pluginData->spec()->settings();
        for (JSPluginModule *moduleData : pluginData->modules()) {
            QString section = moduleData->spec()->section();
            if (settings->value(section + "/" + key) == value) {
                ret.push_back(moduleData);
            }
        }
    }
    return ret;
}

JSPluginModule *JSPluginManager::moduleByGUID(QString guid) const
{
    JSPluginModule *ret = nullptr;
    for (JSPlugin *pluginData : m_pluginDataCollection) {
        for (JSPluginModule *moduleData : pluginData->modules()) {
            if (moduleData->spec()->guid() == guid) {
                ret = moduleData;
            }
        }
    }

    return ret;
}

QAction *JSPluginManager::createPluginAction(QWidget *parent, JSPluginModule *moduleData)
{
    QAction *ac = new QAction(parent);
    initPluginAction(ac, moduleData);
    return ac;
}

void JSPluginManager::initPluginAction(QAction *ac, JSPluginModule *moduleData)
{
    ac->setData(moduleData->spec()->guid());
    ac->setIcon(QIcon(moduleData->spec()->iconFileName()));
    ac->setIconText(moduleData->spec()->caption());
    ac->setText(moduleData->spec()->caption());
    ac->setToolTip(moduleData->spec()->caption());
}

JSPluginHtmlDialog *JSPluginManager::initPluginHtmlDialog(JSPluginModule *moduleData)
{
    JSPluginHtmlDialog *htmlDialog = new JSPluginHtmlDialog(*m_mainWindow, moduleData, nullptr);
    m_pluginHtmlDialogCollection.insert(moduleData->spec()->guid(), htmlDialog);
    return htmlDialog;
}

WizWebsiteView *JSPluginManager::initPluginMainTabView(JSPluginModule *moduleData)
{
    WizWebEngineInjectObjectCollection objects = {
        {"JSPlugin", moduleData->parentPlugin()},
        {"JSPluginModule", moduleData},
        {"WizExplorerApp", m_mainWindow->publicAPIsObject()}
    };
    WizWebEngineView *webView = new WizWebEngineView(objects, nullptr);
    QPointer<WizWebsiteView> websiteView = new WizWebsiteView(webView, *m_mainWindow);
    websiteView->viewHtml(QUrl::fromLocalFile(moduleData->spec()->htmlFileName()));
    m_pluginMainTabViewCollection.insert(moduleData->spec()->guid(), websiteView);
    return websiteView;
}

void JSPluginManager::showPluginHtmlDialog(JSPluginModule *moduleData)
{
    QString guid = moduleData->spec()->guid();
    JSPluginHtmlDialog* dialog;
    auto it = m_pluginHtmlDialogCollection.find(guid);
    if ( it == m_pluginHtmlDialogCollection.end()) {
        dialog = initPluginHtmlDialog(moduleData);
    } else {
        dialog = it.value();
    }
    //
    moduleData->emitShowEvent();
    dialog->setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            dialog->dialogSize(),
            qApp->desktop()->availableGeometry()
        )
    );
    dialog->show();
    dialog->raise();
}

void JSPluginManager::showPluginSelectorWindow(JSPluginModule *moduleData, const QPoint &pt)
{
    QString guid = moduleData->spec()->guid();
    JSPluginSelectorWindow *selectorWindow = new JSPluginSelectorWindow(*m_mainWindow, moduleData, nullptr);
    selectorWindow->setAttribute(Qt::WA_DeleteOnClose);
    moduleData->emitShowEvent();
    selectorWindow->showAtPoint(pt);
}

void JSPluginManager::showPluginMainTabView(JSPluginModule *moduleData)
{
    QString guid = moduleData->spec()->guid();
    QPointer<WizWebsiteView> mainTabView;
    WizMainTabBrowser *tabBrowser = m_mainWindow->mainTabView();
    auto it = m_pluginMainTabViewCollection.find(guid);
    if ( it == m_pluginMainTabViewCollection.end() || it.value().isNull() ) {
        // create one
        mainTabView = initPluginMainTabView(moduleData);
        tabBrowser->createTab(mainTabView);
    } else {
        mainTabView = it.value();
        if (!mainTabView.isNull())
            tabBrowser->setCurrentWidget(mainTabView);
    }
    //
    moduleData->emitShowEvent();
}

void JSPluginManager::handlePluginActionTriggered()
{
    QAction *ac = qobject_cast<QAction *>(sender());
    if (!ac)
        return;

    QString moduleGuid = ac->data().toString();
    if (moduleGuid.isEmpty())
        return;

    JSPluginModule *moduleData = moduleByGUID(moduleGuid);
    if (!moduleData)
        return;

    QString slotType = moduleData->spec()->slotType();
    if ( slotType == "SelectorWindow" ) {
        QToolBar *bar = qobject_cast<QToolBar *>(ac->parentWidget());
        if (!bar)
            return;
        QToolButton *button = qobject_cast<QToolButton *>(bar->widgetForAction(ac));
        if (!button)
            return;
        QRect rc = button->rect();
        QPoint pt = button->mapToGlobal(QPoint(rc.width()/2, rc.height()));
        showPluginSelectorWindow(moduleData, pt);
    } else if ( slotType == "HtmlDialog" ) {
        showPluginHtmlDialog(moduleData);
    } else if ( slotType == "MainTabView") {
        showPluginMainTabView(moduleData);
    } else if ( slotType == "ExecuteScript" ) {
        executeModuleScript(moduleData);
    }
}

void JSPluginManager::handlePluginPopupRequest(QAction *ac, const QPoint &pos)
{
    QString moduleGuid = ac->data().toString();
    if (moduleGuid.isEmpty())
        return;

    JSPluginModule *moduleData = moduleByGUID(moduleGuid);
    if (!moduleData)
        return;

    QString slotType = moduleData->spec()->slotType();
    if ( slotType == "SelectorWindow" ) {
        showPluginSelectorWindow(moduleData, pos);
    } else if ( slotType == "HtmlDialog" ) {
        showPluginHtmlDialog(moduleData);
    } else if ( slotType == "MainTabView") {
        showPluginMainTabView(moduleData);
    } else if ( slotType == "ExecuteScript" ) {
        executeModuleScript(moduleData);
    }
}

void JSPluginManager::handleDocumentChanged()
{
    for (auto data : m_pluginDataCollection) {
        data->emitDocumentChanged();
    }
}

void JSPluginManager::handlePluginEditorRequest(const WIZDOCUMENTDATA &doc, const QString &guid)
{
    WizDatabaseManager *dbMgr = WizDatabaseManager::instance();
    WizDatabase &db = dbMgr->db(doc.strKbGUID);
    QString strDocumentFileName = db.getDocumentFileName(doc.strGUID);
    
    QFileInfo docZiwFile(strDocumentFileName);
    if (!docZiwFile.exists()) {
        qWarning() << tr("Document data does not exist: ") + doc.strTitle;
        return;
    }

    QString strHtmlFile;
    if (!db.documentToTempHtmlFile(doc, strHtmlFile)) {
        qWarning() << tr("Can't unzip note data: ") + doc.strTitle;
        return;
    }
    
    QString htmlContent;
    if (!WizLoadUnicodeTextFromFile(strHtmlFile, htmlContent)) {
        qWarning() << tr("Can't read html file: ") + doc.strTitle;
        return;
    }
    
    // Insert JavaScript and CSS files
    JSPluginModule *module = moduleByGUID(guid);
    QStringList scriptFiles = module->spec()->scriptFiles();
    QStringList styleFiles = module->spec()->styleFiles();
    QString insertion;
    for (auto &cssFile : styleFiles) {
        insertion += QString(
            "<link rel='stylesheet' type='text/css' href='%1' name='wiz_inner_style' wiz_style='unsave' charset='utf-8'>")
            .arg(QUrl::fromLocalFile(cssFile).toString());
    }
    for (auto &jsFile : scriptFiles) {
        insertion += QString(
            "<script type='text/javascript' src='%1' name='wiz_inner_script' wiz_style='unsave' charset='utf-8'></script>")
            .arg(QUrl::fromLocalFile(jsFile).toString());
    }
    
    htmlContent = Utils::WizHtmlInsertText(htmlContent, insertion, "beforeend", "head");

    // Save to temp file
    if (!WizSaveUnicodeTextToUtf8File(strHtmlFile, htmlContent, true)) {
        qWarning() << tr("Can't write html file: ") + doc.strTitle;
        return;
    }

    QString indexFileUrl = QUrl::fromLocalFile(strHtmlFile).toString() + "?guid=" + doc.strGUID + "&kbguid=" + doc.strKbGUID;
    WizWebEngineInjectObjectCollection objects = {
        {"JSPlugin", module->parentPlugin()},
        {"JSPluginModule", module},
        {"WizExplorerApp", m_mainWindow->publicAPIsObject()}
    };
    // Ownership of page is passed on to the QTabWidget.
    WizWebEngineView *webView = new WizWebEngineView(objects, nullptr);
    WizWebsiteView *websiteView = new WizWebsiteView(webView, *m_mainWindow);
    websiteView->viewHtml(indexFileUrl);
    WizMainTabBrowser *tabBrowser = m_mainWindow->mainTabView();
    tabBrowser->createTab(websiteView);
}

void JSPluginManager::executeModuleScript(JSPluginModule *moduleData)
{
    QString scriptFile = moduleData->spec()->scriptFileName();
    QString scriptContent;
    if (!WizLoadUnicodeTextFromFile(scriptFile, scriptContent)) {
        qWarning() << tr("Can't read script file: ") + scriptFile;
        return;
    }

    QJSEngine jsEngine;
    jsEngine.installExtensions(QJSEngine::AllExtensions);
    jsEngine.globalObject().setProperty("global", jsEngine.globalObject());

    WizWebEngineInjectObjectCollection objects = {
        {"JSPlugin", moduleData->parentPlugin()},
        {"JSPluginModule", moduleData},
        {"WizExplorerApp", m_mainWindow->publicAPIsObject()}
    };

    auto i = objects.constBegin();
    while (i != objects.constEnd()) {
        QQmlEngine::setObjectOwnership(i.value(), QQmlEngine::CppOwnership);
        jsEngine.globalObject().setProperty(i.key(), jsEngine.newQObject(i.value()));
        ++i;
    }

    QJSValue result = jsEngine.evaluate(scriptContent, scriptFile);

    if (result.isError())
        QMessageBox::critical(
            nullptr, "QJSEngine Error",
            QString("Uncaught exception at line %1:\n%2")
              .arg(result.property("lineNumber").toInt())
              .arg(result.toString()));
}
