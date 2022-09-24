#include "FileLineEdit.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QDir>

FileLineEdit::FileLineEdit(const QString &labelText, QWidget *parent)
    : QWidget(parent)
    , m_filter("")
{
    m_label = new QLabel;
    m_label->setText(labelText);
    m_label->setAlignment(Qt::AlignVCenter);
    m_lineEdit = new QLineEdit;
    m_btn = new QPushButton;
    m_btn->setText(tr("Open..."));

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_label);
    layout->addWidget(m_lineEdit);
    layout->addWidget(m_btn);
    setLayout(layout);

    connect(m_btn, &QPushButton::clicked,
            this, &FileLineEdit::handleBtnClicked);
}

void FileLineEdit::setLabelText(const QString &text)
{
    m_label->setText(text);
}

void FileLineEdit::handleBtnClicked()
{
    QString file = QFileDialog::getOpenFileName(
        this, tr("Choose file"), QDir::homePath(), m_filter);
    m_lineEdit->setText(file);
}

void DirLineEdit::handleBtnClicked()
{
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Choose directory"), QDir::homePath());
    lineEdit()->setText(dir);
}
