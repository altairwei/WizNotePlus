//
// Created by pikachu on 2/16/2022.
//

#include "WizFileExportDialog.h"

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

#include "WizDef.h"
#include "share/WizSettings.h"
#include "share/WizMisc.h"
#include "database/WizDatabaseManager.h"
#include "database/WizDatabase.h"

class BaseItem : public QTreeWidgetItem
{
public:
    BaseItem(WizExplorerApp& app, QString strName, QString kbGUID): m_name(strName), m_kbGUID(kbGUID) {
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
    FolderItem(WizExplorerApp& app, QString strName, QString kbGUID): BaseItem(app, strName, kbGUID) {
        QString iconKey = "category_folder";
        QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), iconKey);
        setIcon(0, icon);
    }
    bool isFolder() const override { return true; }
};

class NoteItem : public BaseItem
{
public:
    NoteItem(WizExplorerApp& app, QString strName, QString kbGUID, QString docGUID, QString docType)
    : BaseItem(app, strName, kbGUID), m_docGUID(docGUID), m_docType(docType) {
        QString iconKey = "document_badge";
        QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), iconKey);
        setIcon(0, icon);
        setText(1, docGUID);
        setText(2, docType);
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
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_isUpdateItemStatus(false)
{
    this->resize(800, 600);
    auto layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->addWidget(new QLabel(tr("Choose notes")));

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    // m_treeWidget->setHeaderHidden(true);
    m_treeWidget->setHeaderLabels({"Name", "GUID", "Type"});
    connect(m_treeWidget, &QTreeWidget::itemChanged, this, &WizFileExportDialog::handleItemChanged);
    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &WizFileExportDialog::handleItemDoubleClicked);
    layout->addWidget(m_treeWidget);

    m_progress = new QProgressDialog(this);
    m_progress->setFixedWidth(400);

    auto hbox = new QHBoxLayout();
    hbox->addStretch(1);
    auto confirmBtn = new QPushButton(tr("Confirm"));
    auto cancelBtn = new QPushButton(tr("Cancel"));
    hbox->addWidget(confirmBtn);
    hbox->addWidget(cancelBtn);
    layout->addLayout(hbox);

    connect(confirmBtn, &QPushButton::clicked, this, &WizFileExportDialog::handleExportFile);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    initFolders();
}

void WizFileExportDialog::initFolders()
{
    auto pAllFoldersItem = new FolderItem(m_app, tr("Personal Notes"), m_dbMgr.db().kbGUID());
    pAllFoldersItem->setIcon(0, WizLoadSkinIcon(m_app.userSettings().skin(), "category_folders"));
    pAllFoldersItem->setCheckState(0, Qt::Unchecked);
    m_treeWidget->addTopLevelItem(pAllFoldersItem);
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
    qDebug() << "export root path:" << m_exportRootPath;
    exportFolder(m_rootItem);
    QMessageBox::information(nullptr, tr("Export Success"), tr("All notes selected exported"));
    QDesktopServices::openUrl(QUrl(m_exportRootPath));
}

void WizFileExportDialog::exportFolder(QTreeWidgetItem *item)
{
    if (item->checkState(0) == Qt::Unchecked) return;
    int nCount = item->childCount();
    for (int nIndex = 0; nIndex < nCount; ++nIndex)
    {
        auto child = static_cast<BaseItem*>(item->child(nIndex));
        if (child->isFolder()) {
            exportFolder(child);
        } else {
            exportNote(child);
        }
    }
}

void WizFileExportDialog::exportNote(QTreeWidgetItem *item)
{
    auto parentItem = static_cast<FolderItem*>(item->parent());
    if (!parentItem) return;
    QString path = m_exportRootPath + "/" + parentItem->name();
    auto noteItem = static_cast<NoteItem*>(item);
    QString filename = noteItem->name();
    qDebug() << "export " << noteItem->name();
    
    WizDatabase& db = m_dbMgr.db(noteItem->kbGUID());
    WIZDOCUMENTDATA data;
    if (!db.documentFromGuid(noteItem->docGUID(), data))
    {
        return;
    }
    if (!WizMakeSureDocumentExistAndBlockWidthDialog(db, data)) {
        return;
    }
    // TODO: handle cipher, not supported now
    //
    QString strHtmlFile;
    if (db.documentToTempHtmlFile(data, strHtmlFile))
    {
        qDebug() << "html:" << strHtmlFile;
        QFile file(strHtmlFile);
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "open html file fail:" << strHtmlFile;
            return;
        }
        QString strHtml = file.readAll();
        file.close();
        QTextDocument doc;
        doc.setHtml(strHtml);
        QString strText = doc.toPlainText();
        strText = strText.replace("&nbsp", " ");
        // write text to file
        QString outputFilePath = path + "/" + filename;
        qDebug() << "export to:" << outputFilePath;
        QDir().mkpath(path);
        if (!WizSaveUnicodeTextToUtf8File(outputFilePath, strText)) {
            qDebug() << "export fail:" << noteItem->name();
            return;
        }
        qDebug() << "export success:" << noteItem->name();
         
    }
    else
    {
        qDebug() << "error doc to html";
    }


}
