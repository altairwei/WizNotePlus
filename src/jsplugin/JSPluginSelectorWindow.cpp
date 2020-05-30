#include "JSPluginSelectorWindow.h"

#include <QVBoxLayout>

#include "JSPluginSpec.h"
#include "JSPlugin.h"
#include "WizMainWindow.h"
#include "share/WizWebEngineView.h"


JSPluginSelectorWindow::JSPluginSelectorWindow(WizExplorerApp& app, JSPluginModule* module, QWidget* parent)
    : WizPopupWidget(parent)
    , m_module(module)
    , m_windowWidth(module->spec()->width())
    , m_windowHeight(module->spec()->height())
{
    module->parentPlugin()->initStrings();
    //
    WizMainWindow *mw = WizMainWindow::instance();
    WizWebEngineInjectObjectCollection objects = {
        {"JSPlugin", module->parentPlugin()},
        {"JSPluginModule", module},
        {"WizExplorerApp", mw->publicAPIsObject()}
    };
    m_web = new WizWebEngineView(objects, this);
    //
    setContentsMargins(0, 8, 0, 0);
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