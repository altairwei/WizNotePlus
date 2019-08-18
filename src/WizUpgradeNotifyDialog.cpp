#include "WizUpgradeNotifyDialog.h"
#include "ui_WizUpgradeNotifyDialog.h"

#include <QString>
#include <QWebEngineSettings>
#include <QUrl>

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
            <style>
                body {
                    padding: 4px;
                    font-family: -apple-system, "Noto Sans", "Helvetica Neue", Helvetica, "Nimbus Sans L", 
                        Arial, "Liberation Sans", "PingFang SC", "Hiragino Sans GB", "Noto Sans CJK SC", 
                        "Source Han Sans SC", "Source Han Sans CN", "Microsoft YaHei", "Wenquanyi Micro Hei", 
                        "WenQuanYi Zen Hei", "ST Heiti", SimHei, "WenQuanYi Zen Hei Sharp", sans-serif;
                }
            </style>
        </head>
        <body>
            <div style="display: none;" id="content">%1</div>
            <script src="%2"></script>
            <script>
                const content = document.getElementById('content');
                content.innerHTML = marked(content.innerText);
                content.style.display = "block";
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
    m_web->load(QUrl::fromLocalFile(tempHtmlFile));
}
