#include "WizExternalEditorSettingDialog.h"
#include "ui_WizExternalEditorSettingDialog.h"

WizExternalEditorSettingDialog::WizExternalEditorSettingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WizExternalEditorSettingDialog)
{
    ui->setupUi(this);
    m_extEditorTable = ui->listEditor;
}

WizExternalEditorSettingDialog::~WizExternalEditorSettingDialog()
{
    delete ui;
}
