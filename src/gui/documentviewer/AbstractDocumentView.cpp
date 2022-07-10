#include "AbstractDocumentView.h"

#include <QPaintEvent>
#include <QStyleOption>
#include <QPainter>
#include <QStyle>
#include <QAction>

#include "share/WizMisc.h"

AbstractDocumentView::AbstractDocumentView(QWidget *parent)
    : AbstractTabPage(parent)
{
    m_locateAction = new QAction(tr("Locate this document in category"), this);
    connect(m_locateAction, &QAction::triggered, this,
            [&] { Q_EMIT locateDocumentRequest(note()); });

    m_copyInternalLinkAction = new QAction(tr("Copy internal link"), this);
    connect(m_copyInternalLinkAction, &QAction::triggered, [this] {
        WizCopyNoteAsInternalLink(this->note());
    });
}

void AbstractDocumentView::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

QList<QAction *> AbstractDocumentView::TabContextMenuActions()
{
    QList<QAction *> actions;
    actions.append(m_locateAction);
    actions.append(m_copyInternalLinkAction);

    return actions;
}
