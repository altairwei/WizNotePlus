#include "WebEngineWindow.h"
#include "share/WizWebEngineView.h"
#include "share/WizMisc.h"

#include <QStyle>
#include <QApplication>
#include <QDesktopWidget>
#include <QVBoxLayout>
#include <QWebEngineView>

WebEngineWindow::WebEngineWindow(QWidget *parent)
    : QWidget(parent, Qt::Window)
    , m_web(new WizWebEngineView(this))
{
    setContentsMargins(0, 0, 0, 0);
    QPalette pal = palette();
    pal.setBrush(backgroundRole(), QBrush("#FFFFFF"));
    setPalette(pal);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_web);
    setLayout(layout);

    setWindowTitle(tr("Untitled"));
    connect(m_web, &WizWebEngineView::titleChanged, [this](const QString &title) {
        this->setWindowTitle(title);
    });

    // align on center of the screen
    setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            QSize(800, 500),
            qApp->desktop()->availableGeometry()
        )
    );
}

WizWebEngineView* WebEngineWindow::webView() {
    return m_web;
}

QSize WebEngineWindow::sizeHint() const {
    return QSize(800, 500);
}
