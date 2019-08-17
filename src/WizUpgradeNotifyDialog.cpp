#include "WizUpgradeNotifyDialog.h"
#include "ui_WizUpgradeNotifyDialog.h"

#include <QString>
#include <QWebEngineSettings>

#include "utils/WizPathResolve.h"
#include "share/WizWebEngineView.h"
#include "share/WizMisc.h"

#define MULTILINE(...) #__VA_ARGS__

WizUpgradeNotifyDialog::WizUpgradeNotifyDialog(const QString& changelogUrl, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WizUpgradeNotifyDialog)
{
    m_web = new WizWebEngineView(this);
    ui->setupUi(this);
    ui->webViewLayout->addWidget(m_web);

    if (!changelogUrl.isEmpty())
        m_web->load(QUrl(changelogUrl));

    connect(ui->buttonWait, SIGNAL(clicked()), SLOT(reject()));
    connect(ui->buttonNow, SIGNAL(clicked()), SLOT(accept()));
}

WizUpgradeNotifyDialog::~WizUpgradeNotifyDialog()
{
    delete ui;
}

QString WizUpgradeNotifyDialog::createMarkdownContentPage(const QString &markdownSource)
{
    QString htmlTemplate  = MULTILINE(
    <!doctype html>
    <html>
        <head>
            <meta charset="utf-8"/>
            <title>WizNotePlus Release</title>
        </head>
        <body>
            <div style="display: none;" id="content">%1</div>
            <div id="show"></div>
            <script src="%2"></script>
            <script>
                document.getElementById('show').innerHTML = marked(
                    document.getElementById('content').innerText
                );
            </script>
        </body>
    </html>
    );

    QString libFileUrl = Utils::WizPathResolve::resourcesPath() + "files/markdown/marked-0.7.0.min.js";

    return htmlTemplate.arg(markdownSource).arg(libFileUrl);
}

void WizUpgradeNotifyDialog::showMarkdownContent(const QString &markdownSource)
{
    const QString html = createMarkdownContentPage(markdownSource);
    const QString tempHtmlFile = Utils::WizPathResolve::tempPath() + "release_notes.html";
    WizSaveUnicodeTextToUtf8File(tempHtmlFile, html);
    m_web->load(tempHtmlFile);
}
