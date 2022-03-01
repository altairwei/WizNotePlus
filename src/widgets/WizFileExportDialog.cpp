//
// Created by pikachu on 2/16/2022.
//

#include "WizFileExportDialog.h"
#include "ui_WizFileExportDialog.h"

#include <QDebug>
#include <QTreeWidget>
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QTextDocument>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QDesktopServices>
#include <QProgressDialog>
#include <QApplication>

#include "WizFileExporter.h"
#include "WizDef.h"
#include "share/WizSettings.h"
#include "share/WizMisc.h"
#include "database/WizDatabaseManager.h"
#include "database/WizDatabase.h"

class BaseItem : public QTreeWidgetItem
{

public:
    BaseItem(WizExplorerApp& app, QString strName, QString kbGUID)
        : m_name(strName)
        , m_kbGUID(kbGUID)
    {
        setText(0, strName);
    }

    QString name() const { return m_name; }
    QString kbGUID() const { return m_kbGUID; }
    virtual bool isFolder() const { return false; }

protected:
    QString m_name;
    QString m_kbGUID;
};

class FolderItem : public BaseItem
{

public:
    FolderItem(WizExplorerApp& app, QString strName, QString kbGUID)
        : BaseItem(app, strName, kbGUID)
    {

        auto name = WizDatabase::getLocationDisplayName(strName);
        if (name.isEmpty()) name = strName;
        setText(0, name);

        QIcon icon;
        if (::WizIsPredefinedLocation(strName) && strName == "/My Journals/") {
            icon = WizLoadSkinIcon(app.userSettings().skin(), "category_folder_diary");
        } else {
            icon  = WizLoadSkinIcon(app.userSettings().skin(), "category_folder");
        }

        setIcon(0, icon);
    }

    bool isFolder() const override { return true; }
    QString location() const { return m_name; }
};

class NoteItem : public BaseItem
{

public:
    NoteItem(WizExplorerApp& app, QString strName, QString kbGUID, QString docGUID, QString docType)
        : BaseItem(app, strName, kbGUID)
        , m_docGUID(docGUID)
        , m_docType(docType)
    {
        // TODO: display icon for encrypted notes
        QString iconKey = "document_badge";
        QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), iconKey);
        setIcon(0, icon);
    }

    bool isFolder() const override { return false; }
    QString docGUID() const { return m_docGUID; }
    QString docType() const { return m_docType; }

private:
    QString m_docGUID;
    QString m_docType;
};

WizFileExportDialog::WizFileExportDialog(WizExplorerApp &app, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WizFileExportDialog)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_isUpdateItemStatus(false)
{
    ui->setupUi(this);

    ui->treeSelectNotes->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeSelectNotes->setHeaderHidden(true);
    connect(ui->treeSelectNotes, &QTreeWidget::itemChanged, this, &WizFileExportDialog::handleItemChanged);
    connect(ui->treeSelectNotes, &QTreeWidget::itemDoubleClicked, this, &WizFileExportDialog::handleItemDoubleClicked);

    m_progress = new QProgressDialog(this);
    m_progress->setFixedWidth(400);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &WizFileExportDialog::handleExportFile);

    m_exporter = new WizFileExporter(m_dbMgr, this);

    initFolders();
}

WizFileExportDialog::~WizFileExportDialog()
{
    delete ui;
}

void WizFileExportDialog::initFolders()
{
    auto pAllFoldersItem = new FolderItem(m_app, tr("Personal Notes"), m_dbMgr.db().kbGUID());
    pAllFoldersItem->setIcon(0, WizLoadSkinIcon(m_app.userSettings().skin(), "category_folders"));
    pAllFoldersItem->setCheckState(0, Qt::Unchecked);
    ui->treeSelectNotes->addTopLevelItem(pAllFoldersItem);
    m_rootItem = pAllFoldersItem;

    CWizStdStringArray arrayAllLocation;
    m_dbMgr.db().getAllLocations(arrayAllLocation);

    // folder cache
    CWizStdStringArray arrayExtLocation;
    m_dbMgr.db().getExtraFolder(arrayExtLocation);

    if (!arrayExtLocation.empty()) {
        for (CWizStdStringArray::const_iterator it = arrayExtLocation.begin();
             it != arrayExtLocation.end();
             it++) {
            if (-1 == ::WizFindInArray(arrayAllLocation, *it)) {
                arrayAllLocation.push_back(*it);
            }
        }
    }

    if (arrayAllLocation.empty()) {
        arrayAllLocation.push_back(m_dbMgr.db().getDefaultNoteLocation());
    }

    m_progress->setLabelText(tr("Scanning Database:"));
    m_progress->setRange(1, arrayAllLocation.size());
    m_progress->setModal(true);

    initFolders(pAllFoldersItem, "", arrayAllLocation);

    pAllFoldersItem->setExpanded(true);
    pAllFoldersItem->sortChildren(0, Qt::AscendingOrder);
}

void WizFileExportDialog::initFolders(QTreeWidgetItem *pParent, const QString &strParentLocation,
                                      const CWizStdStringArray &arrayAllLocation)
{
    m_progress->setValue(m_progress->value() + 1);
    qApp->processEvents();

    if (m_progress->wasCanceled()) return;

    CWizStdStringArray arrayLocation;
    WizDatabase::getChildLocations(arrayAllLocation, strParentLocation, arrayLocation);

    // Find all sub-folders
    CWizStdStringArray::const_iterator it;
    for (const auto& strLocation : arrayLocation) {
        qApp->processEvents();
        if (m_progress->wasCanceled()) return;

        if (m_dbMgr.db().isInDeletedItems(strLocation))
            continue;

        auto pFolderItem = new FolderItem(m_app, strLocation, m_dbMgr.db().kbGUID());
        pFolderItem->setCheckState(0, Qt::Unchecked);
        pParent->addChild(pFolderItem);

        initFolders(pFolderItem, strLocation, arrayAllLocation);
    }

    // Find all direct documents
    CWizDocumentDataArray arrayDocument;
    m_dbMgr.db().getDocumentsByLocation(strParentLocation, arrayDocument);
    for (const auto& doc: arrayDocument) {
        qApp->processEvents();
        if (m_progress->wasCanceled()) return;

        if (!WizIsMarkdownNote(doc)) continue;
        auto pNoteItem = new NoteItem(m_app, doc.strTitle, m_dbMgr.db().kbGUID(), doc.strGUID, doc.strType);
        pNoteItem->setCheckState(0, Qt::Unchecked);
        pParent->addChild(pNoteItem);
    }

}

void WizFileExportDialog::handleItemChanged(QTreeWidgetItem *item, int column)
{
    if (m_isUpdateItemStatus) return;
    m_isUpdateItemStatus = true;
    updateChildItemStatus(item);
    updateParentItemStatus(item);
    m_isUpdateItemStatus = false;
}

void WizFileExportDialog::updateParentItemStatus(QTreeWidgetItem* item)
{
    auto parent = item->parent();
    if (Q_NULLPTR == parent)
    {
        return;
    }

    parent->setCheckState(0, item->checkState(0));
    int nCount = parent->childCount();
    for (int nIndex = 0; nIndex < nCount; ++nIndex)
    {
        auto child = parent->child(nIndex);
        if (child->checkState(0) != parent->checkState(0))
        {
            parent->setCheckState(0, Qt::PartiallyChecked);
            break;
        }
    }

    updateParentItemStatus(parent);
}

void WizFileExportDialog::updateChildItemStatus(QTreeWidgetItem* item)
{
    int nCount = item->childCount();
    for (int nIndex = 0; nIndex < nCount; ++nIndex)
    {
        auto child = item->child(nIndex);
        child->setCheckState(0, item->checkState(0));
        if (child->childCount() > 0)
        {
            updateChildItemStatus(child);
        }
    }
}

void WizFileExportDialog::handleItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    auto _item = static_cast<BaseItem*>(item);
    if (_item->isFolder()) {
        return;
    }
    auto state = item->checkState(0);
    if (state == Qt::Unchecked) {
        item->setCheckState(0, Qt::Checked);
    } else if (state == Qt::Checked) {
        item->setCheckState(0, Qt::Unchecked);
    }
}

void WizFileExportDialog::handleExportFile()
{
    if (m_rootItem->checkState(0) == Qt::Unchecked) {
        QMessageBox::warning(nullptr, tr("Export Error"), tr("No selected notes"));
        return;
    }

    m_exportRootPath = QFileDialog::getExistingDirectory();
    if (m_exportRootPath.isEmpty()) return;

    auto notes = findSelectedNotes(m_rootItem);

    m_progress->reset();
    m_progress->setLabelText(tr("Exporting notes:"));
    m_progress->setRange(1, notes.size());
    m_progress->setModal(true);

    foreach (auto note, notes) {
        m_progress->setValue(m_progress->value() + 1);
        qApp->processEvents();
        if (m_progress->wasCanceled()) return;

        WizDatabase& db = m_dbMgr.db(note->kbGUID());
        WIZDOCUMENTDATA data;
        if (!db.documentFromGuid(note->docGUID(), data)) {
            auto ret = QMessageBox::critical(
                this,
                tr("Error occurred! Do you want to continue?"),
                tr("Can't find document for GUID: %1").arg(note->docGUID()),
                QMessageBox::Ok | QMessageBox::Abort,
                QMessageBox::Ok
            );
            if (ret == QMessageBox::Abort)
                return;
            else
                continue;
        }

        QString destFolder = m_exportRootPath;
        if (ui->checkKeepFolder->checkState() == Qt::Checked)
            destFolder = m_exportRootPath + data.strLocation;

        QString error = "Unknown error";
        bool ok = m_exporter->exportNote(
            data, destFolder, WizFileExporter::Markdown, false, &error);
        if (!ok) {
            auto ret = QMessageBox::critical(
                this,
                tr("Error occurred! Do you want to continue?"),
                error,
                QMessageBox::Ok | QMessageBox::Abort,
                QMessageBox::Ok
            );
            if (ret == QMessageBox::Abort)
                return;
            else
                continue;
        }
    }

    QMessageBox::information(nullptr, tr("Export Success"), tr("All notes selected exported"));
    QDesktopServices::openUrl(QUrl(m_exportRootPath));
}

QList<NoteItem*> WizFileExportDialog::findSelectedNotes(BaseItem* item)
{
    QList<NoteItem*> notes;
    if (item->checkState(0) == Qt::Unchecked)
        return notes;

    int nCount = item->childCount();
    for (int nIndex = 0; nIndex < nCount; ++nIndex)
    {
        auto child = static_cast<BaseItem*>(item->child(nIndex));
        if (child->isFolder()) {
            if (child->checkState(0) == Qt::Unchecked)
                continue;
            notes.append(findSelectedNotes(child));
        } else {
            if (child->checkState(0) == Qt::Checked) {
                notes << static_cast<NoteItem*>(child);
            }
        }
    }

    return notes;
}
