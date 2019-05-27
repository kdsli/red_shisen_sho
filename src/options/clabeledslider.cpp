#include "clabeledslider.h"

#include <QLabel>
#include <QPainter>

static const int interval = 5;

CLabeledSlider::CLabeledSlider(QWidget *parent, const QStringList &list) : QSlider(parent)
{
    m_labels = list;

    setOrientation(Qt::Horizontal);
    setMinimum(0);
    setMaximum(m_labels.size() - 1);
    setSingleStep(1);
    setPageStep(1);
    setTickPosition(QSlider::TicksBelow);

    QFontMetrics fm(font());
    m_labels_height = fm.height();
    m_label_descent = fm.descent();
}

QSize CLabeledSlider::sizeHint() const
{
    auto size = QSlider::sizeHint();
    size.setHeight(size.height() + m_labels_height + m_label_descent + interval);

    return size;
}

void CLabeledSlider::paintEvent(QPaintEvent *event)
{
    QSlider::paintEvent(event);

    QPainter painter(this);

    auto size = QSlider::sizeHint();

    // Эмпирически вычисленный margin слайдера
    const int margin_x = 10;

    int step_x = (width() - 2 * margin_x) / (m_labels.size() - 1);
    auto top_y = size.height() + m_labels_height + interval;

    // Первая метка
    painter.drawText(0, top_y, m_labels.front());

    // Средние метки
    for (int i = 1; i < m_labels.size() - 1; ++i) {
        auto rect = QFontMetrics(font()).boundingRect(m_labels[i]);
        painter.drawText(margin_x + i * step_x - rect.width() / 2, top_y, m_labels[i]);
    }

    // Последняя метка
    auto rect = QFontMetrics(font()).boundingRect(m_labels.back());
    painter.drawText(width() - rect.width() - 1, top_y, m_labels.back());
}

