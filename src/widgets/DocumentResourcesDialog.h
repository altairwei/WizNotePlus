#ifndef DOCUMENTRESOURCESDIALOG_H
#define DOCUMENTRESOURCESDIALOG_H

#include <QDialog>
#include <vector>

#include "share/WizObject.h"
#include "share/jsoncpp/json/json-forwards.h"

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

    bool getDocResourceObjectListFromServer(const WIZDOCUMENTDATA &doc, std::vector<RESDATA> &res, Json::Value *json = nullptr);

private:
    Ui::DocumentResourcesDialog *ui;
    WIZDOCUMENTDATA m_doc;
};

#endif // DOCUMENTRESOURCESDIALOG_H
