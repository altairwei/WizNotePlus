#ifndef WIZMAINTABWIDGET_H
#define WIZMAINTABWIDGET_H

#include <QTabWidget>
#include <QAbstractButton>
#include "share/WizWebEngineView.h"
#include "WizDocumentWebView.h"

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

class WizExplorerApp;
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

class WizMainTabWidget : public QTabWidget
{
    Q_OBJECT

public:

    typedef QMap<QString, QVariant> TabStatusData;

    WizMainTabWidget(WizExplorerApp& app, QWidget *parent = nullptr);

    //WizDocumentView* currentDocView();
    WizWebEngineView* currentWebView() const;
    WizWebEngineView* getWebView(int index) const;

private:
    WizExplorerApp& m_app;
    QString m_strTheme;

signals:
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
    void loadProgress(int progress);
    void linkHovered(const QString &link);

public slots:
    void handleCurrentChanged(int index);
    void handleContextMenuRequested(const QPoint &pos);
    void createTab(WizDocumentView *docView);
    void createTab(const QUrl &url);
    void closeTab(int index);
    void lockTab(int index);
    void unlockTab(int index);
    void onViewNoteRequested(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool forceEditing);
    void setTabTextToDocumentTitle(WizDocumentView* view, const WIZDOCUMENTDATAEX& doc, bool forceEditing);
    void setTabTextToDocumentTitle(QString strGUID, WizDocumentView* view);
    void setTabTextToDocumentTitle(WizDocumentView*, QString newTitle);
private:
    void setupView(WizWebsiteView *websiteView);
    void setupTab(QWidget* wgt);
    void paintEvent(QPaintEvent *);
    void initStyleBaseOption(QStyleOptionTabBarBase *optTabBase, QTabBar *tabbar, QSize size);
};

#endif // WIZTABWIDGET_H
