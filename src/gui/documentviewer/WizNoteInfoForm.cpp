#include "WizNoteInfoForm.h"
#include "ui_WizNoteInfoForm.h"

#include <QFile>

#include "share/WizObject.h"
#include "database/WizDatabaseManager.h"
#include "database/WizDatabase.h"
#include "utils/WizLogger.h"
#include "WizMainWindow.h"
#include "share/WizAnalyzer.h"
#include "share/WizMessageBox.h"

QString formatLabelLink(const QString& linkHref, const QString& text)
{
    return WizFormatString2("<a href=\"%1\" style=\"color:#5990EF;"
                    "text-decoration:none;\">%2</a>", linkHref, text);
}

WizNoteInfoForm::WizNoteInfoForm(QWidget *parent)
    : WizPopupWidget(parent)
    , ui(new Ui::WizNoteInfoForm)
    , m_size(QSize(340, 420))
{
    ui->setupUi(this);

    ui->editCreateTime->setReadOnly(true);
    ui->editUpdateTime->setReadOnly(true);
    ui->editAccessTime->setReadOnly(true);
    ui->labelNotebook->setReadOnly(true);

    QString openDocument = formatLabelLink("locate", tr("Locate"));
    ui->labelOpenDocument->setText(openDocument);
    QString versionHistory = formatLabelLink("history", tr("Click to view version history"));
    ui->labelHistory->setText(versionHistory);

    setFixedSize(m_size);
}

WizNoteInfoForm::~WizNoteInfoForm()
{
    delete ui;
}

QSize WizNoteInfoForm::sizeHint() const
{
    return m_size;
}

void WizNoteInfoForm::hideEvent(QHideEvent* ev)
{
    QWidget::hideEvent(ev);

    emit widgetStatusChanged();
}

#if defined(Q_OS_WIN)
static const int kNoteInforFormLineHeight = 30;
#elif defined(Q_OS_MAC)
static const int kNoteInforFormLineHeight = 26;
#else
static const int kNoteInforFormLineHeight = 26;
#endif

void WizNoteInfoForm::setDocument(const WIZDOCUMENTDATA& data)
{
    Q_ASSERT(!data.strKbGUID.isEmpty());

    // Use lineCount to calculate window height
    int lineCount = 0;

    m_docKbGuid = data.strKbGUID;
    m_docGuid = data.strGUID;

    WizDatabase& db = WizDatabaseManager::instance()->db(data.strKbGUID);

    const bool isGroupNote = db.isGroup();
    const bool canEdit = (db.canEditDocument(data) && !WizDatabase::isInDeletedItems(data.strLocation));

    QString doc = db.getDocumentFileName(data.strGUID);
    QString sz = ::WizGetFileSizeHumanReadalbe(doc);
    m_sizeText = sz;

    QFont font;
    QFontMetrics fm(font);
    const int nMaxTextWidth = 280;

    // Location
    QString strLocation = db.getDocumentLocation(data);
    ui->labelNotebook->setText(strLocation);
    lineCount++;

    // Owner
    if (isGroupNote) {
        QString userAlias = db.getDocumentOwnerAlias(data);
        ui->labelOwner->setText(userAlias);
        lineCount++;
    } else {
        ui->labelOwner->setVisible(false);
        ui->labelOwnerLabel->setVisible(false);
    }

    // Tags line
    if (data.strKbGUID == WizDatabaseManager::instance()->db().kbGUID()) {
        // private document
        QString tags = db.getDocumentTagsText(data.strGUID);
        tags = fm.elidedText(tags, Qt::ElideMiddle, nMaxTextWidth);
        if (!tags.isEmpty()) {
            ui->labelTags->setText(tags);
            lineCount++;
        } else {
            ui->labelTags->setVisible(false);
            ui->labelTagsLabel->setVisible(false);
        }
    } else {
        // group document
        ui->labelTags->clear();
        ui->labelTags->setVisible(false);
        ui->labelTagsLabel->setVisible(false);
    }

    // URL
    ui->editURL->setText(data.strURL);
    ui->editURL->setReadOnly(!canEdit);
    QString text = data.strURL.isEmpty() ? "" : formatLabelLink(data.strURL, tr("Open"));
    ui->labelOpenURL->setText(text);
    lineCount++;

    // Times
    ui->editCreateTime->setText(data.tCreated.toString());
    ui->editUpdateTime->setText(data.tDataModified.toString());
    ui->editAccessTime->setText(data.tAccessed.toString());
    lineCount += 3;

    // Author line
    ui->editAuthor->setText(data.strAuthor);
    ui->editAuthor->setReadOnly(!canEdit);
    lineCount++;

    // Note Type
    if (!data.strType.isEmpty()) {
        ui->labelNoteType->setText(data.strType);
        lineCount++;
    } else {
        ui->labelNoteTypeLabel->setVisible(false);
        ui->labelNoteType->setVisible(false);
    }

    // File Type
    if (!data.strFileType.isEmpty()) {
        ui->labelFileType->setText(data.strFileType);
        lineCount++;
    } else {
        ui->labelFileTypeLabel->setVisible(false);
        ui->labelFileType->setVisible(false);
    }

    // Size
    ui->labelSize->setText(sz);
    lineCount += 6;

    // Encrypt
    if (!isGroupNote) {
        ui->checkEncrypted->setChecked(data.nProtected ? true : false);
        ui->checkEncrypted->setEnabled(canEdit && !db.isGroup());
        lineCount++;
    } else {
        ui->labelEncrypted->setVisible(false);
        ui->checkEncrypted->setVisible(false);
    }

    // History
    lineCount++;

    m_size = QSize(340, lineCount * kNoteInforFormLineHeight);
    setFixedSize(m_size);
}

void WizNoteInfoForm::setWordCount(int nWords, int nChars, int nCharsWithSpace, int nNonAsianWords, int nAsianChars)
{
    QString textFormat = tr(
        "Words: %1\n"
        "Characters (no spaces): %2\n"
        "Characters (with spaces): %3\n"
        "Non-Asianwords: %4\n"
        "Asian characters: %5"
    );

    QString text = textFormat.arg(
        QString::number(nWords),
        QString::number(nChars),
        QString::number(nCharsWithSpace),
        QString::number(nNonAsianWords),
        QString::number(nAsianChars));

    ui->labelSize->setText(m_sizeText + "\n" + text);
}

void WizNoteInfoForm::on_labelOpenDocument_linkActivated(const QString &link)
{
    Q_UNUSED(link);

    WizMainWindow *mainWindow = WizMainWindow::instance();
    if (mainWindow)
    {
        mainWindow->locateDocument(m_docKbGuid, m_docGuid);
        hide();
    }
}

void WizNoteInfoForm::on_editURL_editingFinished()
{
    WIZDOCUMENTDATA doc;
    WizDatabase& db = WizDatabaseManager::instance()->db(m_docKbGuid);
    if (db.documentFromGuid(m_docGuid, doc))
    {
        if (doc.strURL != ui->editURL->text())
        {
            doc.strURL= ui->editURL->text();
            db.modifyDocumentInfo(doc);
        }
    }
}

void WizNoteInfoForm::on_editAuthor_editingFinished()
{
    WIZDOCUMENTDATA doc;
    WizDatabase& db = WizDatabaseManager::instance()->db(m_docKbGuid);
    if (db.documentFromGuid(m_docGuid, doc))
    {
        if (doc.strAuthor != ui->editAuthor->text())
        {
            doc.strAuthor = ui->editAuthor->text();
            db.modifyDocumentInfo(doc);
        }
    }
}

void WizNoteInfoForm::on_checkEncrypted_clicked(bool checked)
{
    WIZDOCUMENTDATA doc;
    WizDatabase& db = WizDatabaseManager::instance()->db(m_docKbGuid);
    if (db.documentFromGuid(m_docGuid, doc))
    {
        if (checked)
        {
            db.encryptDocument(doc);
        }
        else
        {
            if (doc.nProtected)
            {
                if (!db.cancelDocumentEncryption(doc))
                    return;
            }
        }
    }
}

void WizNoteInfoForm::on_labelHistory_linkActivated(const QString &link)
{
    Q_UNUSED(link);

    WIZDOCUMENTDATA doc;
    WizDatabase& db = WizDatabaseManager::instance()->db(m_docKbGuid);
    if (db.documentFromGuid(m_docGuid, doc))
    {
        WizShowDocumentHistory(doc, nullptr);
        WizGetAnalyzer().logAction("showVersionHistory");
    }
}

void WizNoteInfoForm::on_labelOpenURL_linkActivated(const QString &link)
{
    Q_UNUSED(link);

    QUrl url(link);
    if (url.isValid())
    {
        QDesktopServices::openUrl(url.toString());
    }
    else
    {
        WizMessageBox::information(nullptr, tr("Info"), tr("Url invalid, can not open!"));
    }
}
