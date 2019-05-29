#ifndef CLABELEDSLIDER_H
#define CLABELEDSLIDER_H

#include <QObject>
#include <QSlider>

// Слайдер с метками
class CLabeledSlider : public QSlider
{
public:
    CLabeledSlider(QWidget *parent, const QStringList &);

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QStringList m_labels;
    int m_labels_height;
    int m_label_descent;
};

#endif // CLABELEDSLIDER_H
