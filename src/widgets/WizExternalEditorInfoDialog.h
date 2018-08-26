#ifndef WIZEXTERNALEDITORINFODIALOG_H
#define WIZEXTERNALEDITORINFODIALOG_H

#include <QDialog>

namespace Ui {
class WizExternalEditorInfoDialog;
}

class WizExternalEditorInfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WizExternalEditorInfoDialog(QWidget *parent = nullptr);
    ~WizExternalEditorInfoDialog();

private:
    Ui::WizExternalEditorInfoDialog *ui;
};

#endif // WIZEXTERNALEDITORINFODIALOG_H
