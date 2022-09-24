#ifndef FILEEXPORTWIZARD_H
#define FILEEXPORTWIZARD_H

#include <QWizard>
#include <QWizardPage>
#include <QList>

#include "share/WizQtHelper.h"

class WizExplorerApp;
class WizDatabaseManager;
class WizFileExporter;
class BaseItem;
class NoteItem;
class DirLineEdit;

class QTreeWidget;
class QProgressBar;
class QLabel;
class QPushButton;
class QTreeWidgetItem;
class QTextEdit;
class QLineEdit;
class QCheckBox;

class FileExportWizard : public QWizard
{
    Q_OBJECT

public:
    FileExportWizard(WizExplorerApp& app, QWidget *parent = nullptr);
    FileExportWizard(const QString &location, WizExplorerApp& app, QWidget *parent = nullptr);
};

class FileExportPageIntro : public QWizardPage
{
    Q_OBJECT

public:
    FileExportPageIntro(QWidget *parent = nullptr);
};

class FileExportPageDocList : public QWizardPage
{
    Q_OBJECT

public:
    FileExportPageDocList(WizExplorerApp& app, QWidget *parent = nullptr)
        : FileExportPageDocList("", app, parent) {}
    FileExportPageDocList(const QString &location, WizExplorerApp& app, QWidget *parent = nullptr);
    void initializePage() override;
    bool isComplete() const override;
    int nextId() const override;

public Q_SLOTS:
    void handleCancelBtnClicked() { m_cancel = true; };
    void initFolders();
    void initFoldersFromLocation(const QString &location);
    void handleItemChanged(QTreeWidgetItem *item, int column);
    void handleItemDoubleClicked(QTreeWidgetItem *item, int column);
    void updateSelection();

Q_SIGNALS:
    void initialized();
    void willLeave() const;

private:
    void initFolderItem(QTreeWidgetItem* pParent,
                     const QString& strParentLocation,
                     const CWizStdStringArray& arrayAllLocation);
    void updateParentItemStatus(QTreeWidgetItem* item);
    void updateChildItemStatus(QTreeWidgetItem* item);
    QStringList findSelectedNotes(BaseItem* item);

private:
    WizExplorerApp &m_app;
    WizDatabaseManager &m_dbMgr;
    QTreeWidget *m_treeWidget;
    QProgressBar *m_progress;
    QLabel *m_statusText;
    BaseItem* m_rootItem;
    bool m_cancel;
    bool m_isUpdateItemStatus;
    QString m_rootLocation;
};

class FileExportPageOptions : public QWizardPage
{
    Q_OBJECT

public:
    FileExportPageOptions(QWidget *parent = nullptr);

private:
    DirLineEdit *m_outputFolder;
    QCheckBox *m_keepFolder;
    QCheckBox *m_compress;
    QCheckBox *m_exportMetainfo;
    QCheckBox *m_noTitleFolderIfPossible;
};

class FileExportPageExport : public QWizardPage
{
    Q_OBJECT

public:
    FileExportPageExport(WizExplorerApp& app, QWidget *parent = nullptr);

    void initializePage() override;
    bool isComplete() const override;

Q_SIGNALS:
    void initialized();

private Q_SLOTS:
    void handleCancelBtnClicked() { m_cancel = true; };

private:
    void insertLog(const QString &log);
    void handleExportFile();

private:
    WizExplorerApp &m_app;
    WizDatabaseManager &m_dbMgr;
    QProgressBar *m_progress;
    QTextEdit *m_details;
    bool m_cancel;
    WizFileExporter* m_exporter;
};

#endif // FILEEXPORTWIZARD_H
