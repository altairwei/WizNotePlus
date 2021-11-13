#ifndef WIZPREFERENCEDIALOG_H
#define WIZPREFERENCEDIALOG_H

#include <QDialog>
#include <QPointer>
#include <QFontDialog>

#include "WizDef.h"
#include "share/WizSettings.h"
#include "share/WizMisc.h"
//#include "WizProxyDialog.h"

namespace Ui {
    class WizPreferenceWindow;
}

class WizDatabaseManager;

class WizPreferenceWindow: public QDialog
{
    Q_OBJECT

public:
    WizPreferenceWindow(WizExplorerApp& app, QWidget* parent);
    WizUserSettings& userSettings() const { return m_app.userSettings(); }

    void showPrintMarginPage();

Q_SIGNALS:
    void settingsChanged(WizOptionsType type);
    void restartForSettings();

public Q_SLOTS:
    void on_comboLang_currentIndexChanged(int index);

    void on_btnUIFont_clicked();
    void on_btnResetUIFont_clicked();
    void updateUIFont(const QFont& font);

    void on_radioAuto_clicked(bool checked);
    void on_radioAlwaysReading_clicked(bool checked);
    void on_radioAlwaysEditing_clicked(bool checked);

    void on_comboSyncInterval_activated(int index);
    void on_comboSyncMethod_activated(int index);
    void on_comboSyncGroupMethod_activated(int index);
    void on_comboDownloadAttachments_activated(int index);

    void labelProxy_linkActivated(const QString& link);

    void on_buttonFontSelect_clicked();
    void on_btnResetEditorFont_clicked();
    void updateEditorFont(const QFont& font);

    void on_enableSpellCheck(bool checked);
    void on_enableOpenLinkWithDesktopBrowser(bool checked);

private slots:
    void on_checkBox_stateChanged(int arg1);
    void on_checkBoxTrayIcon_toggled(bool checked);
    void on_comboBox_unit_currentIndexChanged(int index);
    void on_spinBox_top_valueChanged(double arg1);
    void on_spinBox_bottom_valueChanged(double arg1);
    void on_spinBox_left_valueChanged(double arg1);
    void on_spinBox_right_valueChanged(double arg1);
    void on_pushButtonBackgroundColor_clicked();
    void on_pushButtonClearBackground_clicked();

    void on_checkBoxManuallySort_toggled(bool checked);

    void on_tabWidget_currentChanged(int index);

    void on_comboLineHeight_currentIndexChanged(int index);
    void on_btnResetLineHeight_clicked();

    void on_comboParaSpacing_currentIndexChanged(int index);
    void on_btnResetParaSpacing_clicked();

    void on_spinPagePadding_valueChanged(int val);
    void on_btnResetPagePadding_clicked();

protected:
    void showEvent(QShowEvent* event) override;

private:
    Ui::WizPreferenceWindow *ui;
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;

    QStringList m_locales;
    QStringList m_skins;
    bool m_biniting;

    void setSyncGroupTimeLine(int nDays);
    void updateEditorBackgroundColor(const QString& strColorName);
    void updateEditorLineHeight(const QString& strLineHeight, bool save);
    void updateEditorParaSpacing(const QString& spacing, bool save);
    void updateEditorPagePadding(const QString& padding, bool save);
};


#endif // WIZPREFERENCEWINDOW_H
