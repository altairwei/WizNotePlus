#include "JSPluginHtmlDialog.h"
#include "JSPluginSpec.h"

#include "WizMainWindow.h"
#include "share/WizWebEngineView.h"

JSPluginHtmlDialog::JSPluginHtmlDialog(WizExplorerApp& app, JSPluginModuleSpec* data, QWidget* parent)
    : QWidget(parent, Qt::Window)
    , m_data(data)
    , m_dialogWidth(data->width())
    , m_dialogHeight(data->height())
{
    data->parentPlugin()->initStrings();
    //
    setWindowTitle(data->caption());
    //
    WizMainWindow* mw = qobject_cast<WizMainWindow*>(app.mainWindow());
    WizWebEngineInjectObjectCollection objects = {
        {"JSPluginSpec", data->parentPlugin()},
        {"JSPluginModuleSpec", data},
        {"WizExplorerApp", mw->publicAPIsObject()}
    };
    m_web = new WizWebEngineView(objects, this);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
    layout->addWidget(m_web);
    //
    m_web->load(QUrl::fromLocalFile(m_data->htmlFileName()));
}

QSize JSPluginHtmlDialog::dialogSize() const
{
    return QSize(m_dialogWidth, m_dialogHeight);
}

QSize JSPluginHtmlDialog::sizeHint() const
{
    return dialogSize();
}