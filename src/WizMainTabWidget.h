#ifndef WIZMAINTABWIDGET_H
#define WIZMAINTABWIDGET_H

#include <QTabWidget>
#include "share/WizWebEngineView.h"
#include "WizDocumentWebView.h"

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

class WizExplorerApp;
class WizDocumentView;
class WizDocumentWebView;
class WizWebsiteView;

class WizMainTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    WizMainTabWidget(WizExplorerApp& app, QWidget *parent = nullptr);

    //WizDocumentView* currentDocView();
    WizWebEngineView* currentWebView() const;
    WizWebEngineView* getWebView(int index) const;

private:
    WizExplorerApp& m_app;

signals:
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
    void loadProgress(int progress);
    void linkHovered(const QString &link);

public slots:
    void handleCurrentChanged(int index);
    void createTab(WizDocumentView *docView);
    void createTab(const QUrl &url);
    void closeTab(int index);
    void onViewNoteRequested(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool forceEditing);
    void setTabTextToDocumentTitle(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool forceEditing);
    void setTabTextToDocumentTitle(QString strGUID, WizDocumentView* view);
    void setTabTextToDocumentTitle(WizDocumentView*, QString newTitle);
private:
    void setupView(WizWebsiteView *websiteView);
};

#endif // WIZTABWIDGET_H
