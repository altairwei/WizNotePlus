#include "DocumentResourcesDialog.h"
#include "ui_DocumentResourcesDialog.h"

#include "share/jsoncpp/json/json.h"

#include "sync/WizToken.h"
#include "sync/WizKMServer.h"
#include "share/WizRequest.h"
#include "utils/WizLogger.h"

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

    ui->tableResources->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableResources->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableResources->setColumnCount(4);
    ui->tableResources->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableResources->horizontalHeader()->setStretchLastSection(true);
    ui->tableResources->setHorizontalHeaderLabels({tr("Name"), tr("Size"), tr("Time"), tr("Url")});

    if (getDocResourceObjectListFromServer()) {
        ui->tableResources->setRowCount(m_resInServer.size());
        for (int i = 0; i < m_resInServer.size(); i++) {
            QTableWidgetItem *nameItem = new QTableWidgetItem(m_resInServer[i].name);
            ui->tableResources->setItem(i, 0, nameItem);
            QTableWidgetItem *sizeItem = new QTableWidgetItem(locale().formattedDataSize(m_resInServer[i].size));
            ui->tableResources->setItem(i, 1, sizeItem);
            QTableWidgetItem *timeItem = new QTableWidgetItem(m_resInServer[i].time.toString());
            ui->tableResources->setItem(i, 2, timeItem);
            QTableWidgetItem *urlItem = new QTableWidgetItem(m_resInServer[i].url);
            ui->tableResources->setItem(i, 3, urlItem);
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

bool DocumentResourcesDialog::getDocResourceObjectListFromServer()
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

    Json::Value resourcesObj = response["resources"];
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
        m_resInServer.push_back(data);
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
    for (const auto &res : m_resInServer) {
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
