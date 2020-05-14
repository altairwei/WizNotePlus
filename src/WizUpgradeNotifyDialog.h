#ifndef WIZPGRADENOTIFYDIALOG_H
#define WIZPGRADENOTIFYDIALOG_H

#include <QDialog>

class WizWebEngineView;

namespace Ui {
class WizUpgradeNotifyDialog;
}

class WizUpgradeNotifyDialog : public QDialog
{
    Q_OBJECT
    
public:
    WizUpgradeNotifyDialog(const QString& changeUrl, QWidget *parent = 0);
    WizUpgradeNotifyDialog(QWidget *parent = 0) : WizUpgradeNotifyDialog("", parent) { };
    ~WizUpgradeNotifyDialog();

    void showMarkdownContent(const QString &markdownSource);

private:
    QString createMarkdownContentPage(const QString &markdownSource);

private:
    Ui::WizUpgradeNotifyDialog *ui;
    WizWebEngineView *m_web;
};

#endif // WIZPGRADENOTIFYDIALOG_H
