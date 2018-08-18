#include "WizDevToolsDialog.h"

#include "sync/WizToken.h"
#include "WizMainWindow.h"
#include "share/WizGlobal.h"
#include "utils/WizPathResolve.h"
#include "widgets/WizLocalProgressWebView.h"

#include <QMovie>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <QNetworkReply>
#include <QWebEngineView>
#include <QRect>
#include "share/WizWebEngineView.h"
#include "share/WizMisc.h"


WizDevToolsDialog::WizDevToolsDialog(QWidget *parent)
    : WizWebEngineViewContainerDialog(parent)
    , m_web(new WizWebEngineView(this))
{
    // 设置长宽和边界
    setContentsMargins(0, 0, 0, 0);

    QPalette pal = palette();
    pal.setBrush(backgroundRole(), QBrush("#FFFFFF"));
    setPalette(pal);
    //

    // 将页面嵌入布局
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_web);
    setLayout(layout);
}

WizWebEngineView* WizDevToolsDialog::getWeb() {
    return m_web;
}

QSize WizDevToolsDialog::sizeHint() const {
    return QSize(WizSmartScaleUI(800), WizSmartScaleUI(500));
}
