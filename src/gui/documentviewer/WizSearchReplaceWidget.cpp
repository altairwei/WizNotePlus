#include "WizSearchReplaceWidget.h"
#include "ui_WizSearchReplaceWidget.h"

WizSearchReplaceWidget::WizSearchReplaceWidget(QWidget* parent) :
    QDialog(parent),
    ui(new Ui::WizSearchReplaceWidget)
{
    ui->setupUi(this);

    // Disable auto default button
    ui->btn_pre->setAutoDefault(false);
    ui->btn_next->setAutoDefault(false);
    ui->btn_replace->setAutoDefault(false);
    ui->btn_replaceAll->setAutoDefault(false);

    //setWindowFlags(Qt::WindowStaysOnTopHint);  //could cause window fullscreen on mac
}

WizSearchReplaceWidget::~WizSearchReplaceWidget()
{
    emit findNext("", false);
    delete ui;
}

void WizSearchReplaceWidget::showInEditor(const QRect& rcEditor)
{
    QPoint pos;
    pos.setX(rcEditor.x() + (rcEditor.width() - width()) / 2);
    pos.setY(rcEditor.y());
    setGeometry(QRect(pos, size()));

    setParent(parentWidget());
    show();
    setWindowState(windowState() & ~Qt::WindowFullScreen | Qt::WindowActive);
    setFixedSize(size());
    activateWindow();
    raise();
    //ui->lineEdit_source->setFocus();
}

void WizSearchReplaceWidget::setSourceText(const QString& text)
{
    ui->lineEdit_source->setText(text);
}

void WizSearchReplaceWidget::closeEvent(QCloseEvent* event)
{
    clearAllText();
    QWidget::closeEvent(event);
}

void WizSearchReplaceWidget::on_btn_pre_clicked()
{
    emit findPre(ui->lineEdit_source->text(), ui->checkBox_casesenitive->checkState() == Qt::Checked);
}

void WizSearchReplaceWidget::on_btn_next_clicked()
{
    emit findNext(ui->lineEdit_source->text(), ui->checkBox_casesenitive->checkState() == Qt::Checked);
}

void WizSearchReplaceWidget::on_btn_replace_clicked()
{
    emit replaceAndFindNext(ui->lineEdit_source->text(), ui->lineEdit_repalce->text(),
                            ui->checkBox_casesenitive->checkState() == Qt::Checked);
}

void WizSearchReplaceWidget::on_btn_replaceAll_clicked()
{
    emit replaceAll(ui->lineEdit_source->text(), ui->lineEdit_repalce->text(),
                    ui->checkBox_casesenitive->checkState() == Qt::Checked);
}

void WizSearchReplaceWidget::on_lineEdit_source_returnPressed()
{
    emit findNext(ui->lineEdit_source->text(), ui->checkBox_casesenitive->checkState() == Qt::Checked);
}

void WizSearchReplaceWidget::on_lineEdit_source_textChanged(const QString &text)
{
    emit findNext(text, ui->checkBox_casesenitive->checkState() == Qt::Checked);
}

void WizSearchReplaceWidget::clearAllText()
{
    emit findNext("", false);
    ui->lineEdit_repalce->clear();
    ui->lineEdit_source->clear();
}
