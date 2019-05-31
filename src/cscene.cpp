#include "cscene.h"

#include "csettings.h"
#include "cbackgroundmanager.h"
#include "ctilesmanager.h"

#include <QFileInfo>
#include <QSvgRenderer>
#include <QGraphicsSvgItem>

CScene::CScene(QObject *parent) : QGraphicsScene(parent)
{

}

// ------------------------------------------------------------------------------------------------
// Установить изображение из файла
void CScene::setBackground(const QString &file_name)
{
    if (!QFileInfo::exists(file_name)) {
        settings->setDefaultBackground();
    }

    if (!QFileInfo::exists(file_name)) return;

    QSize size;

    if (bg_manager->isCurrentSvg()) {
        // SVG item
        QSvgRenderer bg_renderer(file_name);
        QGraphicsSvgItem m_bg_svg;
        m_bg_svg.setSharedRenderer(&bg_renderer);
        size = m_bg_svg.boundingRect().size().toSize();
        // Рисуем на Image
        QImage img(size, QImage::Format_ARGB32_Premultiplied);
        QPainter painter(&img);
        bg_renderer.render(&painter);
        // И сохраним как зависимый Pixmap
        m_bg_pixmap = QPixmap::fromImage(img);
    } else {
        // Image item
        QImage img(file_name);
        m_bg_pixmap = QPixmap::fromImage(img);
    }
}

// ------------------------------------------------------------------------------------------------
void CScene::newGame(const Field *field)
{
    clear();

    for (const auto &tile : *field) {
        QPoint point(50, 50);

        auto item_base = new QGraphicsSvgItem;
        item_base->setSharedRenderer(tiles_manager->currentRenderer());
        item_base->setElementId("TILE_2");
        item_base->setPos(point);
        addItem(item_base);

        auto item_tile = new QGraphicsSvgItem;
        item_tile->setSharedRenderer(tiles_manager->currentRenderer());
        item_tile->setElementId(tiles_manager->tilesNames()[tile]);
        item_tile->setPos(point);
        addItem(item_tile);
    }
}

// ------------------------------------------------------------------------------------------------
// Отобразить фоновое изображение
void CScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->drawPixmap(rect, m_bg_pixmap, m_bg_pixmap.rect());
}
