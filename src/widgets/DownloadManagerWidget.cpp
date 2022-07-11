/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "DownloadManagerWidget.h"

#include <QFileDialog>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include <QWebEngineDownloadItem>
#include <QWebEngineProfile>
#include <QMap>
#include <QDebug>

#include "utils/WizStyleHelper.h"
#include "share/WizMisc.h"

DownloadWidget::DownloadWidget(QWebEngineDownloadItem *download, QWidget *parent)
    : QFrame(parent)
    , m_download(download)
    , m_timeAdded()
{
    m_timeAdded.start();
    setupUi(this);
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    m_dstName->setText(m_download->downloadFileName());
#else
    m_dstName->setText(QFileInfo(m_download->path()).fileName());
#endif
    m_srcUrl->setText(m_download->url().toDisplayString());
    QIcon removeIcon(WizLoadSkinIcon(Utils::WizStyleHelper::themeName(), "trash"));
    m_cancelButton->setIcon(removeIcon);

    connect(m_cancelButton, &QPushButton::clicked,
            [this](bool) {
        if (m_download->state() == QWebEngineDownloadItem::DownloadInProgress)
            m_download->cancel();
        else
            emit removeClicked(this);
    });

    connect(m_download, &QWebEngineDownloadItem::downloadProgress,
            this, &DownloadWidget::updateWidget);

    connect(m_download, &QWebEngineDownloadItem::stateChanged,
            this, &DownloadWidget::updateWidget);

    updateWidget();
}

inline QString DownloadWidget::withUnit(qreal bytes)
{
    if (bytes < (1 << 10))
        return tr("%L1 B").arg(bytes);
    else if (bytes < (1 << 20))
        return tr("%L1 KiB").arg(bytes / (1 << 10), 0, 'f', 2);
    else if (bytes < (1 << 30))
        return tr("%L1 MiB").arg(bytes / (1 << 20), 0, 'f', 2);
    else
        return tr("%L1 GiB").arg(bytes / (1 << 30), 0, 'f', 2);
}

void DownloadWidget::updateWidget()
{
    qreal totalBytes = m_download->totalBytes();
    qreal receivedBytes = m_download->receivedBytes();
    qreal bytesPerSecond = receivedBytes / m_timeAdded.elapsed() * 1000;

    auto state = m_download->state();
    switch (state) {
    case QWebEngineDownloadItem::DownloadRequested:
        Q_UNREACHABLE();
        break;
    case QWebEngineDownloadItem::DownloadInProgress:
        if (totalBytes >= 0) {
            m_progressBar->setValue(qRound(100 * receivedBytes / totalBytes));
            m_progressBar->setDisabled(false);
            m_progressBar->setFormat(
                tr("%p% - %1 of %2 downloaded - %3/s")
                .arg(withUnit(receivedBytes))
                .arg(withUnit(totalBytes))
                .arg(withUnit(bytesPerSecond)));
        } else {
            m_progressBar->setValue(0);
            m_progressBar->setDisabled(false);
            m_progressBar->setFormat(
                tr("unknown size - %1 downloaded - %2/s")
                .arg(withUnit(receivedBytes))
                .arg(withUnit(bytesPerSecond)));
        }
        break;
    case QWebEngineDownloadItem::DownloadCompleted:
        m_progressBar->setValue(100);
        m_progressBar->setDisabled(true);
        m_progressBar->setFormat(
            tr("completed - %1 downloaded - %2/s")
            .arg(withUnit(receivedBytes))
            .arg(withUnit(bytesPerSecond)));
        break;
    case QWebEngineDownloadItem::DownloadCancelled:
        m_progressBar->setValue(0);
        m_progressBar->setDisabled(true);
        m_progressBar->setFormat(
            tr("cancelled - %1 downloaded - %2/s")
            .arg(withUnit(receivedBytes))
            .arg(withUnit(bytesPerSecond)));
        break;
    case QWebEngineDownloadItem::DownloadInterrupted:
        m_progressBar->setValue(0);
        m_progressBar->setDisabled(true);
        m_progressBar->setFormat(
            tr("interrupted: %1")
            .arg(m_download->interruptReasonString()));
        break;
    }

    if (state == QWebEngineDownloadItem::DownloadInProgress) {
        m_cancelButton->setToolTip(tr("Stop downloading"));
    } else {
        m_cancelButton->setToolTip(tr("Remove from list"));
    }
}


DownloadManagerWidget::DownloadManagerWidget(QWidget *parent)
    : QWidget(parent)
    , m_numDownloads(0)
{
    setupUi(this);

    // Quit application if the download manager window is the only remaining window
    setAttribute(Qt::WA_QuitOnClose, false);
}

void DownloadManagerWidget::downloadRequested(QWebEngineDownloadItem *download)
{
    Q_ASSERT(download && download->state() == QWebEngineDownloadItem::DownloadRequested);

    QString filter;
    QString selectedFilter;

    static QStringList html_filters {
        tr("MIME HTML (*.mht *.mhtml)"),
        tr("Single HTML without resources (*.html)"),
        tr("Complete HTML (*.html)")
    };

    static QMap<QString, QWebEngineDownloadItem::SavePageFormat> formats = {
        {html_filters[0], QWebEngineDownloadItem::MimeHtmlSaveFormat},
        {html_filters[1], QWebEngineDownloadItem::SingleHtmlSaveFormat},
        {html_filters[2], QWebEngineDownloadItem::CompleteHtmlSaveFormat}
    };

    static QString savePageFilter = html_filters.join(";;");

    if (download->isSavePageDownload())
        filter = savePageFilter;

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QString path = QFileDialog::getSaveFileName(
        this, tr("Save as"),
        QDir(download->downloadDirectory()).filePath(download->downloadFileName()),
        filter, &selectedFilter);
#else
    QString path = QFileDialog::getSaveFileName(
        this, tr("Save as"), download->path(), filter, &selectedFilter);
#endif

    if (path.isEmpty())
        return;

    if (download->isSavePageDownload())
        download->setSavePageFormat(formats[selectedFilter]);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    download->setDownloadDirectory(QFileInfo(path).path());
    download->setDownloadFileName(QFileInfo(path).fileName());
#else
    download->setPath(path);
#endif
    download->accept();
    add(new DownloadWidget(download));

    show();
}

void DownloadManagerWidget::add(DownloadWidget *downloadWidget)
{
    connect(downloadWidget, &DownloadWidget::removeClicked, this, &DownloadManagerWidget::remove);
    m_itemsLayout->insertWidget(0, downloadWidget, 0, Qt::AlignTop);
    if (m_numDownloads++ == 0)
        m_zeroItemsLabel->hide();
}

void DownloadManagerWidget::remove(DownloadWidget *downloadWidget)
{
    m_itemsLayout->removeWidget(downloadWidget);
    downloadWidget->deleteLater();
    if (--m_numDownloads == 0)
        m_zeroItemsLabel->show();
}

