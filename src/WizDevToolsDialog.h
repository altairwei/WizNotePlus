#ifndef WIZDEVTOOLSDIALOG_H
#define WIZDEVTOOLSDIALOG_H

#include <QDialog>
#include "share/WizWebEngineView.h"

class QUrl;
class QLabel;
class QMovie;
class WizWebEngineView;
class QPushButton;
class QNetworkReply;
class WizLocalProgressWebView;

class WizDevToolsDialog : public WizWebEngineViewContainerDialog
{
    Q_OBJECT
private:
    WizWebEngineView* m_web;
    virtual QSize sizeHint() const;
public:
    WizDevToolsDialog(QWidget *parent = 0);
    WizWebEngineView* getWeb();


};

#endif // WIZDEVTOOLSDIALOG_H
