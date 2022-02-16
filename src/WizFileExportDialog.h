//
// Created by pikachu on 2/16/2022.
//

#ifndef WIZNOTEPLUS_WIZFILEEXPORTDIALOG_H
#define WIZNOTEPLUS_WIZFILEEXPORTDIALOG_H

#include <QDialog>
#include "share/WizQtHelper.h"
class WizExplorerApp;
class WizDatabaseManager;
class QTreeWidget;
class QTreeWidgetItem;
class BaseItem;
class WizFileExportDialog : public QDialog {
Q_OBJECT
public:
    WizFileExportDialog(WizExplorerApp& app, QWidget *parent = nullptr);
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
    void exportFolder(QTreeWidgetItem* item);
    void exportNote(QTreeWidgetItem* item);

private:
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;
    QTreeWidget* m_treeWidget;
    bool m_isUpdateItemStatus;
    QTreeWidgetItem* m_rootItem;
    QString m_exportRootPath;
};


#endif //WIZNOTEPLUS_WIZFILEEXPORTDIALOG_H
