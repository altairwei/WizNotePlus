#include "AbstractTabPage.h"

#include <QDebug>

AbstractTabPage::~AbstractTabPage()
{

}

QList<QAction *> AbstractTabPage::TabContextMenuActions()
{
    return QList<QAction *>();
}