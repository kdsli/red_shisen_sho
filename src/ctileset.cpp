#include "ctileset.h"

#include "csettings.h"
#include "options/ctilesmanager.h"

#include <QSvgRenderer>
#include <QGraphicsSvgItem>
#include <QApplication>
#include <QPainter>
#include <QDir>
#include <QFileInfo>
#include <math.h>

/*
 * CHARACTER_x 1-9 иероглифы
 * ROD_x 1-9 кружочки
 * BAMBOO_x 1-9 палочки, начиная с дракона
 * DRAGON_x 1-3 пусто, зеленый иероглиф, красное Ф
 * FLOWER_x 1-4 цветы без горшков
 * SEASON_x 1-4 цветы в горшках
 * WIND_x 1-4 N, S, E, W
 * TILE_x 1-4 (северо-восток, юго-восток, юго-запад, северо-запад)
 * TILE_x_SEL 1-4 тоже выделено
 * TILE_SHAD_SEL (67x96)
 * TILE_SHAD_NORM
 *
*/

CTileSet::CTileSet(QObject *parent) : QObject(parent),
    m_renderer(nullptr)
{
    tiles_manager = new CTilesManager(this);

    initTileSet();
}

// ================================================================================================
// Инициализация фигур. Выполняется при каждом изменении файла
void CTileSet::initTileSet()
{
    QString file_name = tiles_manager->currentFile();
    // !!! Обработать отсутствие файла - заменить на по умолчанию и проверить его
    if (!QFileInfo::exists(file_name)) return;

    m_renderer = new QSvgRenderer(file_name, this);

    // Базовый размер основы и костяшки
    getTileSize("TILE_2", m_base_size);
    getTileSize("CHARACTER_2", m_tile_size);

}

// ================================================================================================
// Инициализация фигур. Выполняется при каждом изменении размера поля
void CTileSet::CalcSizes(double ratio)
{
    // Рассчитаем новые размеры костяшек
    m_scaled_base_size = m_base_size * ratio;
    m_scaled_base_size.setWidth(trunc(m_scaled_base_size.width()));
    m_scaled_base_size.setHeight(trunc(m_scaled_base_size.height()));

    m_scaled_tile_size = m_tile_size * ratio;
    m_scaled_tile_size.setWidth(trunc(m_scaled_tile_size.width()));
    m_scaled_tile_size.setHeight(trunc(m_scaled_tile_size.height()));

    // Основа
    m_base_pixmap = getTilePixmap("TILE_2");
    m_base_sel_pixmap = getTilePixmap("TILE_2_SEL");

    // Заполним массив элементов
    m_pixmaps.clear();
    addSeries("CHARACTER", 9);
    addSeries("ROD", 9);
    addSeries("BAMBOO", 9);
    addSeries("WIND", 4);
    addSeries("SEASON", tiles_manager->isCorrectSVG() ? 4 : 1);
    addSeries("DRAGON", 3);
    addSeries("FLOWER", tiles_manager->isCorrectSVG() ? 4 : 1);
}

// ================================================================================================
// Добавить серию костяшек
void CTileSet::addSeries(const QString &serie_name, int count)
{
    QImage image(m_scaled_tile_size.toSize(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);

    for (int i = 1; i < count + 1; ++i) {
        QString name = serie_name + "_" + QString::number(i);
        image.fill(0);
        // Рисуем на Pixmap
        m_renderer->render(&painter, name);
        // После некотрых раздумий пришел к выводу, что использовать vector of pixmap
        // эффективнее в данном случае, чем QPixmapCache, так как доступ
        // к изображениям происходит по индексу, а не по имени или QPixmapCache::Key,
        // который так же надо хранить
        QPixmap pixmap = QPixmap::fromImage(image);
        m_pixmaps.append(std::move(pixmap));
    }
}

// ================================================================================================
// Получить pixmap костяшки
QPixmap CTileSet::getTilePixmap(const QString &tile_name) const
{
    QImage image(m_scaled_base_size.toSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(0);

    QPainter painter;
    painter.begin(&image);
    m_renderer->render(&painter, tile_name);
    painter.end();

    return std::move(QPixmap::fromImage(image));
}

// ================================================================================================
// Получить размер костяшки
void CTileSet::getTileSize(const QString &tile_name, QSizeF &size) const
{
    QGraphicsSvgItem item;
    item.setSharedRenderer(m_renderer);
    item.setElementId(tile_name);
    size.setWidth(item.boundingRect().toRect().width());
    size.setHeight(item.boundingRect().toRect().height());
}
