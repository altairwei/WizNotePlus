#ifndef ABSTRACTDOCUMENTVIEW_H
#define ABSTRACTDOCUMENTVIEW_H

#include "gui/tabbrowser/AbstractTabPage.h"
#include "share/WizWebEngineView.h"
#include "share/WizObject.h"

class WizLocalProgressWebView;
class AbstractDocumentView : public AbstractTabPage
{
    Q_OBJECT

public:
    AbstractDocumentView(QWidget *parent = nullptr)
        : AbstractTabPage(parent) {}

    virtual WizEditorMode editorMode() = 0;
    virtual const WIZDOCUMENTDATAEX& note() const = 0;
};

class AbstractDocumentEditor : public WizWebEngineView
{

    Q_OBJECT

public:
    explicit AbstractDocumentEditor(QWidget *parent = nullptr)
        : WizWebEngineView(parent) {}

    virtual void isModified(std::function<void(bool modified)> callback) = 0;

public Q_SLOTS:
    virtual void onTitleEdited(QString strTitle) = 0;
};

#endif // ABSTRACTDOCUMENTVIEW_H
