
#include "cscene.h"

#include "csettings.h"
#include "cbackgroundmanager.h"
#include "ctilesmanager.h"
#include "cfield.h"

#include <QSvgRenderer>
#include <QTimerEvent>

CScene::CScene(CField *field, QObject *parent) : QGraphicsScene(parent),
    m_field(field)
{
}

// ------------------------------------------------------------------------------------------------
void CScene::newGame()
{
    // Заполнить сцену костяшками
    fillScene();
    // Прересчитать логические координаты сцены
    recalcScene();
}

// ------------------------------------------------------------------------------------------------
// Заполнить сцену костяшками
void CScene::fillScene()
{
    clear();
    m_tiles_list.clear();
    m_selected.clear();

    QPointF point(0, 0);
    int n = 0;

    for (int y = 0; y < m_field->m_y; ++y) {
        for (int x = 0; x < m_field->m_x; ++x) {

            auto item = createTile(n, point, tiles_manager->tilesNames()[m_field->m_tiles[n]]);

            addItem(item);
            m_tiles_list.append(item);
            ++n;

            point.setX(point.x() + tiles_manager->tileSize().width());
        }
        point.setX(0);
        point.setY(point.y() + tiles_manager->tileSize().height());
    }
}

// ------------------------------------------------------------------------------------------------
// Создать плитку (основа + плитка)
QGraphicsSvgItem *CScene::createTile(int n, QPointF point, const QString &element_id)
{
    auto item_base = new QGraphicsSvgItem;
    item_base->setSharedRenderer(tiles_manager->currentRenderer());
    item_base->setElementId(tiles_manager->getBaseName());
    item_base->setPos(point);
    item_base->setZValue(n);

    // Как подчиненный item
    auto item_tile = new QGraphicsSvgItem(item_base);
    item_tile->setSharedRenderer(tiles_manager->currentRenderer());
    item_tile->setElementId(element_id);

    return item_base;
}

// ------------------------------------------------------------------------------------------------
// Прересчитать логические координаты сцены
void CScene::recalcScene()
{
    // Размеры поля костяшек (без тени)
    m_tiles_size = QSizeF(tiles_manager->tileSize().width() * m_field->m_x,
                          tiles_manager->tileSize().height() * m_field->m_y);

    // Реальная ширина/высота поля с учетом тени последних костяшек
    auto w =  m_tiles_size.width() + tiles_manager->baseSize().width() - tiles_manager->tileSize().width();
    auto h =  m_tiles_size.height() + tiles_manager->baseSize().height() - tiles_manager->tileSize().height();
    m_field_rect = QRectF(0, 0, w, h);

    // Это и есть размер поля, начинается с (0,0)
    setSceneRect(m_field_rect);

    // m_field_rect - это поле плюс margins, т.е. то, что должно входить в экран
    // при масштабировании. Больше SceneRect на 10%
    m_viewport_rect = m_field_rect;
    m_viewport_rect.setWidth(m_viewport_rect.width() * 1.1);
    m_viewport_rect.setHeight(m_viewport_rect.height() * 1.1);
}

// ------------------------------------------------------------------------------------------------
// Выделить или не выделить плитку
void CScene::selectTile(int x, int y, bool selected) const
{
    m_tiles_list[m_field->getIndex(x, y)]->setElementId(
        selected ? tiles_manager->getSelectedBaseName() : tiles_manager->getBaseName()
    );
}

void CScene::selectTile(const Tile &tile, bool selected) const
{
    selectTile(tile.x(), tile.y(), selected);
}

// ------------------------------------------------------------------------------------------------
// Нажата левая клавиша мыши
void CScene::mouseLeft(QPointF point)
{
    // Получим костяшку по координатам
    auto tile = getTileIndex(point);
    if (tile.x() == -1) {
        clearSelected();
        return;
    }
    auto curr_type = m_field->getTileType(tile);
    if (curr_type == -1) {
        clearSelected();
        return;
    }

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
                    m_field->Connect(TilePair(m_selected.front(), tile));
                } else {
                    // Тип не совпадает, кликнута на другую костяшку
                    // Убрать выделение
                    clearSelected();
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
    if (curr_type == -1) return;

    for (int i = 0; i < m_field->m_tiles_count; ++i) {
        if (m_field->getTileType(i) == curr_type)
            addSelected(m_field->getTile(i));
    }
}

// ------------------------------------------------------------------------------------------------
// Вернуть левый верхний угол ячейки
const QPointF CScene::getTilePos(const Tile &tile) const
{
    return QPointF(tile.x() * tiles_manager->tileSize().width(),
                   tile.y() * tiles_manager->tileSize().height());
}

const QPointF CScene::getTilePos(const int index) const
{
    return getTilePos(m_field->getTile(index));
}

// ------------------------------------------------------------------------------------------------
// Вернуть центр ячейки
const QPointF CScene::getTileCenter(const Tile &tile) const
{
    auto x = static_cast<int>((tile.x() + 0.5) * tiles_manager->tileSize().width());
    auto y = static_cast<int>((tile.y() + 0.5) * tiles_manager->tileSize().height());

    return Tile(x, y);
}

// ------------------------------------------------------------------------------------------------
// Показать/скрыть все плитки
void CScene::setTilesVisible(bool visible)
{
    for (auto &tile : m_tiles_list)
        if (tile) tile->setVisible(visible);
}

// ------------------------------------------------------------------------------------------------
// Заполнить список координат пути в координатах сцены
void CScene::setPathCoords()
{
    // Формируем путь
    m_path_coords.clear();
    // Сформируем список Rect в координатах сцены, которые нужно перерисовать
    for (int i = 0; i < m_field->m_path.size(); ++i) {
        m_path_coords.append(getTileCenter(m_field->m_path[i]));
        // Шаманим с координатами, чтобы они не сильно вылезали за границу
        correctPoint(m_path_coords.back());
    }
}

// ------------------------------------------------------------------------------------------------
// Рассчитать костяшку по координатам виджета
Tile CScene::getTileIndex(const QPointF point)
{
    auto rect = sceneRect();
    rect.setWidth(rect.width() - tiles_manager->shadowWidht());
    rect.setHeight(rect.height() - tiles_manager->shadowHeight());
    if (!rect.contains(point))
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

// ================================================================================================
// Шаманство с координатами линий, чтобы не сильно уходили за границу поля
void CScene::correctPoint(QPointF &point) const
{
    // Примерно треть костяшки по ширине для граничных костяшек
    auto offset = tiles_manager->tileSize().width() / 3;
    if (point.x() < 0) point.setX(-offset);
    if (point.x() > m_tiles_size.width()) point.setX(m_tiles_size.width() + offset);
    if (point.y() < 0) point.setY(-offset);
    if (point.y() > m_tiles_size.height()) point.setY(m_tiles_size.height() + offset);
}

// ------------------------------------------------------------------------------------------------
// Снять костяшки
void CScene::removeTiles(const TilePair &tiles)
{
    // Удалить QGraphicsSvgItem
    auto index = m_field->getIndex(tiles.first);
    removeItem(m_tiles_list[index]);
    m_tiles_list[index] = nullptr;
    index = m_field->getIndex(tiles.second);
    removeItem(m_tiles_list[index]);
    m_tiles_list[index] = nullptr;

    if (!settings->isGravity()) return;

    // Отдельный случай - если костяшки лежат друг над другом по вертикали
    if (tiles.first.x() == tiles.second.x()) {
        // Просто сдвинуть колонку начиная с верхней
        columnMoveDown(tiles.first.y() < tiles.second.y() ? tiles.first : tiles.second);
        columnMoveDown(tiles.first.y() < tiles.second.y() ? tiles.second : tiles.first);
    } else {
        // Снять обычным образом
        columnMoveDown(tiles.first);
        columnMoveDown(tiles.second);
    }
}

// ------------------------------------------------------------------------------------------------
// Сдвинуть колонку вниз
void CScene::columnMoveDown(const Tile &tile)
{
    auto index = m_field->getIndex(tile);
    forever {
        auto up_index = index - m_field->m_x;
        // Смещаем
        if (up_index < 0 || !m_tiles_list[up_index]) break;
        // Переместим позицию и ZValue
        m_tiles_list[up_index]->setPos(getTilePos(index));
        m_tiles_list[up_index]->setZValue(index);
        // Изменение в поле
        m_tiles_list[index] = m_tiles_list[up_index];
        m_tiles_list[up_index] = nullptr;
        index -= m_field->m_x;
    }
}

// ------------------------------------------------------------------------------------------------
// Сдвинуть колонку вверх (для undo)
void CScene::columnMoveUp(const Tile &tile)
{
    auto index = m_field->getIndex(Tile(tile.x(), 0));
    auto finish_index = m_field->getIndex(tile);

    Q_ASSERT_X(!m_tiles_list[index], "CField::unshiftColumn", "Not empty tile");

    forever {
        auto down_index = index + m_field->m_x;
        // Условие выхода: если достигнута ячейка
        if (index == finish_index) break;
        // Если нижняя ячейка не пустая - копируем ее вверх
        if (m_tiles_list[down_index]) {
            // Перемещаем позицию
            m_tiles_list[down_index]->setPos(getTilePos(index));
            m_tiles_list[down_index]->setZValue(index);
            // Перемещаем в поле
            m_tiles_list[index] = m_tiles_list[down_index];
        }

        index += m_field->m_x;
    }
}

// ------------------------------------------------------------------------------------------------
void CScene::slotUndo()
{
    // Вернуть два костяшки в поле
    auto undo_item = m_field->doUndo();

    auto &tile1 = std::get<0>(undo_item);
    auto &tile2 = std::get<1>(undo_item);
    auto element_id = tiles_manager->tilesNames()[std::get<2>(undo_item)];

    // Сдвинем колонки вверх
    if (settings->isGravity()) {
          // Отдельный случай - если костяшки лежат друг над другом по вертикали
          if (tile1.x() == tile2.x()) {
              // Просто сдвинуть колонку вверх начиная с нижней
              columnMoveUp(tile1.y() > tile2.y() ? tile1 : tile2);
              columnMoveUp(tile1.y() > tile2.y() ? tile2 : tile1);
          } else {
              // Снять обычным образом
              columnMoveUp(tile1);
              columnMoveUp(tile2);
          }
    }

    // Создать новые SvgItem
    auto point1 = getTilePos(tile1);
    auto point2 = getTilePos(tile2);
    auto index1 = m_field->getIndex(tile1);
    auto index2 = m_field->getIndex(tile2);

    auto item1 = createTile(index1, point1, element_id);
    auto item2 = createTile(index2, point2, element_id);

    addItem(item1);
    addItem(item2);

    // Поместим их в m_tiles_list в нужное место
    m_tiles_list[index1] = item1;
    m_tiles_list[index2] = item2;
}

// ------------------------------------------------------------------------------------------------
void CScene::slotRedo()
{
    auto tiles = m_field->doRedo();

    // Удалим эти костяшки из сцены
    removeTiles(tiles);
}
