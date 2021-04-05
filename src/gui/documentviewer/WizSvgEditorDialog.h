#ifndef GUI_DOCUMENTVIEWER_WIZSVGEDITORDIALOG_H
#define GUI_DOCUMENTVIEWER_WIZSVGEDITORDIALOG_H

#include "WizWebSettingsDialog.h"

class WizSvgEditorDialog: public WizWebSettingsDialog
{
    Q_OBJECT
public:
    WizSvgEditorDialog(QString url, QString data, std::function<void(bool changed, std::function<void(bool)> saved)> saveCallback, QString htmlFilePath, QWidget* parent = nullptr);
    //
private:
    bool m_error;
    QString m_data;
    std::function<void(bool changed, std::function<void(bool)> saved)> m_saveCallback;
    QString m_htmlFilePath;
    QString m_indexFilesPath;
private:
protected:
    virtual void onLoaded(bool ok);
    virtual void reject();
public:
    Q_INVOKABLE void loadData(QString fileName, QString callback);
    Q_INVOKABLE void saveData(QString options, QString svgData, QString htmlData);
    //Q_INVOKABLE void postMessageToNoteEditor(QString message);
};

class WizDatabaseManager;
struct WIZDOCUMENTDATAEX;

void editHandwritingNote(WizDatabaseManager& dbMgr, const WIZDOCUMENTDATAEX& doc, QString strHtmlFile, QString data, QWidget* parent);
void createHandwritingNote(WizDatabaseManager& dbMgr, const WIZDOCUMENTDATAEX& doc, QWidget* parent);

#endif // GUI_DOCUMENTVIEWER_WIZSVGEDITORDIALOG_H