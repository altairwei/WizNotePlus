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
    struct RESINFO
    {
        QString name;
        QDateTime time;
        size_t size;
        QString url;
        enum FLAG {
            Recorded = 1 << 0,
            Referenced = 1 << 1,
            Cached = 1 << 2,
            Inserver = 1 << 3
        };
        int status;
    };

    bool getDocumentInfoFromServer();
    bool getRecordedResObjectList();
    bool getReferencedResObjectList();

private Q_SLOTS:
    void handleBtnDeleteInServerClicked();
    void handleBtnDownloadClicked();

private:
    Ui::DocumentResourcesDialog *ui;
    WIZDOCUMENTDATA m_doc;
    Json::Value m_json;
    std::vector<RESINFO> m_resTotal;
    std::vector<RESINFO> m_resToDelete;
};

#endif // DOCUMENTRESOURCESDIALOG_H
