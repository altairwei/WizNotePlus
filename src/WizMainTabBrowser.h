#ifndef WIZMAINTABBROWSER_H
#define WIZMAINTABBROWSER_H

#include <QTabWidget>
#include <QAbstractButton>
#include "share/WizWebEngineView.h"
#include "WizDocumentWebView.h"

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

class WizExplorerApp;
class WizDatabaseManager;
class WIZDOCUMENTDATA;
class WizDocumentView;
class WizDocumentWebView;
class WizWebsiteView;
class QStyleOptionButton;
class QStyleOptionTabBarBase;

class TabButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit TabButton(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override
        { return sizeHint(); }
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void drawTabBtn(const QStyleOptionButton *option, QPainter *painter, const QWidget *widget = nullptr) const;
};

class WizMainTabBrowser : public QTabWidget
{
    Q_OBJECT

public:

    typedef QMap<QString, QVariant> TabStatusData;

    WizMainTabBrowser(WizExplorerApp& app, QWidget *parent = nullptr);

    //WizDocumentView* currentDocView();
    WizWebEngineView* currentWebView() const;
    WizWebEngineView* getWebView(int index) const;

private:
    WizExplorerApp& m_app;
    WizDatabaseManager& m_dbMgr;
    QString m_strTheme;

signals:
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
    void loadProgress(int progress);
    void linkHovered(const QString &link);

public slots:
    void handleCurrentChanged(int index);
    void handleContextMenuRequested(const QPoint &pos);
    int createTab(WizDocumentView *docView);
    int createTab(const QUrl &url);
    void closeTab(int index);
    void closeOtherTabs(int index);
    void closeLeftTabs(int index);
    void closeRightTabs(int index);
    void lockTab(int index);
    void unlockTab(int index);
    void onViewNoteRequested(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool forceEditing);
    void on_document_deleted(const WIZDOCUMENTDATA&);

private:
    void setupView(WizWebEngineView* view);
    void setupWebsiteView(WizWebsiteView *websiteView);
    void setupDocView(WizDocumentView *docView);
    void setupTab(QWidget* wgt);
    void paintEvent(QPaintEvent *);
    void initStyleBaseOption(QStyleOptionTabBarBase *optTabBase, QTabBar *tabbar, QSize size);
};

#endif // WIZTABWIDGET_H
