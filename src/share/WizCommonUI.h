#ifndef WIZCOMMONUI_H
#define WIZCOMMONUI_H

#include "WizQtHelper.h"

class WizCommonUI : public QObject
{
    Q_OBJECT

public:
    WizCommonUI(QObject* parent);

    Q_INVOKABLE QString LoadTextFromFile(const QString& strFileName);
    Q_INVOKABLE bool SaveTextToFile(const QString &strFileName, const QString &strText, const QString &strCharset);
    Q_INVOKABLE QString ClipboardToImage(const QString& strOptions);

    Q_INVOKABLE QString GetSpecialFolder(const QString &bstrFolderName);
    Q_INVOKABLE QString GetATempFileName(const QString &bstrFileExt);
    Q_INVOKABLE bool CreateDirectory(const QString &bstrPath);

    Q_INVOKABLE bool URLDownloadToFile(const QString &bstrURL, const QString &bstrFileName,  bool isImage);
    Q_INVOKABLE bool Base64ToFile(const QString &base64, const QString &fileName);

    Q_INVOKABLE void OpenUrl(const QString &url);
    Q_INVOKABLE QString SelectWindowsFile(bool isOpen, const QString &filter);

    Q_INVOKABLE bool PathFileExists(const QString &path);
    Q_INVOKABLE void CopyFile(const QString &existingFile, const QString &newFileName);

    Q_INVOKABLE QString GetValueFromIni(const QString &fileName, const QString &section, const QString &key);
    Q_INVOKABLE void SetValueToIni(const QString &fileName, const QString &section, const QString &key, const QString &value);

    Q_INVOKABLE QString RunExe(const QString &exeFileName, const QStringList &params);
    Q_INVOKABLE QObject* RunProc(const QString &exeFileName, const QStringList &params,
                                 bool wait = false, bool logging = false);
    Q_INVOKABLE QObject* CreateProcess();

    Q_INVOKABLE void ShowMessage(const QString &title, const QString &text, unsigned int type);
    Q_INVOKABLE bool Confirm(const QString &title, const QString &text);
    Q_INVOKABLE int GetIntValue(const QString &title, const QString &description,
                                int value = 0, int min = -2147483647, int max = 2147483647, int step = 1);
    Q_INVOKABLE double GetDoubleValue(const QString &title, const QString &description,
                                      double value = 0, double min = -2147483647, double max = 2147483647,
                                      double step = 1, double decimals = 1);
    Q_INVOKABLE QString InputBox(const QString &title, const QString &description, const QString &value);
    Q_INVOKABLE QString InputMultiLineText(const QString &title, const QString &description, const QString &value);
    Q_INVOKABLE QString SelectItem(const QString &title, const QString &description, const QStringList &items,
                                   int current = 0, bool editable = false);

private:
    //interface WizKMControls.WizCommonUI;
    QString loadTextFromFile(const QString& strFileName);
    bool saveTextToFile(const QString &strFileName, const QString &strText, const QString &strCharset);
    QString clipboardToImage(int hwnd, const QString& strOptions);

};

#endif // WIZCOMMONUI_H
