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

    ui->tableResources->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableResources->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableResources->setColumnCount(4);
    ui->tableResources->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableResources->horizontalHeader()->setStretchLastSection(true);
    ui->tableResources->setHorizontalHeaderLabels({tr("Name"), tr("Size"), tr("Time"), tr("Url")});

    std::vector<RESDATA> resources;
    Json::Value json;
    if (getDocResourceObjectListFromServer(m_doc, resources, &json)) {
        ui->tableResources->setRowCount(resources.size());
        for (int i = 0; i < resources.size(); i++) {
            QTableWidgetItem *nameItem = new QTableWidgetItem(resources[i].name);
            ui->tableResources->setItem(i, 0, nameItem);
            QTableWidgetItem *sizeItem = new QTableWidgetItem(locale().formattedDataSize(resources[i].size));
            ui->tableResources->setItem(i, 1, sizeItem);
            QTableWidgetItem *timeItem = new QTableWidgetItem(resources[i].time.toString());
            ui->tableResources->setItem(i, 2, timeItem);
            QTableWidgetItem *urlItem = new QTableWidgetItem(resources[i].url);
            ui->tableResources->setItem(i, 3, urlItem);
        }
    }

    Json::StreamWriterBuilder wbuilder;
    wbuilder["indentation"] = "    ";
    QString jsonText = QString::fromStdString(Json::writeString(wbuilder, json));
    //ui->textJson->setPlainText(jsonText);
    //ui->textJson->setLineWrapMode(QPlainTextEdit::NoWrap);
}

DocumentResourcesDialog::~DocumentResourcesDialog()
{
    delete ui;
}

bool DocumentResourcesDialog::getDocResourceObjectListFromServer(
        const WIZDOCUMENTDATA &doc, std::vector<RESDATA> &res, Json::Value *json)
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

    Json::Value resourcesObj = response["resources"];
    if (!resourcesObj.isArray())
        return false;

    if (json)
        *json = resourcesObj;

    int resourceCount = resourcesObj.size();
    for (int i = 0; i < resourceCount; i++)
    {
        Json::Value resObj = resourcesObj[i];
        RESDATA data;
        data.name = QString::fromUtf8(resObj["name"].asString().c_str());
        data.url = QString::fromUtf8(resObj["url"].asString().c_str());
        data.size = atoi((resObj["size"].asString().c_str()));
        data.time = QDateTime::fromTime_t(resObj["time"].asInt64() / 1000);
        res.push_back(data);
    }

    return true;
}
