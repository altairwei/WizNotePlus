#include "AbstractDocumentView.h"

#include <QPaintEvent>
#include <QStyleOption>
#include <QPainter>
#include <QStyle>

void AbstractDocumentView::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
