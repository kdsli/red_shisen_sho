#include "cscene.h"

#include "csettings.h"
#include "cbackgroundmanager.h"
#include "ctilesmanager.h"
#include "cfield.h"

#include <QFileInfo>
#include <QSvgRenderer>
#include <QGraphicsSvgItem>

#include "debug.h"

CScene::CScene(CField *field, QObject *parent) : QGraphicsScene(parent),
    m_field(field)
{
    initDebug(0, 0);
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
void CScene::newGame()
{
    clear();

    QPointF point(0, 0);
    int n = 0;

    for (int y = 0; y < m_field->y(); ++y) {
        for (int x = 0; x < m_field->x(); ++x) {

            auto item_base = new QGraphicsSvgItem;
            item_base->setSharedRenderer(tiles_manager->currentRenderer());
            item_base->setElementId("TILE_2");
            item_base->setPos(point);
            addItem(item_base);

            auto item_tile = new QGraphicsSvgItem;
            item_tile->setSharedRenderer(tiles_manager->currentRenderer());
            item_tile->setElementId(tiles_manager->tilesNames()[m_field->tiles()[n++]]);
            item_tile->setPos(point);
            addItem(item_tile);

            point.setX(point.x() + tiles_manager->tileSize().width());
        }
        point.setX(0);
        point.setY(point.y() + tiles_manager->tileSize().height());
    }

//    auto rect = QRectF(0, 0,
//                       tiles_manager->tileSize().width() * m_field->x() * 1.1,
//                       tiles_manager->tileSize().height() * m_field->y() * 1.1);

//    setSceneRect(rect);
}

// ------------------------------------------------------------------------------------------------
// Отобразить фоновое изображение
void CScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    printVariable<QRectF>("drawBackground", rect);
    painter->drawPixmap(rect, m_bg_pixmap, m_bg_pixmap.rect());
}

// Временно
QTextStream& operator<<(QTextStream& stream, const QRectF& rect) {
    stream << "TopLeft (" << rect.left() << ", " << rect.y() << ") Size (" << rect.width() << ", " << rect.height() << ")";
    return stream;
}

// ------------------------------------------------------------------------------------------------
void CScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    auto r = sceneRect();
    QPen pen(Qt::red);
    pen.setWidth(3);
    painter->setPen(pen);
    painter->drawLine(r.topLeft(), r.bottomRight());
}
