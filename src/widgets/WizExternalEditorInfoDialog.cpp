#include "WizExternalEditorInfoDialog.h"
#include "ui_WizExternalEditorInfoDialog.h"

WizExternalEditorInfoDialog::WizExternalEditorInfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::WizExternalEditorInfoDialog)
{
    ui->setupUi(this);
}

WizExternalEditorInfoDialog::~WizExternalEditorInfoDialog()
{
    delete ui;
}
