#include "WizCommonUI.h"

#include <QClipboard>
#include <QApplication>
#include <QUuid>
#include <QDir>
#include <QDebug>
#include <QByteArray>
#include <QSaveFile>
#include <QDesktopServices>
#include <QUrl>
#include <QFileDialog>
#include <QProcess>
#include <QInputDialog>
#include <QMessageBox>

#include "WizMisc.h"
#include "utils/WizPathResolve.h"
#include "share/WizMisc.h"
#include "share/WizSettings.h"
#include "api/QtClassWrapper.h"


WizCommonUI::WizCommonUI(QObject* parent)
    : QObject(parent)
{
}

QString WizCommonUI::loadTextFromFile(const QString& strFileName)
{
    QString strText;
    ::WizLoadUnicodeTextFromFile(strFileName, strText);
    return strText;
}

bool WizCommonUI::saveTextToFile(const QString &strFileName, const QString &strText, const QString &strCharset)
{
    QString charset =  strCharset.toLower();
    if (charset == "unicode" || charset == "utf-8") {
        return ::WizSaveUnicodeTextToUtf8File(strFileName, strText, false);
    } else if (charset == "utf-16") {
        return ::WizSaveUnicodeTextToUtf16File(strFileName, strText);
    } else {
        return ::WizSaveUnicodeTextToUtf8File(strFileName, strText);
    }
}

QString WizCommonUI::clipboardToImage(int hwnd, const QString& strOptions)
{
    Q_UNUSED(hwnd);
    //
    QClipboard* clipboard = QApplication::clipboard();
    if (!clipboard)
        return CString();
    //
    //
    QImage image = clipboard->image();
    if (image.isNull())
        return CString();
    //
    CString strTempPath = ::WizGetCommandLineValue(strOptions, "TempPath");
    if (strTempPath.isEmpty())
    {
        strTempPath = Utils::WizPathResolve::tempPath();
    }
    else
    {
        ::WizPathAddBackslash(strTempPath);
        ::WizEnsurePathExists(strTempPath);
    }
    //
    CString strFileName = strTempPath + WizIntToStr(WizGetTickCount()) + ".png";
    if (!image.save(strFileName))
        return CString();
    //
    return strFileName;
}

QString WizCommonUI::LoadTextFromFile(const QString& strFileName)
{
    return loadTextFromFile(strFileName);
}

bool WizCommonUI::SaveTextToFile(const QString &strFileName, const QString &strText, const QString &strCharset)
{
    return saveTextToFile(strFileName, strText, strCharset);
}

QString WizCommonUI::ClipboardToImage(const QString& strOptions)
{
    return clipboardToImage(0, strOptions);
}

QString WizCommonUI::GetSpecialFolder(const QString &bstrFolderName)
{
    if (bstrFolderName == "TemporaryFolder") {
        return Utils::WizPathResolve::tempPath();
    } else if (bstrFolderName == "AppPath") {
        return Utils::WizPathResolve::appPath();
    } else if (bstrFolderName == "DataPath") {
        return Utils::WizPathResolve::dataStorePath();
    } else {
        return "";
    }
}

QString WizCommonUI::GetATempFileName(const QString &bstrFileExt)
{
    QString strTempFileName = QUuid::createUuid().toString() + bstrFileExt;
    return QDir(GetSpecialFolder("TemporaryFolder")).absoluteFilePath(strTempFileName);
}

/**
 * @brief Creates the directory path dirPath.
 * 
 *      The function will create all parent directories necessary to create the directory.
 * 
 * @param bstrPath 
 * @return true 
 * @return false 
 */
bool WizCommonUI::CreateDirectory(const QString &bstrPath)
{
    return QDir().mkpath(bstrPath);
}

/**
 * @brief Download resource to file.
 * 
 * @param bstrURL 
 * @param bstrFileName 
 * @return true 
 * @return false 
 */
bool WizCommonUI::URLDownloadToFile(const QString &bstrURL, const QString &bstrFileName, bool isImage)
{
    return WizURLDownloadToFile(bstrURL, bstrFileName, isImage);
}


bool WizCommonUI::Base64ToFile(const QString &base64, const QString &fileName)
{
    QByteArray buffer = QByteArray::fromBase64(base64.toUtf8());
    QSaveFile file(fileName);
    file.open(QIODevice::WriteOnly);
    file.write(buffer);
    file.commit();
    return true;
}

void WizCommonUI::OpenUrl(const QString &url)
{
    QDesktopServices::openUrl(QUrl(url));
}

QString WizCommonUI::SelectWindowsFile(bool isOpen, const QString &filter)
{
    if (isOpen) {
        return QFileDialog::getOpenFileName(
            nullptr, tr("Select File"), QDir::home().absolutePath(), filter);
    } else {
        return QFileDialog::getSaveFileName(
            nullptr, tr("Select File"), QDir::home().absolutePath(), filter);
    }
}

bool WizCommonUI::PathFileExists(const QString &path)
{
    return WizPathFileExists(path);
}

void WizCommonUI::CopyFile(const QString &existingFile, const QString &newFileName)
{
    QFile::copy(existingFile, newFileName);
}

QString WizCommonUI::GetValueFromIni(const QString &fileName, const QString &section, const QString &key)
{
    WizSettings setting(fileName);
    return setting.getString(section, key);
}

void WizCommonUI::SetValueToIni(const QString &fileName, const QString &section, const QString &key, const QString &value)
{
    WizSettings setting(fileName);
    setting.setString(section, key, value);
}

QString WizCommonUI::RunExe(const QString &exeFileName, const QStringList &params)
{
    // FIXME: 在另一个进程中启动
    QProcess *process = new QProcess(this);
    process->start(exeFileName, params);
    process->waitForFinished();
    QByteArray output = process->readAllStandardOutput();
    QByteArray error = process->readAllStandardError();
    int exitCode = process->exitCode();
    process->deleteLater();

    return exitCode == 0 ? QString(output) : QString(error);
}

QObject* WizCommonUI::RunProc(const QString &exeFileName, const QStringList &params,
                              bool wait, bool logging)
{
    auto *process = new QtWrapper::Process(this);

    if (logging) {
        connect(process, &QProcess::readyReadStandardOutput, [=]() {
            QByteArray output = process->readAllStandardOutput();
            QTextStream outputStream(output);
            while (!outputStream.atEnd()) {
                QString line = outputStream.readLine();
                qInfo() << line;
            }
        });

        connect(process, &QProcess::readyReadStandardError, [=]() {
            QByteArray errorOutput = process->readAllStandardError();
            QTextStream errorStream(errorOutput);
            while (!errorStream.atEnd()) {
                QString line = errorStream.readLine();
                qDebug() << line;
            }
        });
    }

    process->start(exeFileName, params);

    if (wait) {
        process->waitForStarted();
        while(process->state() != QProcess::NotRunning)
            QApplication::processEvents();
    }

    return process;
}

QObject* WizCommonUI::CreateProcess()
{
    return new QtWrapper::Process(this);
}

void WizCommonUI::ShowMessage(const QString &title, const QString &text, unsigned int type)
{
    switch(type) {
    case 1:
        QMessageBox::warning(nullptr, title, text);
        break;
    case 2:
        QMessageBox::critical(nullptr, title, text);
        break;
    case 0:
    default:
        QMessageBox::information(nullptr, title, text);
        break;
    }
}

bool WizCommonUI::Confirm(const QString &title, const QString &text)
{
    auto ret = QMessageBox::question(nullptr, title, text);
    return ret == QMessageBox::Yes;
}

int WizCommonUI::GetIntValue(const QString &title, const QString &description,
                                         int value, int min, int max, int step)
{
    return QInputDialog::getInt(nullptr, title, description, value, min, max, step);
}

double WizCommonUI::GetDoubleValue(const QString &title, const QString &description,
                                double value, double min, double max, double step, double decimals)
{
    return QInputDialog::getDouble(nullptr, title, description, value, min, max, decimals, nullptr,
                                   Qt::WindowFlags(), step);
}

QString WizCommonUI::InputBox(const QString &title, const QString &description, const QString &value)
{
    return QInputDialog::getText(nullptr, title, description, QLineEdit::Normal, value);
}

QString WizCommonUI::InputMultiLineText(const QString &title, const QString &description, const QString &value)
{
    return QInputDialog::getMultiLineText(nullptr, title, description, value);
}

QString WizCommonUI::SelectItem(const QString &title, const QString &description, const QStringList &items,
                                   int current, bool editable)
{
    return QInputDialog::getItem(nullptr, title, description, items, current, editable);
}
