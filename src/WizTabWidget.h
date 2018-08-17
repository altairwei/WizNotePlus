#ifndef WIZTABWIDGET_H
#define WIZTABWIDGET_H

#include <QTabWidget>

QT_BEGIN_NAMESPACE
class QUrl;
QT_END_NAMESPACE

class WizDocumentView;
class WizDocumentWebView;
class WizWebEngineView;

class WizTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    WizTabWidget(QWidget *parent = nullptr);

    WizDocumentView* currentDocView();
    WizDocumentWebView* currentWebView();

signals:
    void titleChanged(const QString &title);
    void urlChanged(const QUrl &url);
    void loadProgress(int progress);
    void linkHovered(const QString &link);

private slots:
    void handleCurrentChanged(int index);
    void createTab(WizDocumentView *docView);
    void createTab(QUrl &url);

private:
    void setupView(WizWebEngineView *webView);
};

#endif // WIZTABWIDGET_H
