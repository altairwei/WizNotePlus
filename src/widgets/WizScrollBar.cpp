#include "WizScrollBar.h"

#include <QtGui>

WizScrollBar::WizScrollBar(QWidget* parent /* = 0 */)
    : QScrollBar(parent)
{
    setMouseTracking(true);

    connect(&m_timerScrollTimeout, SIGNAL(timeout()), this, SLOT(on_scrollTimeout()));
    connect(this, SIGNAL(valueChanged(int)), SLOT(on_valueChanged(int)));

    m_timerScrollTimeout.setSingleShot(true);
    m_timerScrollTimeout.start(3000);
}

void WizScrollBar::mouseMoveEvent(QMouseEvent* event)
{
    m_timerScrollTimeout.start(3000);

    QScrollBar::mouseMoveEvent(event);
}

void WizScrollBar::syncWith(QScrollBar* source)
{
    setMinimum(source->minimum());
    setMaximum(source->maximum());
    setSliderPosition(source->sliderPosition());
    setPageStep(source->pageStep());
    setSingleStep(source->singleStep());

    connect(source, SIGNAL(valueChanged(int)), SLOT(on_sourceValueChanged(int)));
    connect(source, SIGNAL(rangeChanged(int, int)), SLOT(on_sourceRangeChanged(int, int)));

    m_scrollSyncSource = source;

}

void WizScrollBar::showHandle()
{
    if (maximum() == minimum())
        return;

    show();

    m_timerScrollTimeout.start(1000);
}

void WizScrollBar::hideHandle()
{
    hide();
}

void WizScrollBar::on_sourceValueChanged(int value)
{
    setSliderPosition(value);
}

void WizScrollBar::on_sourceRangeChanged(int min, int max)
{
    setMinimum(min);
    setMaximum(max);
    setPageStep(m_scrollSyncSource->pageStep());
    setSingleStep(m_scrollSyncSource->singleStep());
}

void WizScrollBar::on_valueChanged(int value)
{
    if (m_scrollSyncSource) {
        m_scrollSyncSource->setSliderPosition(value);
    }

    showHandle();
}

void WizScrollBar::on_scrollTimeout()
{
    hideHandle();
}


WizListWidgetWithCustomScorllBar::WizListWidgetWithCustomScorllBar(QWidget* parent)
    : QListWidget(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_vScroll = new WizScrollBar(this);
    m_vScroll->syncWith(verticalScrollBar());
}

void WizListWidgetWithCustomScorllBar::resizeEvent(QResizeEvent* event)
{
    // reset scrollbar
    m_vScroll->resize(m_vScroll->sizeHint().width(), event->size().height());
    m_vScroll->move(event->size().width() - m_vScroll->sizeHint().width(), 0);
    QListWidget::resizeEvent(event);
}
