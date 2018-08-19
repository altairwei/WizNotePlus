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

class WizDevToolsDialog : public QWidget
{
    Q_OBJECT
private:
    WizWebEngineView* m_web;
    virtual QSize sizeHint() const;
public:
    WizDevToolsDialog(QWidget *parent = nullptr);
    WizWebEngineView* getWeb();


};

#endif // WIZDEVTOOLSDIALOG_H
