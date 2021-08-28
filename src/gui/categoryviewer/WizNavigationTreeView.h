#ifndef GUI_CATEGORYVIEWER_WIZNAVIGATIONTREEVIEW_H
#define GUI_CATEGORYVIEWER_WIZNAVIGATIONTREEVIEW_H

#include <QTreeView>

class WizNavigationTreeView : public QTreeView
{
    Q_OBJECT

public:
    WizNavigationTreeView(QWidget *parent = nullptr);

    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;

protected:
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
};

#endif // GUI_CATEGORYVIEWER_WIZNAVIGATIONTREEVIEW_H