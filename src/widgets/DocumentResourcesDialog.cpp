#include "DocumentResourcesDialog.h"
#include "ui_DocumentResourcesDialog.h"

#include <QUrl>
#include <QDir>
#include <algorithm>

#include "share/jsoncpp/json/json.h"

#include "sync/WizToken.h"
#include "sync/WizKMServer.h"
#include "share/WizRequest.h"
#include "utils/WizLogger.h"
#include "html/WizHtmlTool.h"

DocumentResourcesDialog::DocumentResourcesDialog(const WIZDOCUMENTDATA &doc, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DocumentResourcesDialog)
    , m_doc(doc)
{
    ui->setupUi(this);

    setWindowTitle(tr("Resources - %1").arg(m_doc.strTitle));

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(ui->btnDeleteInServer, &QPushButton::clicked, this, &DocumentResourcesDialog::handleBtnDeleteInServerClicked);
    connect(ui->btnDownload, &QPushButton::clicked, this, &DocumentResourcesDialog::handleBtnDownloadClicked);

    ui->tableResources->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableResources->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableResources->setColumnCount(6);
    ui->tableResources->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableResources->horizontalHeader()->setStretchLastSection(true);
    ui->tableResources->setHorizontalHeaderLabels({
        tr("Name"), tr("Recorded"), tr("Referenced"), tr("Size"), tr("Time"), tr("Url")});

    if (getDocumentInfoFromServer()) {
        getRecordedResObjectList();
        getReferencedResObjectList();
        ui->tableResources->setRowCount(m_resTotal.size());
        for (int i = 0; i < m_resTotal.size(); i++) {
            QTableWidgetItem *nameItem = new QTableWidgetItem(m_resTotal[i].name);
            ui->tableResources->setItem(i, 0, nameItem);
            QTableWidgetItem *recordItem = new QTableWidgetItem(
                m_resTotal[i].status & RESINFO::FLAG::Recorded ? "TRUE" : "FALSE");
            ui->tableResources->setItem(i, 1, recordItem);
            QTableWidgetItem *refItem = new QTableWidgetItem(
                m_resTotal[i].status & RESINFO::FLAG::Referenced ? "TRUE" : "FALSE");
            ui->tableResources->setItem(i, 2, refItem);
            QTableWidgetItem *sizeItem = new QTableWidgetItem(locale().formattedDataSize(m_resTotal[i].size));
            ui->tableResources->setItem(i, 3, sizeItem);
            QTableWidgetItem *timeItem = new QTableWidgetItem(m_resTotal[i].time.toString());
            ui->tableResources->setItem(i, 4, timeItem);
            QTableWidgetItem *urlItem = new QTableWidgetItem(m_resTotal[i].url);
            ui->tableResources->setItem(i, 5, urlItem);
        }

        Json::StreamWriterBuilder wbuilder;
        wbuilder["indentation"] = "    ";
        QString jsonText = QString::fromStdString(Json::writeString(wbuilder, m_json));
        ui->textJson->setPlainText(jsonText);
        ui->textJson->setLineWrapMode(QTextEdit::NoWrap);
    }
}

DocumentResourcesDialog::~DocumentResourcesDialog()
{
    delete ui;
}

bool DocumentResourcesDialog::getDocumentInfoFromServer()
{
    QString token = WizToken::token();
    if (token.isEmpty()) {
        return false;
    }

    WIZUSERINFO info;
    info = WizToken::userInfo();
    info.strToken = token;
    info.strKbGUID = m_doc.strKbGUID;

    WizKMDatabaseServer ksServer(info);
    QString url = ksServer.buildUrl(
        "/ks/note/download/" + info.strKbGUID + "/" + m_doc.strGUID + "?downloadData=1");

    Json::Value response;
    WIZSTANDARDRESULT jsonRet = WizRequest::execStandardJsonRequest(url, response);
    if (!jsonRet)
    {
        TOLOG1("Failed to download document data: %1", m_doc.strTitle);
        return false;
    }

    m_json = response;

    return true;
}

bool DocumentResourcesDialog::getRecordedResObjectList()
{
    if (m_json.isNull())
        return false;

    Json::Value resourcesObj = m_json["resources"];
    if (!resourcesObj.isArray())
        return false;

    int resourceCount = resourcesObj.size();
    for (int i = 0; i < resourceCount; i++)
    {
        Json::Value resObj = resourcesObj[i];
        RESINFO data;
        data.name = QString::fromUtf8(resObj["name"].asString().c_str());
        data.url = QString::fromUtf8(resObj["url"].asString().c_str());
        data.size = atoi((resObj["size"].asString().c_str()));
        data.time = QDateTime::fromTime_t(resObj["time"].asInt64() / 1000);
        data.status = data.status | RESINFO::FLAG::Inserver | RESINFO::FLAG::Recorded;
        m_resTotal.push_back(data);
    }

    return true;
}

bool DocumentResourcesDialog::getReferencedResObjectList()
{
    if (m_json.isNull())
        return false;

    auto html = QString::fromStdString(m_json["html"].asString());
    QStringList srcs = Utils::WizHtmlExtractAttrValues(html, "src");
    srcs.append(Utils::WizHtmlExtractAttrValues(html, "href"));
    foreach (const auto &src, srcs) {
        if (src.startsWith("index_files") || src.startsWith("./index_files")) {
            auto filename = QUrl(src).fileName();
            auto it = std::find_if(m_resTotal.begin(), m_resTotal.end(),
                [&filename](auto res) {
                    return res.name == filename;
                }
            );

            if (it != m_resTotal.end()) {
                auto &res = *it;
                res.status = res.status | RESINFO::FLAG::Referenced;
            } else {
                RESINFO newRes;
                newRes.name = filename;
                newRes.size = 0;
                newRes.status = RESINFO::FLAG::Referenced;
                m_resTotal.push_back(newRes);
            }
        }
    }

    return true;
}

void DocumentResourcesDialog::handleBtnDeleteInServerClicked()
{
    QStringList files;
    QItemSelectionModel* selection = ui->tableResources->selectionModel();
    if (selection->hasSelection())
    {
        for (QModelIndex& row : selection->selectedRows())
        {
            files << ui->tableResources->item(row.row(), 0)->text();
        }
    }

    ui->textJson->setText(files.join('\n'));

    QString token = WizToken::token();
    WIZUSERINFO info;
    info = WizToken::userInfo();
    info.strToken = token;
    info.strKbGUID = m_doc.strKbGUID;

    WizKMDatabaseServer ksServer(info);
    QString url = ksServer.buildUrl(
        "/ks/note/upload/" + info.strKbGUID + "/" + m_doc.strGUID);

    Json::Value doc;
    doc["kbGuid"] = info.strKbGUID.toStdString();
    doc["docGuid"] = m_doc.strGUID.toStdString();
    doc["title"] = m_doc.strTitle.toStdString();
    doc["dataMd5"] = m_doc.strDataMD5.toStdString();
    doc["dataModified"] = (Json::UInt64)m_doc.tDataModified.toTime_t() * (Json::UInt64)1000;
    doc["category"] = m_doc.strLocation.toStdString();
    doc["owner"] = m_doc.strOwner.toStdString();
    doc["protected"] = (int)m_doc.nProtected;
    doc["readCount"] = (int)m_doc.nReadCount;
    doc["attachmentCount"] = (int)m_doc.nAttachmentCount;
    doc["type"] = m_doc.strType.toStdString();
    doc["fileType"] = m_doc.strFileType.toStdString();
    doc["created"] = (Json::UInt64)m_doc.tCreated.toTime_t() * (Json::UInt64)1000;
    doc["accessed"] = (Json::UInt64)m_doc.tAccessed.toTime_t() * (Json::UInt64)1000;
    doc["url"] = m_doc.strURL.toStdString();
    doc["styleGuid"] = m_doc.strStyleGUID.toStdString();
    doc["seo"] = m_doc.strSEO.toStdString();
    doc["author"] = m_doc.strAuthor.toStdString();
    doc["keywords"] = m_doc.strKeywords.toStdString();
    doc["tags"] = m_json["info"]["tags"];
    doc["withData"] = true;
    doc["html"] = m_json["html"];

    Json::Value res_array(Json::arrayValue);
    for (const auto &res : m_resTotal) {
        if (files.contains(res.name))
            continue;
        Json::Value elemObj;
        elemObj["name"] = res.name.toStdString();
        elemObj["time"] = (Json::UInt64)res.time.toTime_t() * (Json::UInt64)1000;
        elemObj["size"] = (Json::UInt64)res.size;
        res_array.append(elemObj);

        doc["resources"] = res_array;
    }

    Json::Value ret;
    WIZSTANDARDRESULT jsonRet = WizRequest::execStandardJsonRequest(url, "POST", doc, ret);
    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "    ";
    QString jsonText = QString::fromStdString(Json::writeString(wbuilder, ret));
    ui->textJson->setText(jsonText);
}

void DocumentResourcesDialog::handleBtnDownloadClicked()
{
    QStringList files;
    QItemSelectionModel* selection = ui->tableResources->selectionModel();
    if (selection->hasSelection())
    {
        for (QModelIndex& row : selection->selectedRows())
        {
            files << ui->tableResources->item(row.row(), 0)->text();
        }
    }


}
