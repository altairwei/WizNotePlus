#include "FileExportWizard.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QTreeWidgetItem>
#include <QApplication>
#include <QHeaderView>
#include <QFrame>
#include <QTextEdit>
#include <QMessageBox>
#include <QDesktopServices>
#include <QCheckBox>
#include <QDebug>
#include <QLineEdit>
#include <QInputDialog>
#include <QTimer>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QItemDelegate>
#include <QComboBox>
#include <QProcess>

#include "WizDef.h"
#include "database/WizDatabase.h"
#include "database/WizDatabaseManager.h"
#include "share/WizSettings.h"
#include "WizFileExporter.h"
#include "widgets/FileLineEdit.h"
#include "html/WizHtmlConverter.h"
#include "utils/WizPathResolve.h"

FileExportWizard::FileExportWizard(const QString &location, WizExplorerApp& app, QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle(tr("Export Wizard"));
#ifndef Q_OS_MAC
    setWizardStyle(ModernStyle);
#endif

    addPage(new FEPageIntro);
    addPage(new FEPageDocList(location, app));
    addPage(new FEPageFormatSelect);
    addPage(new FEPageOptions);
    addPage(new FEPageExport(app));
}

FEPageIntro::FEPageIntro(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Introduction"));

    QLabel *label = new QLabel(tr(
        "This wizard will help you export documents to "
        "various formats."));
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    setLayout(layout);
}

/////////////////////////////////////////////////////////////////////
/// Document List Page
/////////////////////////////////////////////////////////////////////

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
    NoteItem(WizExplorerApp& app, QString kbGUID, const WIZDOCUMENTDATAEX &data)
        : BaseItem(app, data.strTitle, kbGUID)
        , m_docGUID(data.strGUID)
        , m_docType(data.strType)
        , m_docData(data)
    {
        QString iconKey = data.nProtected == 1 ?
                    "document_badge_encrypted" : "document_badge";
        QIcon icon = WizLoadSkinIcon(app.userSettings().skin(), iconKey);
        setIcon(0, icon);
    }

    bool isFolder() const override { return false; }
    QString docGUID() const { return m_docGUID; }
    QString docType() const { return m_docType; }
    const WIZDOCUMENTDATA *docData() const { return &m_docData; }

private:
    QString m_docGUID;
    QString m_docType;
    WIZDOCUMENTDATA m_docData;
};


FEPageDocList::FEPageDocList(const QString &location, WizExplorerApp& app, QWidget *parent)
    : QWizardPage(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_cancel(false)
    , m_firstInitialized(false)
    , m_rootLocation(location)
{
    setTitle(tr("Select Documents"));
    setSubTitle(
        tr("Please select documents you want to export.") +
        " " + tr("Collaboration notes are not supported now!")
    );

    m_statusText = new QLabel;

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->hide();
    m_treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    m_treeWidget->setHeaderHidden(true);

    m_progress = new QProgressBar(this);

    QHBoxLayout *progressLayout = new QHBoxLayout;
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_statusText);
    layout->addLayout(progressLayout);
    progressLayout->addWidget(m_progress);
    layout->addWidget(m_treeWidget);
    setLayout(layout);

    connect(m_treeWidget, &QTreeWidget::itemChanged,
            this, &FEPageDocList::handleItemChanged);
    connect(m_treeWidget, &QTreeWidget::itemDoubleClicked,
            this, &FEPageDocList::handleItemDoubleClicked);

    registerField("documents*", this, "documents", SIGNAL(documentsChanged(const QList<QVariant>&)));
}

void FEPageDocList::initializePage()
{
    if (!m_firstInitialized) {
        // wizard() will be available after initialization
        connect(wizard(), &QWizard::finished,
                this, &FEPageDocList::handleCancelBtnClicked);
        m_firstInitialized = true;
        // Push initFolders to event loop
        QTimer::singleShot(0, this, SLOT(initFolders()));
        Q_EMIT firstInitialized();
    }
}

void FEPageDocList::cleanupPage()
{
    {
        const QSignalBlocker blocker(m_treeWidget);
        m_rootItem->setCheckState(0, Qt::Unchecked);
        updateChildItemStatus(m_rootItem);
    }

    QWizardPage::cleanupPage();
}

void FEPageDocList::initFolders()
{
    const QSignalBlocker blocker(m_treeWidget);
    QString rootLabel = m_rootLocation.isEmpty() ? tr("Personal Notes") : m_rootLocation;
    auto rootFoldersItem = new FolderItem(m_app, rootLabel, m_dbMgr.db().kbGUID());
    rootFoldersItem->setIcon(0, WizLoadSkinIcon(m_app.userSettings().skin(), "category_folders"));
    rootFoldersItem->setCheckState(0, Qt::Unchecked);
    m_treeWidget->addTopLevelItem(rootFoldersItem);
    m_rootItem = rootFoldersItem;

    CWizStdStringArray locationsToSearch;

    if (m_rootLocation.isEmpty()) {
        m_dbMgr.db().getAllLocations(locationsToSearch);

        CWizStdStringArray arrayExtLocation;
        m_dbMgr.db().getExtraFolder(arrayExtLocation);

        if (!arrayExtLocation.empty()) {
            for (CWizStdStringArray::const_iterator it = arrayExtLocation.begin();
                 it != arrayExtLocation.end();
                 it++) {
                if (-1 == ::WizFindInArray(locationsToSearch, *it)) {
                    locationsToSearch.push_back(*it);
                }
            }
        }

        if (locationsToSearch.empty())
            locationsToSearch.push_back(m_dbMgr.db().getDefaultNoteLocation());

    } else {
        m_dbMgr.db().getAllChildLocations(m_rootLocation, locationsToSearch);
    }

    m_statusText->setText(tr("<i>Scanning Database...</i>"));
    m_progress->setRange(1, (int)locationsToSearch.size());

    initFolderItem(rootFoldersItem, m_rootLocation, locationsToSearch);

    rootFoldersItem->setExpanded(true);
    //pAllFoldersItem->sortChildren(0, Qt::AscendingOrder);

    m_treeWidget->show();
    m_progress->hide();

    m_statusText->setText(tr("Choose documents:"));
    Q_EMIT completeChanged();
}

// TODO: 优化 IO，减少 SQL 查询次数。
void FEPageDocList::initFolderItem(QTreeWidgetItem *pParent, const QString &strParentLocation,
                                      const CWizStdStringArray &arrayAllLocation)
{
    m_progress->setValue(m_progress->value() + 1);
    qApp->processEvents();

    if (m_cancel) return;

    CWizStdStringArray arrayLocation;
    WizDatabase::getChildLocations(arrayAllLocation, strParentLocation, arrayLocation);

    // Find all sub-folders
    CWizStdStringArray::const_iterator it;
    for (const auto& strLocation : arrayLocation) {
        qApp->processEvents();
        if (m_cancel) return;

        if (m_dbMgr.db().isInDeletedItems(strLocation))
            continue;

        auto pFolderItem = new FolderItem(m_app, strLocation, m_dbMgr.db().kbGUID());
        pFolderItem->setCheckState(0, Qt::Unchecked);
        pParent->addChild(pFolderItem);

        initFolderItem(pFolderItem, strLocation, arrayAllLocation);
    }

    // Find all direct documents
    CWizDocumentDataArray arrayDocument;
    m_dbMgr.db().getDocumentsByLocation(strParentLocation, arrayDocument);
    for (const auto& doc: arrayDocument) {
        qApp->processEvents();
        if (m_cancel) return;

        if (doc.strType == "collaboration")
            continue;

        auto pNoteItem = new NoteItem(m_app, m_dbMgr.db().kbGUID(), doc);
        pNoteItem->setCheckState(0, Qt::Unchecked);
        pParent->addChild(pNoteItem);
    }

}

void FEPageDocList::handleItemChanged(QTreeWidgetItem *item, int column)
{
    // Block signals emitted by updateChild/ParentItemStatus
    const QSignalBlocker blocker(m_treeWidget);
    updateChildItemStatus(item);
    updateParentItemStatus(item);
    updateSelection();
}

void FEPageDocList::updateParentItemStatus(QTreeWidgetItem* item)
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

void FEPageDocList::updateChildItemStatus(QTreeWidgetItem* item)
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

void FEPageDocList::handleItemDoubleClicked(QTreeWidgetItem *item, int column)
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

QList<QVariant> FEPageDocList::findSelectedNotes(BaseItem* item)
{
    QList<QVariant> notes;
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
                notes.append(QVariant::fromValue(
                    static_cast<NoteItem*>(child)->docData()));
            }
        }
    }

    return notes;
}

void FEPageDocList::updateSelection()
{
    m_documents = findSelectedNotes(m_rootItem);
    emit documentsChanged(m_documents);
}

/////////////////////////////////////////////////////////////////////
/// Output Format Page
/////////////////////////////////////////////////////////////////////

QWidget *FEOutputFormatDelegate::createEditor(
    QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const
{
    QComboBox *editor = new QComboBox(parent);

    // Get the value of first column
    QModelIndex siblingIndex = index.siblingAtColumn(0);
    QString value = index.model()->data(siblingIndex).toString();

    if(value == "Common") {
        editor->addItems({"Markdown", "HTML", "MHTML", "PDF"});
    } else if(value == "Markdown") {
        editor->addItems({"Markdown", "HTML", "MHTML", "PDF"});
    } else if (value == "Outline") {
        editor->addItems({"Markdown", "HTML", "MHTML", "PDF"});
    } else if (value == "Handwriting") {
        editor->addItems({"HTML"});
    } else {
        editor->addItems({"HTML"});
    }

    return editor;
}

void FEOutputFormatDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    comboBox->setCurrentText(value);
}

void FEOutputFormatDelegate::setModelData(
    QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentText(), Qt::EditRole);
}

FEPageFormatSelect::FEPageFormatSelect(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Output Format Selection"));
    setSubTitle(tr("Please select what format you want "
                   "to export each type of document in."));

    m_table = new QTableWidget;
    m_table->setColumnCount(3);
    m_table->setEditTriggers(QAbstractItemView::AllEditTriggers);
    m_table->setItemDelegateForColumn(2, new FEOutputFormatDelegate());

    m_table->verticalHeader()->setHidden(true);
    m_table->setHorizontalHeaderLabels(
        QStringList() << tr("Doc Type") << tr("Count") << tr("Output Format"));
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_table);
    setLayout(layout);

    registerField("outputFormats*", this, "formats", SIGNAL(formatsChanged(const QMap<QString, QString>&)));
    connect(m_table, &QTableWidget::cellChanged,
            this, &FEPageFormatSelect::updateOutputFormats);
}

void FEPageFormatSelect::initializePage()
{
    QMap<QString, size_t> noteTypeNo;
    auto notes = field("documents").toList();
    foreach (const QVariant &no, notes) {
        if (!no.canConvert<const WIZDOCUMENTDATA*>())
            continue;
        auto doc = no.value<const WIZDOCUMENTDATA*>();
        if (WizIsMarkdownNote(*doc)) {
            noteTypeNo["Markdown"]++;
        } else if (doc->strType == "outline") {
            noteTypeNo["Outline"]++;
        } else if (doc->strType == "collaboration") {
            noteTypeNo["Collaboration"]++;
        } else if (doc->strType == "svgpainter") {
            noteTypeNo["Handwriting"]++;
        } else {
            noteTypeNo["Common"]++;
        }
    }

    m_table->setRowCount(noteTypeNo.size());
    unsigned int row = 0;
    QMap<QString, size_t>::const_iterator i = noteTypeNo.constBegin();
    while (i != noteTypeNo.constEnd()) {
        // First column
        QTableWidgetItem *item = new QTableWidgetItem(i.key());
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(row, 0, item);
        // Second column
        item = new QTableWidgetItem(QString::number(i.value()));
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(row, 1, item);
        // Third column
        QString defaultFormat("HTML");
        if (i.key() == "Markdown")
            defaultFormat = "Markdown";
        m_table->setItem(row, 2, new QTableWidgetItem(defaultFormat));

        ++row;
        ++i;
    }

}

void FEPageFormatSelect::cleanupPage()
{
    m_table->clearContents();
}

void FEPageFormatSelect::updateOutputFormats(int row, int column)
{
    if (column != 2)
        return;

    QTableWidgetItem *it = m_table->item(row, column);
    QString output_format = it->text();
    it = m_table->item(row, 0);
    QString doc_format = it->text();

    m_formats.insert(doc_format, output_format);
    Q_EMIT formatsChanged(m_formats);
}

/////////////////////////////////////////////////////////////////////
/// Options Page
/////////////////////////////////////////////////////////////////////

FEPageOptions::FEPageOptions(QWidget *parent)
    : QWizardPage(parent)
    , isPandocAvailable(false)
{
    setTitle(tr("Options"));
    setSubTitle(tr("Please choose exporting options."));

    m_outputFolder = new DirLineEdit;
    m_outputFolder->setLabelText(tr("Output Folder:"));
    registerField("outputFolder*", m_outputFolder->lineEdit());

    m_pandocExe = new FileLineEdit;
    m_pandocExe->setLabelText(tr("Pandoc Program:"));
    registerField("pandocExe", m_pandocExe->lineEdit());

    m_keepFolder = new QCheckBox;
    m_keepFolder->setChecked(true);
    m_keepFolder->setText(tr("Keep document folder"));
    m_keepFolder->setToolTip(tr(
            "Ensure that the folder hierarchy "
            "of the output is consistent with that in the software"));
    registerField("keepFolder", m_keepFolder);

    m_compress = new QCheckBox;
    m_compress->setText(tr("Compress document folder"));
    m_compress->setToolTip(tr("Compress each note's file and its associated "
                              "resources together"));
    registerField("compress", m_compress);

    m_exportMetainfo = new QCheckBox;
    m_exportMetainfo->setChecked(true);
    m_exportMetainfo->setText(tr("Export meta information"));
    m_exportMetainfo->setToolTip(tr("Export meta information of notes in JSON format"));
    registerField("metainformation", m_exportMetainfo);

    m_noTitleFolderIfPossible = new QCheckBox;
    m_noTitleFolderIfPossible->setText(tr("Don't create title folder if possible"));
    m_noTitleFolderIfPossible->setToolTip(tr(
        "Generally, notes are associated with many resource files, such \n"
        "as images, CSS files, and attachments, which are referenced in \n"
        "the text of the note. So it is necessary to create a title folder \n"
        "to place these closely related files together."));
    registerField("noTitleFolderIfPossible", m_noTitleFolderIfPossible);

    m_handleRichTextInMarkdown = new QCheckBox;
    m_handleRichTextInMarkdown->setText(tr("Handle rich-text in Markdown"));
    m_handleRichTextInMarkdown->setToolTip(tr(
        "The internal editor of WizNotePlus allows users to insert rich text\n"
        "images and tables inside Markdown notes. Turning this option on \n"
        "will cause the exporter to attempt to convert the rich text content\n"
        "mixed in the note into Markdown markup."));
    registerField("handleRichTextInMarkdown", m_handleRichTextInMarkdown);

    m_convertRichTextToMarkdown = new QCheckBox;
    m_convertRichTextToMarkdown->setText(tr("Convert rich-text notes into Markdown"));
    m_convertRichTextToMarkdown->setToolTip(tr(
        "Default notes created by WizNotePlus are in rich-text format, \n"
        "check this option to convert these HTML notes to Markdown format \n"
        "during export."));
    registerField("convertRichTextToMarkdown", m_convertRichTextToMarkdown);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_outputFolder);
    layout->addWidget(m_pandocExe);
    layout->addWidget(m_keepFolder);
    layout->addWidget(m_compress);
    layout->addWidget(m_exportMetainfo);
    layout->addWidget(m_noTitleFolderIfPossible);
    layout->addWidget(m_handleRichTextInMarkdown);
    layout->addWidget(m_convertRichTextToMarkdown);
    setLayout(layout);
}

void FEPageOptions::handlePandocExeSet(const QString &location)
{
    if (location.isEmpty()) {
        isPandocAvailable = false;
        Q_EMIT QWizardPage::completeChanged();
        return;
    }

    Utils::PandocWrapper pandoc(location);
    m_pandocExe->lineEdit()->setPlaceholderText("");

    if (pandoc.isAvailable()) {
        isPandocAvailable = true;
        WizSettings wizSettings(Utils::WizPathResolve::globalSettingsFile());
        wizSettings.setString("Pandoc", "ExeLocation", location);
    } else {
        QMessageBox::critical(this, tr("Can't find Pandoc"), pandoc.errorMessage());
        isPandocAvailable = false;
    }

    Q_EMIT QWizardPage::completeChanged();
}

void FEPageOptions::initializePage()
{
    WizSettings wizSettings(Utils::WizPathResolve::globalSettingsFile());
    QString location = wizSettings.getString("Pandoc", "ExeLocation");

    if (!location.isEmpty()) {
        m_pandocExe->lineEdit()->setText(location);
    } else {
        location = "pandoc";
    }

    Utils::PandocWrapper pandoc(location);

    if (pandoc.isAvailable()) {
        m_pandocExe->lineEdit()->setPlaceholderText("Found pandoc " + pandoc.version());
        isPandocAvailable = true;
    } else {
        m_pandocExe->lineEdit()->clear();
        m_pandocExe->lineEdit()->setPlaceholderText(pandoc.errorMessage());
        isPandocAvailable = false;
    }

    connect(m_pandocExe->lineEdit(), &QLineEdit::textChanged,
            this, &FEPageOptions::handlePandocExeSet);
}

bool FEPageOptions::isComplete() const
{
    if (!isPandocAvailable)
        return false;

    return QWizardPage::isComplete();
}

/////////////////////////////////////////////////////////////////////
/// Export Page
/////////////////////////////////////////////////////////////////////

FEPageExport::FEPageExport(WizExplorerApp& app, QWidget *parent)
    : QWizardPage(parent)
    , m_app(app)
    , m_dbMgr(app.databaseManager())
    , m_cancel(false)
{
    setTitle(tr("Exporting"));
    setFinalPage(true);

    m_exporter = new WizFileExporter(m_dbMgr, this);
    m_progress = new QProgressBar(this);
    m_details = new QTextEdit(this);
    m_details->setReadOnly(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_progress);
    layout->addWidget(m_details);
    setLayout(layout);

    connect(this, &FEPageExport::initialized,
            this, &FEPageExport::handleExportFile,
            Qt::QueuedConnection);
}

void FEPageExport::initializePage()
{
    connect(wizard(), &QWizard::finished,
            this, &FEPageExport::handleCancelBtnClicked);
    Q_EMIT initialized();
}

void FEPageExport::insertLog(const QString& text)
{
    QTextCursor cursor = m_details->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(text);
}


#define ASKCONTINUE(x, y) \
    QMessageBox::StandardButton ret = QMessageBox::critical(    \
        this, (x), (y),                                         \
        QMessageBox::Ok | QMessageBox::Abort,                   \
        QMessageBox::Ok                                         \
    );                                                          \
    if (ret == QMessageBox::Abort)                              \
        return;                                                 \
    else                                                        \
        continue                                               \

void FEPageExport::handleExportFile()
{
    auto notes = field("documents").toList();

    auto outputFormats = field("outputFormats").value<QMap<QString, QString> >();
    QString outputFolder = field("outputFolder").toString();
    bool keepFolder = field("keepFolder").toBool();

    m_exporter->setCompress(field("compress").toBool());
    m_exporter->setExportMetaInfo(field("metainformation").toBool());
    m_exporter->setNoTitleFolderIfPossible(field("noTitleFolderIfPossible").toBool());
    m_exporter->setHandleRichTextInMarkdown(field("handleRichTextInMarkdown").toBool());
    m_exporter->setConvertRichTextToMarkdown(field("convertRichTextToMarkdown").toBool());

    m_progress->setRange(0, notes.size());
    m_progress->setValue(0);

    foreach (const QVariant &note, notes) {
        m_progress->setValue(m_progress->value() + 1);
        qApp->processEvents();
        if (m_cancel) return;

        auto data = note.value<const WIZDOCUMENTDATA*>();
        insertLog(QString("Exporting %1\n").arg(data->strTitle));

        // Get the corresponding databaset for a document
        WizDatabase& db = m_dbMgr.db(data->strKbGUID);

        if (data->nProtected == 1) {
            if (!db.loadUserCert()) {
                ASKCONTINUE(
                    tr("Do you want to continue?"),
                    tr("Can't load user cert.")
                );
            }

            if (db.getCertPassword().isEmpty()) {
                insertLog("Asking for password...\n");
                bool ok;
                QString password = QInputDialog::getText(
                    this, tr("Password for Encrypted Notes"),
                    tr("Password:"), QLineEdit::Password, "", &ok);
                if (ok && !password.isEmpty()) {
                    if (!db.verifyCertPassword(password)) {
                        ASKCONTINUE(
                            tr("Do you want to continue?"),
                            tr("Invalid password.")
                        );
                    }
                } else {
                    ASKCONTINUE(
                        tr("Do you want to continue?"),
                        tr("Can't get password.")
                    );
                }

            }
        }

        QString destFolder = outputFolder;
        if (keepFolder)
            destFolder = outputFolder + data->strLocation;

        auto format = WizFileExporter::HTML;
        if (WizIsMarkdownNote(*data))
            format = WizFileExporter::Markdown;

        bool ok = m_exporter->exportNote(
            *data, destFolder, format);
        if (!ok) {
            ASKCONTINUE(
                tr("Do you want to continue?"),
                m_exporter->errorMessage()
            );
        }
    }

    Q_EMIT completeChanged();
    QDesktopServices::openUrl(QUrl(outputFolder));
}

bool FEPageExport::isComplete() const
{
    if (m_progress->value() != m_progress->maximum()) {
        return false;
    } else {
        return true;
    }
}



