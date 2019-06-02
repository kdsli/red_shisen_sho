#include "cscene.h"

#include "csettings.h"
#include "cbackgroundmanager.h"
#include "ctilesmanager.h"
#include "cfield.h"

#include <QFileInfo>
#include <QSvgRenderer>

#include <QDebug>

CScene::CScene(CField *field, QObject *parent) : QGraphicsScene(parent),
    m_field(field)
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
void CScene::newGame()
{
    clear();
    m_tiles_list.clear();
    m_bases_list.clear();
    m_selected.clear();

    QPointF point(0, 0);
    int n = 0;

    for (int y = 0; y < m_field->y(); ++y) {
        for (int x = 0; x < m_field->x(); ++x) {

            auto item_base = new QGraphicsSvgItem;
            item_base->setSharedRenderer(tiles_manager->currentRenderer());
            item_base->setElementId(tiles_manager->getBaseName());
            item_base->setPos(point);
            addItem(item_base);
            m_bases_list.append(item_base);

            auto item_tile = new QGraphicsSvgItem;
            item_tile->setSharedRenderer(tiles_manager->currentRenderer());
            item_tile->setElementId(tiles_manager->tilesNames()[m_field->tiles()[n++]]);
            item_tile->setPos(point);
            addItem(item_tile);
            m_tiles_list.append(item_tile);

            point.setX(point.x() + tiles_manager->tileSize().width());
        }
        point.setX(0);
        point.setY(point.y() + tiles_manager->tileSize().height());
    }

    // Координаты ниже - в логических единицах

    // Реальная ширина/высота поля с учетом дополнительного фон
    auto w = tiles_manager->tileSize().width() * m_field->x() +
            (tiles_manager->baseSize().width() - tiles_manager->tileSize().width());
    auto h = tiles_manager->tileSize().height() * m_field->y() +
            (tiles_manager->baseSize().height() - tiles_manager->tileSize().height());
    auto rect = QRectF(0, 0, w, h);

    // Это и есть размер поля
    setSceneRect(rect);

    // m_field_rect - это поле плюс margins, т.е. то, что должно входить в экран
    // при масштабировании. Больше SceneRect на 10%
    m_field_rect = rect;
    m_field_rect.setWidth(m_field_rect.width() * 1.1);
    m_field_rect.setHeight(m_field_rect.height() * 1.1);

    // Рассчитаем прямоугольник сообщения
    m_message_rect = QRectF(0, 0, m_field_rect.width() * 0.6, m_field_rect.height() * 0.6);
    // Центрируем прямоугольник
    m_message_rect.moveCenter(QPointF(width() / 2, height() / 2));
    // Размер шрифта сообщения в пискелах. Думаю, процентов 7 от высоты
    m_message_font_pixel = static_cast<int>(m_message_rect.height() * 0.07);
}

// ------------------------------------------------------------------------------------------------
// Выделить или не выделить плитку
void CScene::selectTile(int x, int y, bool selected) const
{
    m_bases_list[m_field->getIndex(x, y)]->setElementId(
        selected ? tiles_manager->getSelectedBaseName() : tiles_manager->getBaseName()
    );
}

void CScene::selectTile(const Tile &tile, bool selected) const
{
    selectTile(tile.x(), tile.y(), selected);
}

// ------------------------------------------------------------------------------------------------
// Показать сообщение
void CScene::showMessage(const QString &message, bool is_hide_tiles)
{
    // Скрыть все плитки
    if (is_hide_tiles) setTilesVisible(false);
    // Вывести сообщение
    m_message = message;
}

// ------------------------------------------------------------------------------------------------
// Скрыть сообщение
void CScene::hideMessage(bool is_show_tiles)
{
    if (is_show_tiles) setTilesVisible(true);
    m_message.clear();
}

// ------------------------------------------------------------------------------------------------
// Нажата левая клавиша мыши
void CScene::mouseLeft(QPointF point)
{
    // Получим костяшку по координатам
    auto tile = getTileIndex(point);

    // Если кликнули мимо
    if (tile.x() == -1 || m_field->getTileType(tile) == -1) {
        clearSelected();
    } else {
        // Если выбранных нет - выделить ее
        if (m_selected.isEmpty()) {
            addSelected(tile);
        } else {
            // Если заявок выбрано больше одной - все очистить, нужную выделить
            if (m_selected.size() > 1) {
                clearSelected();
                addSelected(tile);
            } else {
                // Выбрана ровно одна заявка, если она уже выделена - убрать выделение
                if (m_selected.front() == tile) {
                    clearSelected();
                } else {
                    // Проверить, совпадает ли тип
                    if (m_field->getTileType(tile) == m_field->getTileType(m_selected.front())) {
                        // Пробуем соединить между собой
//                        Connect(TilePair(m_selected.front(), tile));
                    } else {
                        // Тип не совпадает, кликнута на другую костяшку
                        // Убрать выделение
                        clearSelected();
                    }
                }
            }
        }
    }
}

// ------------------------------------------------------------------------------------------------
// Нажата правая клавиша мыши
void CScene::mouseRight(QPointF point)
{
    clearSelected();

    auto tile = getTileIndex(point);
    if (tile.x() == -1) return;

    auto curr_type = m_field->getTileType(tile);
    for (int i = 0; i < m_field->tilesCount(); ++i) {
        if (m_field->getTileType(i) == curr_type)
            addSelected(m_field->getTile(i));
    }
}

// ------------------------------------------------------------------------------------------------
// Отобразить фоновое изображение
void CScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->drawPixmap(rect, m_bg_pixmap, m_bg_pixmap.rect());
}

// ------------------------------------------------------------------------------------------------
// Вывод после отображения фона и всех костяшек
void CScene::drawForeground(QPainter *painter, const QRectF &)
{
    if (!m_message.isEmpty())
        paintMessage(painter);
}

// ------------------------------------------------------------------------------------------------
// Показать/скрыть все плитки
void CScene::setTilesVisible(bool visible)
{
    for (auto base : m_bases_list) base->setVisible(visible);
    for (auto tile : m_tiles_list) tile->setVisible(visible);
}

// ------------------------------------------------------------------------------------------------
void CScene::paintMessage(QPainter *painter) const
{
    QPen pen(Qt::black);
    pen.setWidth(1);
    pen.setColor(Qt::white);
    painter->setPen(pen);
    painter->setBrush(QColor::fromRgb(50, 50, 50, 150));
    painter->drawRoundedRect(m_message_rect, 10, 10);

    QFont font;
    font.setPixelSize(m_message_font_pixel);
    painter->setFont(font);

    QTextOption to;
    to.setAlignment(Qt::AlignCenter);

    painter->drawText(m_message_rect, m_message, to);
}

// ------------------------------------------------------------------------------------------------
// Рассчитать костяшку по координатам виджета
Tile CScene::getTileIndex(const QPointF point)
{
    if (!sceneRect().contains(point))
        return Tile(-1, -1);

    int x = static_cast<int>(point.x() / tiles_manager->tileSize().width());
    int y = static_cast<int>(point.y() / tiles_manager->tileSize().height());

    return Tile(x, y);
}

// ------------------------------------------------------------------------------------------------
// Очистить выбранные костяшки
void CScene::clearSelected()
{
    for (const auto &tile : m_selected) {
        selectTile(tile, false);
    }
    m_selected.clear();
}

// ------------------------------------------------------------------------------------------------
void CScene::addSelected(const Tile &tile)
{
    m_selected.append(tile);
    selectTile(tile, true);
}

