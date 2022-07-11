#ifndef DOWNLOADMANAGERWIDGET_H
#define DOWNLOADMANAGERWIDGET_H

#include "ui_DownloadManagerWidget.h"
#include "ui_DownloadWidget.h"

#include <QWidget>
#include <QFrame>
#include <QElapsedTimer>
#include <QIcon>

class QWebEngineDownloadItem;

// Displays one ongoing or finished download (QWebEngineDownloadItem).
class DownloadWidget final : public QFrame, public Ui::DownloadWidget
{
    Q_OBJECT

public:
    // Precondition: The QWebEngineDownloadItem has been accepted.
    explicit DownloadWidget(QWebEngineDownloadItem *download, QWidget *parent = nullptr);

signals:
    // This signal is emitted when the user indicates that they want to remove
    // this download from the downloads list.
    void removeClicked(DownloadWidget *self);

private slots:
    void updateWidget();

private:
    QString withUnit(qreal bytes);

    QWebEngineDownloadItem *m_download;
    QElapsedTimer m_timeAdded;
};

// Displays a list of downloads.
class DownloadManagerWidget final : public QWidget, public Ui::DownloadManagerWidget
{
    Q_OBJECT

public:
    explicit DownloadManagerWidget(QWidget *parent = nullptr);

    static DownloadManagerWidget& instance() {
        static DownloadManagerWidget instance;
        return instance;
    }

    DownloadManagerWidget(DownloadManagerWidget const &) = delete;
    void operator=(DownloadManagerWidget const &)  = delete;

    // Prompts user with a "Save As" dialog. If the user doesn't cancel it, then
    // the QWebEngineDownloadItem will be accepted and the DownloadManagerWidget
    // will be shown on the screen.
    void downloadRequested(QWebEngineDownloadItem *webItem);

private:
    void add(DownloadWidget *downloadWidget);
    void remove(DownloadWidget *downloadWidget);

    int m_numDownloads;
};

#endif // DOWNLOADMANAGERWIDGET_H
