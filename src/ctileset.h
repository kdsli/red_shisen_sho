#ifndef CTILESET_H
#define CTILESET_H

#include <QObject>
#include <QVector>
#include <QSizeF>
#include <QPixmap>

QT_BEGIN_NAMESPACE
class QSvgRenderer;
QT_END_NAMESPACE

// Объект - набор изображений костяшек. Формируется из SVG файла при каждом
// изменении размера поля. Хранит текущий размер основы и костяшки

class CTileSet : public QObject
{
public:
    explicit CTileSet(QObject * parent = nullptr);

    // Инициализация
    void initTileSet();
    // Рассчитать размеры в зависимости от пропорции
    void CalcSizes(double ratio);

    // Список изображений костяшек
    const QVector<QPixmap> &pixmaps() const { return m_pixmaps; }
    // Изображения подложек
    const QPixmap &base_pixmap() const { return m_base_pixmap; }
    const QPixmap &base_sel_pixmap() const { return m_base_sel_pixmap; }
    // Исходная ширина и высота костяшек
    const QSizeF base_size() const { return m_base_size; }
    const QSizeF tile_size() const { return m_tile_size; }
    // Текущая ширина и высота костяшек с учетом масштаба
    const QSizeF scaled_base_size() const { return m_scaled_base_size; }
    const QSizeF scaled_tile_size() const { return m_scaled_tile_size; }

private:
    QSvgRenderer *m_renderer;
    QVector<QPixmap> m_pixmaps;
    QPixmap m_base_pixmap;
    QPixmap m_base_sel_pixmap;
    QSizeF m_base_size;
    QSizeF m_tile_size;
    QSizeF m_scaled_base_size;
    QSizeF m_scaled_tile_size;

    void addSeries(const QString &serie_name, int count);
    QPixmap getTilePixmap(const QString &tile_name) const;
    void getTileSize(const QString &tile_name, QSizeF &size) const;

};

#endif // CTILESET_H
