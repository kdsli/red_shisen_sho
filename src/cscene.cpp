#include "cscene.h"

#include "csettings.h"
#include "cbackgroundmanager.h"
#include "ctilesmanager.h"
#include "cfield.h"

#include <QFileInfo>
#include <QSvgRenderer>
#include <QTimerEvent>

#include <QDebug>


CScene::CScene(CField *field, QObject *parent) : QGraphicsScene(parent),
    m_field(field)
{
    connect(m_field, &CField::signalStartConnect, this, &CScene::slotStartConnect);
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
    m_path_type = ptNone;
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

            auto item_base = new QGraphicsSvgItem;
            item_base->setSharedRenderer(tiles_manager->currentRenderer());
            item_base->setElementId(tiles_manager->getBaseName());
            item_base->setPos(point);
            item_base->setZValue(n);

            // Как подчиненный item
            auto item_tile = new QGraphicsSvgItem(item_base);
            item_tile->setSharedRenderer(tiles_manager->currentRenderer());
            item_tile->setElementId(tiles_manager->tilesNames()[m_field->m_tiles[n++]]);

            addItem(item_base);
            m_tiles_list.append(item_base);

            point.setX(point.x() + tiles_manager->tileSize().width());
        }
        point.setX(0);
        point.setY(point.y() + tiles_manager->tileSize().height());
    }
}

// ------------------------------------------------------------------------------------------------
// Прересчитать логические координаты сцены
void CScene::recalcScene()
{
    // Реальная ширина/высота поля с учетом тени последних костяшек
    auto w = tiles_manager->tileSize().width() * m_field->m_x +
            (tiles_manager->baseSize().width() - tiles_manager->tileSize().width());
    auto h = tiles_manager->tileSize().height() * m_field->m_y +
            (tiles_manager->baseSize().height() - tiles_manager->tileSize().height());
    auto rect = QRectF(0, 0, w, h);

    // Это и есть размер поля, начинается с (0,0)
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
    m_tiles_list[m_field->getIndex(x, y)]->setElementId(
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
    setTilesVisible(is_show_tiles);
    m_message.clear();
    update();
}

// ------------------------------------------------------------------------------------------------
// Показать hint
void CScene::showHint()
{
    if (m_path_type != ptNone) return;

    clearSelected();
    // В m_field.path() лежит текущая подсказка
    m_path_type = ptHint;
    doStartPath(m_field->m_hint_tiles);
}

// ------------------------------------------------------------------------------------------------
// Нажата левая клавиша мыши
void CScene::mouseLeft(QPointF point)
{
    // Если перерисовывается путь - мышь недоступна
    if (m_path_type != ptNone) return;

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
}

// ------------------------------------------------------------------------------------------------
// Нажата правая клавиша мыши
void CScene::mouseRight(QPointF point)
{
    if (m_path_type != ptNone) return;

    clearSelected();

    auto tile = getTileIndex(point);
    if (tile.x() == -1) return;

    auto curr_type = m_field->getTileType(tile);
    for (int i = 0; i < m_field->m_tiles_count; ++i) {
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
    if (m_path_type != ptNone)
        paintPath(painter);
}

// ------------------------------------------------------------------------------------------------
// Сработал таймер
void CScene::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_path_timer) {
        doFinishPath();
    }
    if (event->timerId() == m_demostration_timer) {
        doDemonstration();
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
        if (tile)
            tile->setVisible(visible);
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

// ------------------------------------------------------------------------------------------------
// Нарисовать путь
void CScene::paintPath(QPainter *painter)
{
    QPen pen(Qt::red);
    // Толщина линии в 10% от высоты костяшки
    pen.setWidth(static_cast<int>(tiles_manager->tileSize().height() / 10));
    pen.setCapStyle(Qt::RoundCap);

    painter->setPen(pen);
    for (int i = 0; i < m_path_coords.size() - 1; ++i)
        painter->drawLine(m_path_coords[i], m_path_coords[i + 1]);
}

// ------------------------------------------------------------------------------------------------
// Запустить таймер рисования пути
void CScene::doStartPath(const TilePair &tiles)
{
    // Формируем путь
    m_path_coords.clear();
    // Сформируем список Rect в координатах сцены, которые нужно перерисовать
    for (int i = 0; i < m_field->m_path.size(); ++i)
        m_path_coords.append(getTileCenter(m_field->m_path[i]));

    // Сохраним костяшки и запустим таймер
    m_deleted_tiles = tiles;
    m_path_timer = startTimer(settings->timeDelay());

    // Сказать board'у перерисовать путь
    emit signalPaintPath();
}

// ------------------------------------------------------------------------------------------------
// Окончание таймера пути
void CScene::doFinishPath()
{
    m_selected.clear();

    Q_ASSERT(m_path_timer != -1);
    killTimer(m_path_timer);
    m_path_timer = -1;

    // Сказать board'у перерисовать путь
    emit signalPaintPath();

    // Для окончания снятия костяшек
    if (m_path_type == ptPath) {
        m_field->m_path.clear();

        // Удалить костяшки
        removeTiles(m_deleted_tiles);

        // Проверим состояние программы
        auto result = m_field->getGameStatus();
        if (result == vsNotVariant || result == vsVictory)
            emit signalVariantStatus(result);

        m_path_type = ptNone;
    }

    // Для подсказки
    if (m_path_type == ptHint) {
        m_path_type = ptNone;
        // Сказать board'у перерисовать путь
        emit signalPaintPath();
    }

    // Для демонстрации PathType не меняется

    // Статусная строка
    //    emit signalUpdateInfo();
}

// ------------------------------------------------------------------------------------------------
// Поле сказало, что нужно удалить костяшки
void CScene::slotStartConnect(const TilePair &tiles)
{
    m_path_type = ptPath;

    // Выделить вторую костяшку
    addSelected(tiles.second);

    // Стартуем показ пути
    doStartPath(tiles);
}

// ------------------------------------------------------------------------------------------------
// Снять костяшки
void CScene::removeTiles(const TilePair &tiles)
{
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
// Демонстрация (по таймеру)
void CScene::doDemonstration()
{
    auto tiles = m_field->m_right_pathes[m_demostration_index];

    // Старый путь
    if (m_demostration_index > 0) {
        m_field->m_path.clear();
        m_path_coords = m_old_path_coords;
        // Сказать board'у перерисовать путь
        emit signalPaintPath();
        // Снимем ячейки
        clearDemostrationTiles(m_field->m_right_pathes[m_demostration_index-1]);
    }

    // Находим путь
    m_field->checkConnect(m_field->m_tiles, tiles);

    // Формируем путь из m_field->m_path
    m_path_coords.clear();
    // Сформируем список Rect в координатах сцены, которые нужно перерисовать
    for (int i = 0; i < m_field->m_path.size(); ++i)
        m_path_coords.append(getTileCenter(m_field->m_path[i]));
    m_old_path_coords = m_path_coords;
    // Сказать board'у перерисовать путь
    emit signalPaintPath();

    // Проверка окончания
    ++m_demostration_index;
        if (m_demostration_index == m_field->m_right_pathes.size()) {
        closeDemonstration();
    }
}

// ------------------------------------------------------------------------------------------------
// Снять костяшки для демонстрации
void CScene::clearDemostrationTiles(const TilePair &tiles)
{
    // Снимем в поле
    m_field->clearTiles(m_field->m_tiles, tiles);
    // Снимем в сцене
    removeTiles(tiles);
}

// ------------------------------------------------------------------------------------------------
void CScene::closeDemonstration()
{
    // Очистить поле
    m_field->m_tiles.clear();
    clear();

    // Убрать путь
    m_field->m_path.clear();
    m_path_coords.clear();
    emit signalPaintPath();

    Q_ASSERT(m_demostration_timer != -1);
    killTimer(m_demostration_timer);
    m_demostration_timer = -1;
}

// ------------------------------------------------------------------------------------------------
// Начать демонстрацию
void CScene::startDemonstration()
{
    m_path_type = ptDemostration;

    // Текущий номер демонстрации - индекс в массиве m_field->m_right_pathes
    m_demostration_index = 0;

    // Запускаем таймер демонстрации
    m_demostration_timer = startTimer(settings->timeDelay());
}
