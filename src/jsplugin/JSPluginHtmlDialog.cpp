#include "JSPluginHtmlDialog.h"

#include <QVBoxLayout>

#include "JSPlugin.h"
#include "WizMainWindow.h"
#include "share/WizWebEngineView.h"


JSPluginHtmlDialog::JSPluginHtmlDialog(WizExplorerApp& app, JSPluginModule* module, QWidget* parent)
    : QWidget(parent, Qt::Window)
    , m_module(module)
    , m_dialogWidth(module->spec()->width())
    , m_dialogHeight(module->spec()->height())
{
    module->parentPlugin()->initStrings();
    //
    setWindowTitle(module->spec()->caption());
    //
    WizMainWindow* mw = qobject_cast<WizMainWindow*>(app.mainWindow());
    WizWebEngineInjectObjectCollection objects = {
        {"JSPlugin", module->parentPlugin()},
        {"JSPluginModule", module},
        {"WizExplorerApp", mw->publicAPIsObject()}
    };
    m_web = new WizWebEngineView(objects, this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    layout->addWidget(m_web);
    //
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