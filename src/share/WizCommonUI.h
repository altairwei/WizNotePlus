#ifndef WIZCOMMONUI_H
#define WIZCOMMONUI_H

#include "WizQtHelper.h"

class WizCommonUI : public QObject
{
    Q_OBJECT
    
public:
    WizCommonUI(QObject* parent);
    //interface WizKMControls.WizCommonUI
    Q_INVOKABLE QString LoadTextFromFile(const QString& strFileName);
    Q_INVOKABLE void SaveTextToFile(const QString &strFileName, const QString &strText, const QString &strCharset);
    Q_INVOKABLE QString ClipboardToImage(int hwnd, const QString& strOptions);

    Q_INVOKABLE QString GetSpecialFolder(const QString &bstrFolderName);
    Q_INVOKABLE QString GetATempFileName(const QString &bstrFileExt);

private:
    //interface WizKMControls.WizCommonUI;
    QString loadTextFromFile(const QString& strFileName);
    void saveTextToFile(const QString &strFileName, const QString &strText, const QString &strCharset);
    QString clipboardToImage(int hwnd, const QString& strOptions);

};

#endif // WIZCOMMONUI_H
