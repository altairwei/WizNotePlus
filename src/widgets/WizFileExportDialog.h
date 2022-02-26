//
// Created by pikachu on 2/16/2022.
//

#ifndef WIZNOTEPLUS_WIZFILEEXPORTDIALOG_H
#define WIZNOTEPLUS_WIZFILEEXPORTDIALOG_H

#include <QDialog>
#include <QList>

#include "share/WizQtHelper.h"

class QTreeWidget;
class QTreeWidgetItem;
class QProgressDialog;
class QLabel;

class WizExplorerApp;
class WizDatabaseManager;
class BaseItem;
class NoteItem;
class WizFileExporter;

namespace Ui {
class WizFileExportDialog;
}

class WizFileExportDialog : public QDialog
{
    Q_OBJECT

public:
    WizFileExportDialog(WizExplorerApp& app, QWidget *parent = nullptr);
    ~WizFileExportDialog();

private:
    void initFolders();
    void initFolders(QTreeWidgetItem* pParent,
                     const QString& strParentLocation,
                     const CWizStdStringArray& arrayAllLocation);
    void handleExportFile();
    void handleItemChanged(QTreeWidgetItem *item, int column);
    void handleItemDoubleClicked(QTreeWidgetItem *item, int column);
    void updateParentItemStatus(QTreeWidgetItem* item);
    void updateChildItemStatus(QTreeWidgetItem* item);
    QList<NoteItem*> findSelectedNotes(BaseItem* item);

private:
    Ui::WizFileExportDialog *ui;
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;
    QTreeWidget* m_treeWidget;
    bool m_isUpdateItemStatus;
    BaseItem* m_rootItem;
    QString m_exportRootPath;
    QProgressDialog* m_progress;
    WizFileExporter* m_exporter;
};


#endif //WIZNOTEPLUS_WIZFILEEXPORTDIALOG_H
