#include "JSPluginSelectorWindow.h"
#include "JSPluginSpec.h"

#include "WizMainWindow.h"
#include "share/WizWebEngineView.h"

JSPluginSelectorWindow::JSPluginSelectorWindow(WizExplorerApp& app, JSPluginModuleSpec* data, QWidget* parent)
    : WizPopupWidget(parent)
    , m_data(data)
    , m_windowWidth(data->width())
    , m_windowHeight(data->height())
{
    data->parentPlugin()->initStrings();
    //
    WizMainWindow *mw = WizMainWindow::instance();
    WizWebEngineInjectObjectCollection objects = {
        {"JSPluginSpec", data->parentPlugin()},
        {"JSPluginModuleSpec", data},
        {"WizExplorerApp", mw->publicAPIsObject()}
    };
    m_web = new WizWebEngineView(objects, this);
    //
    setContentsMargins(0, 8, 0, 0);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    layout->addWidget(m_web);
    m_web->load(QUrl::fromLocalFile(m_data->htmlFileName()));

}

QSize JSPluginSelectorWindow::sizeHint() const
{
    return QSize(m_windowWidth, m_windowHeight);
}