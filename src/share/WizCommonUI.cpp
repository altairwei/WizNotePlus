#include "WizCommonUI.h"

#include <QClipboard>
#include <QApplication>
#include <QUuid>
#include <QDir>

#include "WizMisc.h"
#include "utils/WizPathResolve.h"


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

void WizCommonUI::saveTextToFile(const QString &strFileName, const QString &strText, const QString &strCharset)
{
    QString charset =  strCharset.toLower();
    if (charset == "unicode" || charset == "utf-8") {
        ::WizSaveUnicodeTextToUtf8File(strFileName, strText, false);
    } else if (charset == "utf-16") {
        ::WizSaveUnicodeTextToUtf16File(strFileName, strText);
    } else {
        ::WizSaveUnicodeTextToUtf8File(strFileName, strText);
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

void WizCommonUI::SaveTextToFile(const QString &strFileName, const QString &strText, const QString &strCharset)
{
    saveTextToFile(strFileName, strText, strCharset);
}

QString WizCommonUI::ClipboardToImage(int hwnd, const QString& strOptions)
{
    return clipboardToImage(hwnd, strOptions);
}

QString WizCommonUI::GetSpecialFolder(const QString &bstrFolderName)
{
    if (bstrFolderName == "TemporaryFolder") {
        return Utils::WizPathResolve::tempPath();
    } else if (bstrFolderName == "AppPath") {
        return Utils::WizPathResolve::appPath();
    } else {
        return "";
    }
}

QString WizCommonUI::GetATempFileName(const QString &bstrFileExt)
{
    QString strTempFileName = QUuid::createUuid().toString() + bstrFileExt;
    return QDir(GetSpecialFolder("TemporaryFolder")).absoluteFilePath(strTempFileName);
}
