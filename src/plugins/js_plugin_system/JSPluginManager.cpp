#include "JSPluginManager.h"
#include "JSPluginSpec.h"
#include "JSPluginHtmlDialog.h"
#include "JSPluginSelectorWindow.h"

#include "sync/WizToken.h"
#include "WizMainWindow.h"
#include "share/WizGlobal.h"
#include "utils/WizPathResolve.h"
#include "widgets/WizLocalProgressWebView.h"

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
#include "share/WizSettings.h"
#include "share/WizWebEngineView.h"
#include "share/WizMisc.h"
#include "WizWebsiteView.h"
#include "WizMainTabBrowser.h"

JSPluginManager::JSPluginManager()
    : QObject(nullptr)
    , m_app(*WizMainWindow::instance())
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
        JSPluginSpec* data = new JSPluginSpec(folder, this);
        m_pluginDataCollection.push_back(data);
        qDebug() << "Loaded plugin: " + data->name();
    }
}

QList<JSPluginModuleSpec *> JSPluginManager::modulesByButtonLocation(QString buttonLocation) const
{
    QList<JSPluginModuleSpec *> ret;
    for (JSPluginSpec *pluginData : m_pluginDataCollection) {
        for (JSPluginModuleSpec *moduleData : pluginData->modules()) {
            if (moduleData->buttonLocation() == buttonLocation) {
                ret.push_back(moduleData);
            }
        }
    }
    return ret;
}

QList<JSPluginModuleSpec *> JSPluginManager::modulesByKeyValue(QString key, QString value) const
{
    QList<JSPluginModuleSpec *> ret;
    for (JSPluginSpec *pluginData : m_pluginDataCollection) {
        WizSettings *settings = pluginData->settings();
        for (JSPluginModuleSpec *moduleData : pluginData->modules()) {
            QString section = moduleData->section();
            if (settings->getString(section, key) == value) {
                ret.push_back(moduleData);
            }
        }
    }
    return ret;
}

JSPluginModuleSpec *JSPluginManager::moduleByGUID(QString guid) const
{
    JSPluginModuleSpec *ret = nullptr;
    for (JSPluginSpec *pluginData : m_pluginDataCollection) {
        for (JSPluginModuleSpec *moduleData : pluginData->modules()) {
            if (moduleData->guid() == guid) {
                ret = moduleData;
            }
        }
    }

    return ret;
}

QAction *JSPluginManager::createPluginAction(QWidget *parent, JSPluginModuleSpec *moduleData)
{
    QAction *ac = new QAction(parent);
    ac->setData(moduleData->guid());
    ac->setIcon(QIcon(moduleData->iconFileName()));
    ac->setIconText(moduleData->caption());
    ac->setText(moduleData->caption());
    ac->setToolTip(moduleData->caption());
    return ac;
}

JSPluginHtmlDialog *JSPluginManager::initPluginHtmlDialog(JSPluginModuleSpec *moduleData)
{
    JSPluginHtmlDialog *htmlDialog = new JSPluginHtmlDialog(m_app, moduleData, nullptr);
    m_pluginHtmlDialogCollection.insert(moduleData->guid(), htmlDialog);
    return htmlDialog;
}

JSPluginSelectorWindow *JSPluginManager::initPluginSelectorWindow(JSPluginModuleSpec *moduleData)
{
    JSPluginSelectorWindow *selectorWindow = new JSPluginSelectorWindow(m_app, moduleData, nullptr);
    m_pluginPopupDialogCollection.insert(moduleData->guid(), selectorWindow);
    return selectorWindow;
}

WizWebsiteView *JSPluginManager::initPluginMainTabView(JSPluginModuleSpec *moduleData)
{
    WizWebEngineInjectObjectCollection objects = {
        {"JSPluginSpec", moduleData->parentPlugin()},
        {"JSPluginModuleSpec", moduleData},
        {"WizExplorerApp", WizMainWindow::instance()->componentInterface()}
    };
    WizWebEngineView *webView = new WizWebEngineView(objects, nullptr);
    QPointer<WizWebsiteView> websiteView = new WizWebsiteView(webView, m_app);
    websiteView->viewHtml(QUrl::fromLocalFile(moduleData->htmlFileName()));
    m_pluginMainTabViewCollection.insert(moduleData->guid(), websiteView);
    return websiteView;
}

void JSPluginManager::showPluginHtmlDialog(JSPluginModuleSpec *moduleData)
{
    QString guid = moduleData->guid();
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

void JSPluginManager::showPluginSelectorWindow(JSPluginModuleSpec *moduleData, QPoint &pt)
{
    QString guid = moduleData->guid();
    JSPluginSelectorWindow* selectorWindow;
    auto it = m_pluginPopupDialogCollection.find(guid);
    if ( it == m_pluginPopupDialogCollection.end()) {
        selectorWindow = initPluginSelectorWindow(moduleData);
    } else {
        selectorWindow = it.value();
    }
    //
    moduleData->emitShowEvent();
    selectorWindow->showAtPoint(pt);
}

void JSPluginManager::showPluginMainTabView(JSPluginModuleSpec *moduleData)
{
    QString guid = moduleData->guid();
    QPointer<WizWebsiteView> mainTabView;
    WizMainTabBrowser *tabBrowser = WizMainWindow::instance()->mainTabView();
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

    JSPluginModuleSpec *moduleData = moduleByGUID(moduleGuid);
    if (!moduleData)
        return;

    QString slotType = moduleData->slotType();
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
    }
}
