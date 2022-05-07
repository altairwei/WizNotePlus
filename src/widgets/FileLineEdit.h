#ifndef FILELINEEDIT_H
#define FILELINEEDIT_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;

class FileLineEdit : public QWidget
{
    Q_OBJECT

public:
    explicit FileLineEdit(QWidget *parent = nullptr) : FileLineEdit("", parent) {}
    FileLineEdit(const QString &labelText, QWidget *parent = nullptr);

    void setLabelText(const QString &text);
    void setFilter(const QString &text);
    QLineEdit *lineEdit() { return m_lineEdit; }
    QLabel *label() { return m_label; }
    QPushButton *button() { return m_btn; }

private Q_SLOTS:
    virtual void handleBtnClicked();

private:
    QLabel *m_label;
    QLineEdit *m_lineEdit;
    QPushButton *m_btn;
    QString m_filter;
};

class DirLineEdit : public FileLineEdit
{
    Q_OBJECT

public:
    explicit DirLineEdit(QWidget *parent = nullptr) : DirLineEdit("", parent) {}
    DirLineEdit(const QString &labelText, QWidget *parent = nullptr)
        : FileLineEdit(labelText, parent) {}

    void handleBtnClicked() override;
};

#endif // FILELINEEDIT_H
