#include "WizNavigationTreeView.h"

#include <QFocusEvent>
#include <QHeaderView>
#include <QScrollBar>

WizNavigationTreeView::WizNavigationTreeView(QWidget *parent)
    : QTreeView(parent)
{
    setFrameStyle(QFrame::NoFrame);
    setIndentation(indentation() * 9/10);
    setUniformRowHeights(true);
    setTextElideMode(Qt::ElideNone);
    setAttribute(Qt::WA_MacShowFocusRect, false);

    setHeaderHidden(true);
    // We let the column adjust to contents, but note
    // the setting of a minimum size in resizeEvent()
    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    header()->setStretchLastSection(false);
}

void WizNavigationTreeView::scrollTo(const QModelIndex &index, QAbstractItemView::ScrollHint hint)
{
    // work around QTBUG-3927
    QScrollBar *hBar = horizontalScrollBar();
    int scrollX = hBar->value();

    const int viewportWidth = viewport()->width();
    QRect itemRect = visualRect(index);

    QAbstractItemDelegate *delegate = itemDelegate(index);
    if (delegate) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const QStyleOptionViewItem option = viewOptions();
#else
        QStyleOptionViewItem option;
        initViewItemOption(&option);
#endif
        itemRect.setWidth(delegate->sizeHint(option, index).width());
    }

    if (itemRect.x() - indentation() < 0) {
        // scroll so left edge minus one indent of item is visible
        scrollX += itemRect.x() - indentation();
    } else if (itemRect.right() > viewportWidth) {
        // If right edge of item is not visible and left edge is "too far right",
        // then move so it is either fully visible, or to the left edge.
        // For this move the left edge one indent to the left, so the parent can potentially
        // still be visible.
        if (itemRect.width() + indentation() < viewportWidth)
            scrollX += itemRect.right() - viewportWidth;
        else
            scrollX += itemRect.x() - indentation();
    }
    scrollX = qBound(hBar->minimum(), scrollX, hBar->maximum());
    QTreeView::scrollTo(index, hint);
    hBar->setValue(scrollX);
}

// This is a workaround to stop Qt from redrawing the project tree every
// time the user opens or closes a menu when it has focus. Would be nicer to
// fix it in Qt.
void WizNavigationTreeView::focusInEvent(QFocusEvent *event)
{
    if (event->reason() != Qt::PopupFocusReason)
        QTreeView::focusInEvent(event);
}

void WizNavigationTreeView::focusOutEvent(QFocusEvent *event)
{
    if (event->reason() != Qt::PopupFocusReason)
        QTreeView::focusOutEvent(event);
}

void WizNavigationTreeView::resizeEvent(QResizeEvent *event)
{
    const int columns = header()->count();
    const int minimumWidth = columns > 1 ? viewport()->width() / columns
                                         : viewport()->width();
    header()->setMinimumSectionSize(minimumWidth);
    QTreeView::resizeEvent(event);
}

void WizNavigationTreeView::keyPressEvent(QKeyEvent *event)
{
    // Note: This always eats the event
    // whereas QAbstractItemView never eats it
    if ((event->key() == Qt::Key_Return
            || event->key() == Qt::Key_Enter)
            && event->modifiers() == 0
            && QTreeView::currentIndex().isValid()
            && QTreeView::state() != QAbstractItemView::EditingState) {
        emit QTreeView::activated(QTreeView::currentIndex());
        return;
    }
    QTreeView::keyPressEvent(event);
}