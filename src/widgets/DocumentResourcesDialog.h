#ifndef DOCUMENTRESOURCESDIALOG_H
#define DOCUMENTRESOURCESDIALOG_H

#include <QDialog>
#include <vector>

#include "share/WizObject.h"
#include "share/jsoncpp/json/json.h"

namespace Ui {
class DocumentResourcesDialog;
}

class DocumentResourcesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DocumentResourcesDialog(const WIZDOCUMENTDATA &doc, QWidget *parent = nullptr);
    ~DocumentResourcesDialog();

private:
    struct RESDATA
    {
        QString name;
        QDateTime time;
        size_t size;
        QString url;
    };

    bool getDocResourceObjectListFromServer();

private Q_SLOTS:
    void handleBtnDeleteInServerClicked();

private:
    Ui::DocumentResourcesDialog *ui;
    WIZDOCUMENTDATA m_doc;
    Json::Value m_json;
    std::vector<RESDATA> m_resInServer;
    std::vector<RESDATA> m_resToDelete;
};

#endif // DOCUMENTRESOURCESDIALOG_H
