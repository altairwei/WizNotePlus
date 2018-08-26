#ifndef WIZEXTERNALEDITORSETTINGDIALOG_H
#define WIZEXTERNALEDITORSETTINGDIALOG_H

#include <QDialog>

class QTableWidget;

namespace Ui {
class WizExternalEditorSettingDialog;
}

class WizExternalEditorSettingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizExternalEditorSettingDialog(QWidget *parent = nullptr);
    ~WizExternalEditorSettingDialog();

private:
    Ui::WizExternalEditorSettingDialog *ui;
    QTableWidget* m_extEditorTable;

};

#endif // WIZEXTERNALEDITORSETTINGDIALOG_H
