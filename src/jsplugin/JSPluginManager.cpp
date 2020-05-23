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

#include "sync/WizToken.h"
#include "WizMainWindow.h"
#include "share/WizGlobal.h"
#include "utils/WizPathResolve.h"
#include "widgets/WizLocalProgressWebView.h"

#include "share/WizSettings.h"
#include "share/WizWebEngineView.h"
#include "share/WizMisc.h"
#include "WizWebsiteView.h"
#include "WizMainTabBrowser.h"
#include "JSPlugin.h"
#include "JSPluginSpec.h"
#include "JSPluginHtmlDialog.h"
#include "JSPluginSelectorWindow.h"

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
        JSPlugin* data = new JSPlugin(folder, this);
        if (data->isAvailable()) {
            m_pluginDataCollection.push_back(data);
            qDebug() << "Loaded plugin: " + data->spec()->name();
        } else {
            qWarning() << "Failed to load plugin: " + data->spec()->name();
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
    ac->setData(moduleData->spec()->guid());
    ac->setIcon(QIcon(moduleData->spec()->iconFileName()));
    ac->setIconText(moduleData->spec()->caption());
    ac->setText(moduleData->spec()->caption());
    ac->setToolTip(moduleData->spec()->caption());
    return ac;
}

JSPluginHtmlDialog *JSPluginManager::initPluginHtmlDialog(JSPluginModule *moduleData)
{
    JSPluginHtmlDialog *htmlDialog = new JSPluginHtmlDialog(m_app, moduleData, nullptr);
    m_pluginHtmlDialogCollection.insert(moduleData->spec()->guid(), htmlDialog);
    return htmlDialog;
}

JSPluginSelectorWindow *JSPluginManager::initPluginSelectorWindow(JSPluginModule *moduleData)
{
    JSPluginSelectorWindow *selectorWindow = new JSPluginSelectorWindow(m_app, moduleData, nullptr);
    m_pluginPopupDialogCollection.insert(moduleData->spec()->guid(), selectorWindow);
    return selectorWindow;
}

WizWebsiteView *JSPluginManager::initPluginMainTabView(JSPluginModule *moduleData)
{
    WizWebEngineInjectObjectCollection objects = {
        {"JSPlugin", moduleData->parentPlugin()},
        {"JSPluginModule", moduleData},
        {"WizExplorerApp", WizMainWindow::instance()->publicAPIsObject()}
    };
    WizWebEngineView *webView = new WizWebEngineView(objects, nullptr);
    QPointer<WizWebsiteView> websiteView = new WizWebsiteView(webView, m_app);
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

void JSPluginManager::showPluginSelectorWindow(JSPluginModule *moduleData, QPoint &pt)
{
    QString guid = moduleData->spec()->guid();
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

void JSPluginManager::showPluginMainTabView(JSPluginModule *moduleData)
{
    QString guid = moduleData->spec()->guid();
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
    }
}

void JSPluginManager::notifyDocumentChanged()
{
    for (auto data : m_pluginDataCollection) {
        data->emitDocumentChanged();
    }
}