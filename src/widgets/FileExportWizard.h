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
class QTableWidget;
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
    FileExportWizard(WizExplorerApp& app, QWidget *parent = nullptr)
        : FileExportWizard("", app, parent) {}
    FileExportWizard(const QString &location, WizExplorerApp& app, QWidget *parent = nullptr);
};

class FEPageIntro : public QWizardPage
{
    Q_OBJECT

public:
    FEPageIntro(QWidget *parent = nullptr);
};

class FEPageDocList : public QWizardPage
{
    Q_OBJECT

public:
    FEPageDocList(WizExplorerApp& app, QWidget *parent = nullptr)
        : FEPageDocList("", app, parent) {}
    FEPageDocList(const QString &location, WizExplorerApp& app, QWidget *parent = nullptr);
    void initializePage() override;
    void cleanupPage() override;

    Q_PROPERTY(QList<QVariant> documents MEMBER m_documents NOTIFY documentsChanged)

public Q_SLOTS:
    void handleCancelBtnClicked() { m_cancel = true; };
    void initFolders();
    void handleItemChanged(QTreeWidgetItem *item, int column);
    void handleItemDoubleClicked(QTreeWidgetItem *item, int column);
    void updateSelection();

Q_SIGNALS:
    void firstInitialized();
    void documentsChanged(const QList<QVariant>&);

private:
    void initFolderItem(QTreeWidgetItem* pParent,
                     const QString& strParentLocation,
                     const CWizStdStringArray& arrayAllLocation);
    void updateParentItemStatus(QTreeWidgetItem* item);
    void updateChildItemStatus(QTreeWidgetItem* item);
    QList<QVariant> findSelectedNotes(BaseItem* item);

private:
    WizExplorerApp &m_app;
    WizDatabaseManager &m_dbMgr;
    QTreeWidget *m_treeWidget;
    QProgressBar *m_progress;
    QLabel *m_statusText;
    BaseItem* m_rootItem;
    bool m_cancel;
    bool m_firstInitialized;
    QString m_rootLocation;
    QList<QVariant> m_documents;
};

class FEPageFormatSelect : public QWizardPage
{
    Q_OBJECT

public:
    FEPageFormatSelect(QWidget *parent = nullptr);
    void initializePage() override;
    void cleanupPage() override;

private:
    QTableWidget *m_table;
};

class FEPageOptions : public QWizardPage
{
    Q_OBJECT

public:
    FEPageOptions(QWidget *parent = nullptr);

private:
    DirLineEdit *m_outputFolder;
    QCheckBox *m_keepFolder;
    QCheckBox *m_compress;
    QCheckBox *m_exportMetainfo;
    QCheckBox *m_noTitleFolderIfPossible;
    QCheckBox *m_handleRichTextInMarkdown;
    QCheckBox *m_convertRichTextToMarkdown;
};

class FEPageExport : public QWizardPage
{
    Q_OBJECT

public:
    FEPageExport(WizExplorerApp& app, QWidget *parent = nullptr);

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
