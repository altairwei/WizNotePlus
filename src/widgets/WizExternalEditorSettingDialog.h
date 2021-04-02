#ifndef WIZEXTERNALEDITORSETTINGDIALOG_H
#define WIZEXTERNALEDITORSETTINGDIALOG_H

#include <QDialog>
#include <QMap>

class QTableWidget;
class QSettings;
class QPushButton;
class QLineEdit;
class QCheckBox;

typedef QMap<QString, QString> SettingMap;

namespace Ui {
class WizExternalEditorInfoDialog;
class WizExternalEditorSettingDialog;
}

class WizExternalEditorInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizExternalEditorInfoDialog(QWidget *parent = nullptr);
    WizExternalEditorInfoDialog(int dataRow, SettingMap data, QWidget *parent = nullptr);

    ~WizExternalEditorInfoDialog();

Q_SIGNALS:
    void dataAdded(SettingMap& data);
    void dataEdited(int row, SettingMap& data);

private:
    Ui::WizExternalEditorInfoDialog *ui;
    QLineEdit* m_editProgram;
    QLineEdit* m_editName;
    QLineEdit* m_editArguments;
    QCheckBox* m_checkTextEditor;
    QCheckBox* m_checkUTF8;
    QLineEdit* m_editOpenShortCut;
    int m_dataRow;
    bool m_isEditing = false;

private:
    void initForm(SettingMap& data);

private Q_SLOTS:
    void accept();
    void setSelectedProgramFile();
};

class WizExternalEditorSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizExternalEditorSettingDialog(QWidget *parent = nullptr);
    ~WizExternalEditorSettingDialog();

private:
    Ui::WizExternalEditorSettingDialog *ui;
    QTableWidget* m_extEditorTable;
    QSettings* m_extEditorSetting;

private:
    void loadDataFromIni(QSettings* settings);
    void writeDataToIni(QSettings* settings);
    void modifyRowContent(int row, SettingMap& data);

signals:
    void settingChanged();

private Q_SLOTS:
    void accept();
    void on_btnAddEditor_clicked();
    void on_btnDeleteEditor_clicked();
    void on_btnEditSetting_clicked();

    void addEditor(SettingMap& data);
    void modifyEditor(int row, SettingMap& data);

};

#endif // WIZEXTERNALEDITORSETTINGDIALOG_H
