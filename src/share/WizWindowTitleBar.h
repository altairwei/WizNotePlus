#ifndef WIZTITLEBAR_H
#define WIZTITLEBAR_H

#include <QWidget>

class QToolButton;
class QLabel;

class WizWindowTitleBar : public QWidget
{
    Q_OBJECT
public:
    WizWindowTitleBar(QWidget *parent, QWidget* window, bool canResize);

private:
    QWidget* m_window;
    QWidget* m_shadowContainerWidget;
    QMargins m_oldContentsMargin;

public slots:
    void showSmall();
    void showMaxRestore();

public:
    QToolButton* maxButton() const { return m_maximize; }
    QToolButton* minButton() const { return m_minimize; }
    QToolButton* closeButton() const { return m_close; }
    QLabel* titleLabel() const { return m_titleLabel; }

    void setContentsMargins(QMargins margins);

    void setText(QString title);
    QString text() const;

protected:
   void paintEvent(QPaintEvent* ev) override;

public:
    virtual void layoutTitleBar();
    virtual void windowStateChanged();

private:
    QToolButton *m_minimize;
    QToolButton *m_maximize;
    QToolButton *m_close;
    QLabel* m_titleLabel;
    bool m_canResize;
};



#endif // WIZTITLEBAR_H
