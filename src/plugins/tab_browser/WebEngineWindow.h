#ifndef WEBENGINEWINDOW_H
#define WEBENGINEWINDOW_H

#include "share/WizWebEngineView.h"

class WizWebEngineView;

class WebEngineWindow : public QWidget
{
    Q_OBJECT
private:
    WizWebEngineView* m_web;
    virtual QSize sizeHint() const;

public:
    WebEngineWindow(QWidget *parent = nullptr);
    WizWebEngineView* webView();

};

#endif // WEBENGINEWINDOW_H
