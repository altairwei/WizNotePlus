#include "WizAboutDialog.h"
#include "../WizDef.h"

#include <QApplication>
#include <QLabel>
#include <QFileInfo>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QDateTime>

#include <QTextStream>

WizAboutDialog::WizAboutDialog(QWidget *parent)
    : QDialog(parent)
{
    QLabel* labelIcon = new QLabel(this);
    labelIcon->setPixmap(qApp->windowIcon().pixmap(QSize(58, 58)));

#if defined Q_OS_MAC
    QString strProduct("<span style=\"font-weight:bold;font-size:14px\">WizNotePlus for Mac</span>");
#elif defined Q_OS_LINUX
    QString strProduct("<span style=\"font-weight:bold;font-size:14px\">WizNotePlus for Linux</span>");
#else
    QString strProduct("<span style=\"font-weight:bold;font-size:14px\">WizNotePlus for Windows</span>");
#endif

    QLabel* labelProduct = new QLabel(this);
    labelProduct->setText(strProduct);

    QString strPath = QApplication::applicationDirPath();
    QFileInfo fi(strPath);
    QDateTime t = fi.lastModified();
    QString strBuildNumber("<span style=\"font-size:11px\">Build Time %1.%2.%3 %4:%5</span>");
    strBuildNumber = strBuildNumber.\
            arg(t.date().year()).\
            arg(t.date().month()).\
            arg(t.date().day()).\
            arg(t.time().hour()).\
            arg(t.time().minute());
    QLabel* labelBuildTime = new QLabel(this);
    labelBuildTime->setText(strBuildNumber);

    QString strInfo = QString(tr("<span style=\"font-size:11px\">Version %2</span>")).arg(WIZ_CLIENT_VERSION);
    QLabel* labelBuild = new QLabel(this);
    labelBuild->setText(strInfo);

    QString devInfo = QString(
        tr("<span style=\"font-size:11px\">Development Stage %1.%2</span>").arg(WIZ_DEV_STAGE, WIZ_DEV_STAGE_VERSION));
    QLabel* labelDev = new QLabel(this);
    labelDev->setText(devInfo);

    QString qtInfo = QString(tr("<span style=\"font-size:11px\">Build against Qt %1</span>").arg(QT_VERSION_STR));
    QLabel* labelQt = new QLabel(this);
    labelQt->setText(qtInfo);

    QTextBrowser* textCredits = new QTextBrowser(this);
    textCredits->setOpenExternalLinks(true);
    textCredits->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QString strHtml;

    QFile file(":/credits.html");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        strHtml = stream.readAll();
        file.close();
    }

    //Utils::Misc::loadUnicodeTextFromFile(":/credits.html", strHtml);
    textCredits->setHtml(strHtml);

    QLabel* labelCopyright = new QLabel(this);
    labelCopyright->setText(tr("<span style=\"font-size:10px\">Copyright 2011-2017 Beijing Wozhi Technology Co., Ltd. All rights reserved.</span>"));

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QVBoxLayout* layout = new QVBoxLayout(this);

    mainLayout->addLayout(layout);
    layout->setContentsMargins(0, 10, 0, 10);

    layout->addWidget(labelIcon);
    layout->setAlignment(labelIcon, Qt::AlignCenter);

    layout->addWidget(labelProduct);
    layout->setAlignment(labelProduct, Qt::AlignCenter);

    layout->addWidget(labelBuild);
    layout->setAlignment(labelBuild, Qt::AlignCenter);

    layout->addWidget(labelDev);
    layout->setAlignment(labelDev, Qt::AlignCenter);

    layout->addWidget(labelQt);
    layout->setAlignment(labelQt, Qt::AlignCenter);

    layout->addWidget(labelBuildTime);
    layout->setAlignment(labelBuildTime, Qt::AlignCenter);

    layout->addSpacing(50);
    
    layout->addWidget(labelCopyright);
    layout->setAlignment(labelCopyright, Qt::AlignCenter);

    mainLayout->addWidget(textCredits);

    setWindowTitle(tr("About WizNote"));
}
