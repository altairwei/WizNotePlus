#include "JSPluginWidgets.h"

#include <QVBoxLayout>

#include "JSPlugin.h"
#include "WizMainWindow.h"
#include "share/WizWebEngineView.h"

JSPluginWebsiteView::JSPluginWebsiteView(WizExplorerApp& app, JSPluginModule* module, QWidget* parent)
    : QWidget(parent)
    , m_module(module)
{
    WizMainWindow* mw = qobject_cast<WizMainWindow*>(app.mainWindow());
    WizWebEngineInjectObjectCollection objects = {
        {"JSPlugin", module->parentPlugin()},
        {"JSPluginModule", module},
        {"WizExplorerApp", mw->publicAPIsObject()},
        {"JSPluginWindow", this}
    };
    m_web = new WizWebEngineView(objects, this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    layout->addWidget(m_web);
}

/////////////////////////////////////////////////////////////
/// JSPluginHtmlDialog
/////////////////////////////////////////////////////////////

JSPluginHtmlDialog::JSPluginHtmlDialog(WizExplorerApp& app, JSPluginModule* module, QWidget* parent)
    : JSPluginWebsiteView(app, module, parent)
    , m_dialogWidth(module->spec()->width())
    , m_dialogHeight(module->spec()->height())
{
    setWindowFlag(Qt::Window, true);
    module->parentPlugin()->initStrings();
    setWindowTitle(module->spec()->caption());
    m_web->load(QUrl::fromLocalFile(m_module->spec()->htmlFileName()));
}

QSize JSPluginHtmlDialog::dialogSize() const
{
    return QSize(m_dialogWidth, m_dialogHeight);
}

QSize JSPluginHtmlDialog::sizeHint() const
{
    return dialogSize();
}

/////////////////////////////////////////////////////////////
/// JSPluginSelectorWindow
/////////////////////////////////////////////////////////////

JSPluginSelectorWindow::JSPluginSelectorWindow(WizExplorerApp& app, JSPluginModule* module, QWidget* parent)
    : WizPopupWidget(parent)
    , m_module(module)
    , m_windowWidth(module->spec()->width())
    , m_windowHeight(module->spec()->height())
{
    module->parentPlugin()->initStrings();

    WizMainWindow *mw = WizMainWindow::instance();
    WizWebEngineInjectObjectCollection objects = {
        {"JSPlugin", module->parentPlugin()},
        {"JSPluginModule", module},
        {"WizExplorerApp", mw->publicAPIsObject()},
        {"JSPluginWindow", this}
    };
    m_web = new WizWebEngineView(objects, this);

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    layout->addWidget(m_web);
    m_web->load(QUrl::fromLocalFile(m_module->spec()->htmlFileName()));

}

QSize JSPluginSelectorWindow::sizeHint() const
{
    return QSize(m_windowWidth, m_windowHeight);
}

/////////////////////////////////////////////////////////////
/// JSPluginDocSidebar
/////////////////////////////////////////////////////////////

JSPluginDocSidebar::JSPluginDocSidebar(WizExplorerApp& app, JSPluginModule* module, QWidget *parent)
    : JSPluginWebsiteView(app, module, parent)
    , m_sidebarWidth(module->spec()->width())
{
    module->parentPlugin()->initStrings();
    m_web->load(QUrl::fromLocalFile(m_module->spec()->htmlFileName()));
}

QSize JSPluginDocSidebar::sizeHint() const
{
    return QSize(m_sidebarWidth, 0);
}

