﻿#include "WizPreferenceDialog.h"
#include "ui_WizPreferenceDialog.h"

#include <QMessageBox>
#include <QFontDialog>
#include <QColorDialog>
#include <QTimer>

#include "share/WizGlobal.h"
#include "utils/WizPathResolve.h"
#include "share/WizMessageBox.h"
#include "database/WizDatabaseManager.h"
#include "share/WizThreads.h"
#include "share/WizRequest.h"

#include "WizMainWindow.h"
#include "WizProxyDialog.h"
#include "sync/WizToken.h"
#include "sync/WizApiEntry.h"

#include "widgets/WizExecutingActionDialog.h"


WizPreferenceWindow::WizPreferenceWindow(WizExplorerApp& app, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::WizPreferenceWindow)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_biniting(true)
{
    ui->setupUi(this);
    setWindowIcon(QIcon());
    setWindowTitle(tr("Preference"));

    connect(ui->btnClose, SIGNAL(clicked()), SLOT(accept()));

    // general tab
    ::WizGetTranslatedLocales(m_locales);
    ui->comboLang->blockSignals(true);
    for (int i = 0; i < m_locales.count(); i++) {
        ui->comboLang->addItem(::WizGetTranslatedLocaleDisplayName(i));
    }

    for (int i = 0; i < ui->comboLang->count(); i++) {
        if (m_locales[i] == userSettings().locale()) {
            ui->comboLang->setCurrentIndex(i);
        }
    }
    ui->comboLang->blockSignals(false);

    ui->lineEditUIFont->setText(
        m_app.userSettings().UIFontFamily() + " " +
        QString::number(m_app.userSettings().UIFontSize())
    );

    ui->checkBox->blockSignals(true);
    Qt::CheckState checkState = userSettings().autoCheckUpdate() ? Qt::Checked : Qt::Unchecked;
    ui->checkBox->setCheckState(checkState);
    ui->checkBox->blockSignals(false);

    ui->checkBoxTrayIcon->blockSignals(true);
    checkState = userSettings().showSystemTrayIcon() ? Qt::Checked : Qt::Unchecked;
    ui->checkBoxTrayIcon->setCheckState(checkState);
    ui->checkBoxTrayIcon->blockSignals(false);

#ifdef BUILD4APPSTORE
    // hide language choice and upgrade for appstore
    ui->comboLang->setEnabled(false);
    ui->checkBox->setVisible(false);
#endif

    // reading tab
    switch (userSettings().noteViewMode())
    {
        case viewmodeAlwaysEditing:
            ui->radioAlwaysEditing->setChecked(true);
            break;
        case viewmodeAlwaysReading:
            ui->radioAlwaysReading->setChecked(true);
            break;
        default:
            ui->radioAuto->setChecked(true);
            break;
    }

    ui->spellCheck->setChecked(userSettings().isEnableSpellCheck());
    connect(ui->spellCheck, SIGNAL(toggled(bool)), this, SLOT(on_enableSpellCheck(bool)));

    ui->openLinkWithDesktopBrowser->setChecked(userSettings().isEnableOpenLinkWithDesktopBrowser());
    connect(ui->openLinkWithDesktopBrowser, SIGNAL(toggled(bool)), this, SLOT(on_enableOpenLinkWithDesktopBrowser(bool)));
    if (qEnvironmentVariableIsSet("QTWEBENGINE_DISABLE_SANDBOX"))
        ui->openLinkWithDesktopBrowser->setDisabled(true);

    // syncing tab
    int nInterval = userSettings().syncInterval();
    switch (nInterval) {
        case 5:
            ui->comboSyncInterval->setCurrentIndex(0);
            break;
        case 15:
            ui->comboSyncInterval->setCurrentIndex(1);
            break;
        case 30:
            ui->comboSyncInterval->setCurrentIndex(2);
            break;
        case 60:
            ui->comboSyncInterval->setCurrentIndex(3);
            break;
        case -1:
            ui->comboSyncInterval->setCurrentIndex(4);
            break;
        default:
            ui->comboSyncInterval->setCurrentIndex(1);
    }

    switch (m_dbMgr.db().getObjectSyncTimeline()) {
        case -1:
            ui->comboSyncMethod->setCurrentIndex(0);
            break;
        case 1:
            ui->comboSyncMethod->setCurrentIndex(1);
            break;
        case 7:
            ui->comboSyncMethod->setCurrentIndex(2);
            break;
        case 30:
            ui->comboSyncMethod->setCurrentIndex(3);
            break;
        case 99999:
            ui->comboSyncMethod->setCurrentIndex(4);
            break;
        default:
            ui->comboSyncMethod->setCurrentIndex(4);
    }

    int nDays = 1;
    if (m_dbMgr.count()) {
        nDays = m_dbMgr.at(0).getObjectSyncTimeline();
    }

    switch (nDays) {
        case -1:
            ui->comboSyncGroupMethod->setCurrentIndex(0);
            break;
        case 1:
            ui->comboSyncGroupMethod->setCurrentIndex(1);
            break;
        case 7:
            ui->comboSyncGroupMethod->setCurrentIndex(2);
            break;
        case 30:
            ui->comboSyncGroupMethod->setCurrentIndex(3);
            break;
        case 99999:
            ui->comboSyncGroupMethod->setCurrentIndex(4);
            break;
        default:
            ui->comboSyncGroupMethod->setCurrentIndex(1);
    }

    bool downloadAttachments = m_dbMgr.db().getDownloadAttachmentsAtSync();
    ui->comboDownloadAttachments->setCurrentIndex(downloadAttachments ? 1 : 0);

    connect(ui->comboSyncInterval, SIGNAL(activated(int)), SLOT(on_comboSyncInterval_activated(int)));
    connect(ui->comboSyncMethod, SIGNAL(activated(int)), SLOT(on_comboSyncMethod_activated(int)));
    connect(ui->comboSyncGroupMethod, SIGNAL(activated(int)), SLOT(on_comboSyncGroupMethod_activated(int)));
    connect(ui->comboDownloadAttachments, SIGNAL(activated(int)), SLOT(on_comboDownloadAttachments_activated(int)));

    QString proxySettings = WizFormatString1("<a href=\"proxy_settings\" style=\"color:#3CA2E0;\">%1</a>", tr("Proxy settings"));
    ui->labelProxySettings->setText(proxySettings);
    connect(ui->labelProxySettings, SIGNAL(linkActivated(const QString&)),
            SLOT(labelProxy_linkActivated(const QString&)));

    // format tab
    QString strFont = QString("%1  %2").
            arg(m_app.userSettings().defaultFontFamily())
            .arg(m_app.userSettings().defaultFontSize());
    ui->editFont->setText(strFont);

    ui->comboBox_unit->setCurrentIndex(m_app.userSettings().printMarginUnit());
    ui->spinBox_bottom->setValue(m_app.userSettings().printMarginValue(wizPositionBottom));
    ui->spinBox_left->setValue(m_app.userSettings().printMarginValue(wizPositionLeft));
    ui->spinBox_right->setValue(m_app.userSettings().printMarginValue(wizPositionRight));
    ui->spinBox_top->setValue(m_app.userSettings().printMarginValue(wizPositionTop));

    ui->comboLineHeight->addItem("1");
    ui->comboLineHeight->addItem("1.2");
    ui->comboLineHeight->addItem("1.5");
    ui->comboLineHeight->addItem("1.7");
    ui->comboLineHeight->addItem("2.0");
    ui->comboLineHeight->setCurrentText(m_app.userSettings().editorLineHeight());

    ui->comboParaSpacing->addItem("5");
    ui->comboParaSpacing->addItem("8");
    ui->comboParaSpacing->addItem("12");
    ui->comboParaSpacing->addItem("15");
    ui->comboParaSpacing->setCurrentText(m_app.userSettings().editorParaSpacing());

    ui->spinPagePadding->setValue(m_app.userSettings().editorPagePadding().toInt());

    ui->tabWidget->setCurrentIndex(0);

    QString strColor = m_app.userSettings().editorBackgroundColor();
    updateEditorBackgroundColor(strColor);

    bool manuallySortFolders = m_app.userSettings().isManualSortingEnabled();
    ui->checkBoxManuallySort->setChecked(manuallySortFolders);

    m_biniting = false;
}


void WizPreferenceWindow::showEvent(QShowEvent*)
{
    QSize size = ui->pushButtonClearBackground->size();
    ui->btnResetLineHeight->setFixedSize(size);
}


void WizPreferenceWindow::showPrintMarginPage()
{
    ui->tabWidget->setCurrentWidget(ui->tabPrint);
}

void WizPreferenceWindow::on_radioAuto_clicked(bool chcked)
{
    if (!chcked)
        return;

    userSettings().setNoteViewMode(viewmodeKeep);
    Q_EMIT settingsChanged(wizoptionsNoteView);
}

void WizPreferenceWindow::on_radioAlwaysReading_clicked(bool chcked)
{
    if (!chcked)
        return;

    userSettings().setNoteViewMode(viewmodeAlwaysReading);
    Q_EMIT settingsChanged(wizoptionsNoteView);
}

void WizPreferenceWindow::on_radioAlwaysEditing_clicked(bool chcked)
{
    if (!chcked)
        return;

    userSettings().setNoteViewMode(viewmodeAlwaysEditing);
    Q_EMIT settingsChanged(wizoptionsNoteView);
}

void WizPreferenceWindow::on_comboSyncInterval_activated(int index)
{
    switch (index) {
        case 0:
            userSettings().setSyncInterval(5);
            break;
        case 1:
            userSettings().setSyncInterval(15);
            break;
        case 2:
            userSettings().setSyncInterval(30);
            break;
        case 3:
            userSettings().setSyncInterval(60);
            break;
        case 4:
            userSettings().setSyncInterval(-1);
            break;
        default:
            Q_ASSERT(0);
    }

    Q_EMIT settingsChanged(wizoptionsSync);
}

void WizPreferenceWindow::on_comboSyncMethod_activated(int index)
{
    switch (index) {
        case 0:
            m_dbMgr.db().setObjectSyncTimeLine(-1);
            break;
        case 1:
            m_dbMgr.db().setObjectSyncTimeLine(1);
            break;
        case 2:
            m_dbMgr.db().setObjectSyncTimeLine(7);
            break;
        case 3:
            m_dbMgr.db().setObjectSyncTimeLine(30);
            break;
        case 4:
            m_dbMgr.db().setObjectSyncTimeLine(99999);
            break;
        default:
            Q_ASSERT(0);
    }

    Q_EMIT settingsChanged(wizoptionsSync);
}

void WizPreferenceWindow::on_comboSyncGroupMethod_activated(int index)
{
    switch (index) {
    case 0:
        setSyncGroupTimeLine(-1);
        break;
    case 1:
        setSyncGroupTimeLine(1);
        break;
    case 2:
        setSyncGroupTimeLine(7);
        break;
    case 3:
        setSyncGroupTimeLine(30);
        break;
    case 4:
        setSyncGroupTimeLine(99999);
        break;
    default:
        Q_ASSERT(0);
    }

    Q_EMIT settingsChanged(wizoptionsSync);
}

void WizPreferenceWindow::setSyncGroupTimeLine(int nDays)
{
    for (int i = 0; i < m_dbMgr.count(); i++) {
        m_dbMgr.at(i).setObjectSyncTimeLine(nDays);
    }
}

void WizPreferenceWindow::labelProxy_linkActivated(const QString& link)
{
    Q_UNUSED(link);

    WizProxyDialog dlg(this);
    if (QDialog::Accepted != dlg.exec()) {
        Q_EMIT settingsChanged(wizoptionsSync);
    }
}

void WizPreferenceWindow::on_buttonFontSelect_clicked()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(
        &ok,
        QFont(
            m_app.userSettings().defaultFontFamily(),
            m_app.userSettings().defaultFontSize()
        ),
        this,
        tr("Select font"),
        QFontDialog::DontUseNativeDialog
    );

    if (ok)
        updateEditorFont(font);
}

void WizPreferenceWindow::on_btnResetEditorFont_clicked()
{
    QString default_font_family = m_app.userSettings().defaultFontFamily(true);
    int default_font_size = m_app.userSettings().defaultFontSize(true);
    QFont default_font(default_font_family, default_font_size);

    updateEditorFont(default_font);
}

void WizPreferenceWindow::updateEditorFont(const QFont& font)
{
    QString str = font.family() + " " + QString::number(font.pointSize());
    ui->editFont->setText(str);

    m_app.userSettings().setDefaultFontFamily(font.family());
    m_app.userSettings().setDefaultFontSize(font.pointSize());

    Q_EMIT settingsChanged(wizoptionsFont);
}

void WizPreferenceWindow::on_comboLang_currentIndexChanged(int index)
{
    QString strLocaleName = m_locales[index];
    if (strLocaleName.compare(userSettings().locale())) {
        userSettings().setLocale(strLocaleName);

        WizMessageBox::information(this, tr("Info"), tr("Language will be changed after restart WizNote."));
    }
}

void WizPreferenceWindow::on_btnUIFont_clicked()
{
    bool ok = false;
    QFont font = QFontDialog::getFont(
        &ok,
        QFont(
            m_app.userSettings().UIFontFamily(),
            m_app.userSettings().UIFontSize()
        ),
        this,
        tr("Select font"),
        QFontDialog::DontUseNativeDialog
    );

    if (ok)
        updateUIFont(font);
}

void WizPreferenceWindow::on_btnResetUIFont_clicked()
{
    QFont font(
        m_app.userSettings().UIFontFamily(true),
        m_app.userSettings().UIFontSize(true)
    );

    updateUIFont(font);
}

void WizPreferenceWindow::updateUIFont(const QFont& font)
{
    ui->lineEditUIFont->setText(
        font.family() + " " + QString::number(font.pointSize())
    );

    m_app.userSettings().setUIFontFamily(font.family());
    m_app.userSettings().setUIFontSize(font.pointSize());

    WizMessageBox::information(
        m_app.mainWindow(), tr("Info"), tr("Application font will be changed after restart WizNote."));
}

void WizPreferenceWindow::on_checkBox_stateChanged(int arg1)
{
    bool autoUpdate = (arg1 == Qt::Checked);
    m_app.userSettings().setAutoCheckUpdate(autoUpdate);

    if (autoUpdate) {
        WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
        mainWindow->checkWizUpdate();
    }
}

void WizPreferenceWindow::on_checkBoxTrayIcon_toggled(bool checked)
{
    m_app.userSettings().setShowSystemTrayIcon(checked);
    WizMainWindow* mainWindow = qobject_cast<WizMainWindow*>(m_app.mainWindow());
    mainWindow->setSystemTrayIconVisible(checked);
}

void WizPreferenceWindow::on_comboBox_unit_currentIndexChanged(int index)
{
    m_app.userSettings().setPrintMarginUnit(index);
}

void WizPreferenceWindow::on_spinBox_top_valueChanged(double arg1)
{
    m_app.userSettings().setPrintMarginValue(wizPositionTop, arg1);
}

void WizPreferenceWindow::on_spinBox_bottom_valueChanged(double arg1)
{
    m_app.userSettings().setPrintMarginValue(wizPositionBottom, arg1);
}

void WizPreferenceWindow::on_spinBox_left_valueChanged(double arg1)
{
    m_app.userSettings().setPrintMarginValue(wizPositionLeft, arg1);
}

void WizPreferenceWindow::on_spinBox_right_valueChanged(double arg1)
{
    m_app.userSettings().setPrintMarginValue(wizPositionRight, arg1);
}

void WizPreferenceWindow::on_pushButtonBackgroundColor_clicked()
{
    QColorDialog dlg;
    QString color = m_app.userSettings().editorBackgroundColor();
    if (!color.isEmpty()) {
        dlg.setCurrentColor(color);
    }
    if (dlg.exec() == QDialog::Accepted)
    {
        QString strColor = dlg.currentColor().name();
        updateEditorBackgroundColor(strColor);
    }
}

void WizPreferenceWindow::on_pushButtonClearBackground_clicked()
{
    updateEditorBackgroundColor("");
}

void WizPreferenceWindow::updateEditorBackgroundColor(const QString& strColorName)
{
    m_app.userSettings().setEditorBackgroundColor(strColorName);
    ui->pushButtonBackgroundColor->setStyleSheet(QString("QPushButton "
                                                             "{ border: 1px; background: %1; height:20px;} ").arg(strColorName));
    ui->pushButtonBackgroundColor->setText(strColorName.isEmpty() ? tr("Click to select color") : QString());
    ui->pushButtonClearBackground->setVisible(!strColorName.isEmpty());

    Q_EMIT settingsChanged(wizoptionsFont);
}

void WizPreferenceWindow::on_checkBoxManuallySort_toggled(bool checked)
{
    m_app.userSettings().setManualSortingEnable(checked);
    emit settingsChanged(wizoptionsFolders);
}

void WizPreferenceWindow::on_comboDownloadAttachments_activated(int index)
{
    switch (index) {
    case 0:
        m_dbMgr.db().setDownloadAttachmentsAtSync(false);
        break;
    case 1:
        m_dbMgr.db().setDownloadAttachmentsAtSync(true);
        break;
    default:
        Q_ASSERT(0);
    }

    Q_EMIT settingsChanged(wizoptionsSync);
}

void WizPreferenceWindow::on_tabWidget_currentChanged(int index)
{
//    if (index == 1)
//    {
//        setFixedHeight(350);
//        resize(width(), 350);
//    }
//    else
//    {
//        setFixedHeight(290);
//        resize(width(), 290);
//    }
}


void WizPreferenceWindow::on_enableSpellCheck(bool checked)
{
    userSettings().setEnableSpellCheck(checked);
    Q_EMIT settingsChanged(wizoptionsSpellCheck);
}

void WizPreferenceWindow::on_enableOpenLinkWithDesktopBrowser(bool checked)
{
    userSettings().setEnableOpenLinkWithDesktopBrowser(checked);
    Q_EMIT settingsChanged(wizoptionsOpenLinkWithDesktopBrowser);
}

// Line Height

void WizPreferenceWindow::updateEditorLineHeight(const QString& strLineHeight, bool save)
{
    if (save) {
        m_app.userSettings().setEditorLineHeight(strLineHeight);
    }

    Q_EMIT settingsChanged(wizoptionsSpacing);
}

void WizPreferenceWindow::on_comboLineHeight_currentIndexChanged(int index)
{
    if (m_biniting)
        return;

    QString LineHeight = ui->comboLineHeight->itemText(index);
    updateEditorLineHeight(LineHeight, true);
}

void WizPreferenceWindow::on_btnResetLineHeight_clicked()
{
    QString default_val = m_app.userSettings().editorLineHeight(true);
    ui->comboLineHeight->setCurrentText(default_val);
    updateEditorLineHeight(default_val, true);
}


// Para Spacing

void WizPreferenceWindow::updateEditorParaSpacing(const QString& spacing, bool save)
{
    if (save) {
        m_app.userSettings().setEditorParaSpacing(spacing);
    }

    Q_EMIT settingsChanged(wizoptionsSpacing);
}

void WizPreferenceWindow::on_comboParaSpacing_currentIndexChanged(int index)
{
    if (m_biniting)
        return;

    QString ParaSpacing = ui->comboParaSpacing->itemText(index);
    updateEditorParaSpacing(ParaSpacing, true);
}

void WizPreferenceWindow::on_btnResetParaSpacing_clicked()
{
    QString default_val = m_app.userSettings().editorParaSpacing(true);
    ui->comboParaSpacing->setCurrentText(default_val);
    updateEditorParaSpacing(default_val, true);
}


// Page Padding

void WizPreferenceWindow::updateEditorPagePadding(const QString& padding, bool save)
{
    if (save) {
        m_app.userSettings().setEditorPagePadding(padding);
    }

    Q_EMIT settingsChanged(wizoptionsSpacing);
}

void WizPreferenceWindow::on_spinPagePadding_valueChanged(int val)
{
    if (m_biniting)
        return;

    QString PagePadding = QString::number(ui->spinPagePadding->value());
    updateEditorPagePadding(PagePadding, true);
}

void WizPreferenceWindow::on_btnResetPagePadding_clicked()
{
    QString default_val = m_app.userSettings().editorPagePadding(true);
    ui->spinPagePadding->setValue(default_val.toInt());
    updateEditorPagePadding(default_val, true);
}
