#include "cfield.h"

#include "csettings.h"

#include <random>

CField::CField(CTileSet *ts, QObject *parent) : QObject(parent),
    m_ts(ts)
{
    // Заполним массив типов полей
    m_field_types.insert(fz14x6, {14, 6, 4});
    m_field_types.insert(fz16x9, {16, 9, 4});
    m_field_types.insert(fz18x8, {18, 8, 4});
    m_field_types.insert(fz24x12, {24, 12, 8});
    m_field_types.insert(fz26x14, {26, 14, 8});
    m_field_types.insert(fz30x16, {30, 16, 12});

    // Максимальное количество выделенных элементов
    m_selected.reserve(12);
    m_path.reserve(3);

    newGame(settings->currentGameType());
}

// ================================================================================================
// Новое поле (новая игра)
void CField::newGame(GameType field_type)
{
    m_selected.clear();
    m_path.clear();
    m_repaint_list.clear();

    // Данные поля
    m_x = m_field_types[field_type].x;
    m_y = m_field_types[field_type].y;
    m_tile_count = m_x * m_y;
    m_count_in_type = m_field_types[field_type].count;

    // Создадим последовательный список чисел
    Field v(m_tile_count);
    // Заполним его последовательными числами
    std::iota(v.begin(), v.end(), 0);
    // Перемешаем случайным образом
    shuffleField(v);

    // Массив костяшек поля - установим размер
    m_tiles.resize(m_tile_count);

    m_tiles.fill(-1);

    // Распределяем костяшки
    // Возможно, что типов костяшек не хватит заполнить все поле (есть такие поля)
    // тогда начинаем заполнять с начала оставшиеся поля
    int n = 0, k = 0, type = 0;
    int in_type = m_count_in_type / 2;
    int max_type = m_ts->pixmaps().size();
    forever {
        if (n == m_tile_count) break;
        m_tiles[v[n++]] = type;
        m_tiles[v[n++]] = type;
        if (++k == in_type) {
            k = 0;
            if (++type == max_type)
                type = 0;
        }
    }

    if (settings->isDecision()) {
        // Перемешать так, чтобы набор костяшек потенциально можно решить
        shuffleDecisionVariant();
    }

    // Сохраним поле для возможности пересоздания
    m_saved_field = m_tiles;

    doNewGame();
}

// ================================================================================================
// Очистить поле (используется после прерывания демонстрации)
void CField::clearGame()
{
    m_tiles.fill(-1);
    doNewGame();
}

// ================================================================================================
// Повторить игру
void CField::repeatGame()
{
    m_tiles = m_saved_field;
    doNewGame();
}

// ================================================================================================
// Новая игра, общие настройки
void CField::doNewGame()
{
    m_selected.clear();
    m_path.clear();
    m_undo.clear();
    m_current_undo = -1;
    checkStatusUndoRedo();
}

// ================================================================================================
// Перемешать массив
void CField::shuffleField(Field &field) const
{
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    auto rnd = std::default_random_engine(seed);
    std::shuffle(field.begin(), field.end(), rnd);
}

// ================================================================================================
// Добавить в путь порцию из демонстрации
void CField::setDemostrationPath(int index)
{
    m_path.clear();
    Q_ASSERT_X(tileType(m_right_pathes[index].first) == tileType(m_right_pathes[index].second),
            "CField::setDemostrationPath", "Error on demostration - types");
    auto result = checkConnect(m_right_pathes[index]);
    Q_ASSERT_X(result, "CField::setDemostrationPath", "Error on demostration");
}

// ================================================================================================
// Очистить ячейку
void CField::clearTiles(TilePair tiles, Field *collation)
{
    m_tiles[getIndex(tiles.first)] = -1;
    m_tiles[getIndex(tiles.second)] = -1;
    m_current_count -= 2;

    if (!settings->isGravity()) return;

    // Гравитация

    m_repaint_list.clear();
    // Отдельный случай - если костяшки лежат друг над другом по вертикали
    if (tiles.first.x() == tiles.second.x()) {
        // Просто сдвинуть колонку начиная с верхней
        columnMoveDown(tiles.first.y() < tiles.second.y() ? tiles.first : tiles.second, collation);
        columnMoveDown(tiles.first.y() < tiles.second.y() ? tiles.second : tiles.first, collation);
    } else {
        // Снять обычным образом
        columnMoveDown(tiles.first, collation);
        columnMoveDown(tiles.second, collation);
    }

    // Дать сигнал перерисовать repaintList
    if (!m_is_shuffle) emit signalRepaint();
}

// ================================================================================================
// Сдвинуть колонку вниз. Collation нужен для демонстрации в режиме гравитации
void CField::columnMoveDown(const Tile &point, Field *collation)
{
    auto index = getIndex(point);
    forever {
        m_repaint_list.append(getTile(index));
        auto up_index = index - m_x;
        // Если пытаемся сдвинуть верхнюю костяшку или встретили пустую клетку - выходим
        if (up_index < 0) {
            // То вместо нее вставляем пустую и выходим
            m_tiles[index] = -1;
            if (collation) (*collation)[index] = -1;
            m_repaint_list.append(getTile(index));
            break;
        }
        // Смещаем
        m_tiles[index] = m_tiles[up_index];
        if (collation) (*collation)[index] = (*collation)[up_index];
        if (m_tiles[index] == -1) break;
        index -= m_x;
    }
}

// ================================================================================================
// Получить индекс в массиве m_field по координатам
int CField::getIndex(Tile point) const
{
    return point.y() * m_x + point.x();
}

int CField::getIndex(int x, int y) const
{
    return y * m_x + x;
}

// ================================================================================================
// Получить координаты в поле по индексу
Tile CField::getTile(int index) const
{
    return Tile(index % m_x, index / m_x);
}

// ================================================================================================
// Тип костяшки по координатам
int CField::tileType(const Tile &point) const
{
    return m_tiles[getIndex(point)];
}

int CField::tileType(int x, int y) const
{
    return m_tiles[getIndex(x, y)];
}

int CField::tileType(int index) const
{
    return m_tiles[index];
}

// ================================================================================================
// Признак, выделена ли костяшка
bool CField::isSelected(Tile point) const
{
    auto it = std::find(m_selected.cbegin(), m_selected.cend(), point);
    return it != m_selected.cend();
}

bool CField::isSelected(int x, int y) const
{
    return isSelected(Tile(x, y));
}

// ================================================================================================
// Нажатие левой кнопки мыши
void CField::mouseLeft(Tile point)
{
    // Если кликнули мимо
    if (point.x() == -1 || tileType(point) == -1) {
        clearSelected();
    } else {
        // Если выбранных нет - выделить ее
        if (m_selected.isEmpty()) {
            addSelected(point);
        } else {
            // Если заявок выбрано больше одной - все очистить, нужную выделить
            if (m_selected.size() > 1) {
                clearSelected();
                addSelected(point);
            } else {
                // Выбрана ровно одна заявка, если она уже выделена - убрать выделение
                if (m_selected.front() == point) {
                    removeSelected(point);
                } else {
                    // Проверить, совпадает ли тип
                    if (tileType(point) == tileType(m_selected.front())) {
                        // Пробуем соединить между собой
                        Connect(TilePair(m_selected.front(), point));
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

// ================================================================================================
// Нажатие правой кнопки мыши
void CField::mouseRight(Tile point)
{
    // Все выделенные элементы поместить в список обновляемых и очистить
    m_repaint_list = m_selected;
    m_selected.clear();

    // Если кликнули не мимо
    if (point.x() != -1) {
        // Выделить все элементы того же типа
        auto type = tileType(point);
        for (int i = 0; i < m_tile_count; ++i) {
            if (type == m_tiles[i]) {
                m_repaint_list.append(getTile(i));
                m_selected.append(getTile(i));
            }
        }
    }

    emit signalRepaint();
}

// ================================================================================================
// Завершить удаление костяшек - вызывает board
void CField::cancelRemoveTiles(TilePair tiles)
{
    // Заполним Undo
    ++m_current_undo;
    // Обрубим значения, которые осталось после Undo
    m_undo.resize(m_current_undo);
    // Сохраним снятые костяшки в списке ходов
    m_undo.append(UndoItem(tiles.first, tiles.second, m_tiles[getIndex(tiles.first)]));
    checkStatusUndoRedo();

    // Очистим место под костяшки
    clearTiles(tiles);
    // Проверим возможность дальнейшей игры
    if (!checkVariants(tiles)) {
        emit m_current_count == 0 ? signalVictory() : signalNotVariants();
    }
    // Очистим выделенные элементы. Путь будет очищен потом в board'е
    m_selected.clear();
}

// ================================================================================================
// Снять костяшки (для демонстрации)
void CField::clearDemostrationTiles(int index)
{
    // Очистим костяшки
    clearTiles(m_right_pathes[index]);
}

// ================================================================================================
// Проверить наличие вариантов для продолжения
// Одновременно первый найденный вариант будет выдан как подссказка
bool CField::checkVariants(TilePair &tiles)
{
    // Идея: идем не по m_field, а по массиву со случайным образом
    // перемешанными числами от 0 до m_field.size()-1.
    // Этим обеспечим случайный порядок подсказки
    Field v(m_tile_count);
    // Заполним его последовательными числами
    std::iota(v.begin(), v.end(), 0);
    // Перемешаем случайным образом
    shuffleField(v);

    // Идем по m_field и для каждой костяшки будем проверять возможность
    // соединения со всеми костяшками данного типа, лажащих после текущей
    for (auto k = 0; k < m_tile_count; ++k) {
        auto i = v[k];
        auto type = m_tiles[i];
        if (type == -1) continue;
        auto first_point = getTile(i);
        // Найти все tile того же типа
        for (auto n = 0; n < m_tile_count; ++n) {
            if (m_tiles[n] != type || n == i) continue;

            // Проверяем возможность
            auto next_point = getTile(n);
            if (checkConnect(TilePair(first_point, next_point))) {
                tiles.first = first_point;
                tiles.second = next_point;
                return true;
            }
        }
    }
    return false;
}

// ================================================================================================
// Проверка возможности соединения
bool CField::checkConnect(TilePair tiles)
{
    Q_ASSERT_X(tileType(tiles.first) == tileType(tiles.second), "CField::checkConnect", "Types is not equivalent");

    m_path.clear();

    // Идея следующая: в начале обработаем прямое соединение (по вертикали и горизонтали)
    if (tiles.first.y() == tiles.second.y() && checkDirectHorz(tiles))
        return true;
    if (tiles.first.x() == tiles.second.x() && checkDirectVert(tiles))
        return true;
    // При условии что они не лежат на одной прямой
    if (tiles.first.x() != tiles.second.x() && tiles.first.y() != tiles.second.y()) {
        // Проверяем, можно ли соединить двумя линиями
        if (checkDoubleLines(tiles)) return true;
        // Далее все тремя линиями
        // Линии, идущие навстречу друг другу (так же по горизонтали и вертикали),
        // при условии что они не лежат на одной прямой
        if (checkTowardsHorz(tiles))
            return true;
        if (checkTowardsVert(tiles))
            return true;
    }
    // А уже затем идущие в одном направлении
    // По горизонтали
    if (tiles.first.y() != tiles.second.y() && checkDirectionHorz(tiles))
        return true;
    // По вертикали
    if (tiles.first.x() != tiles.second.x() && checkDirectionVert(tiles))
        return true;

    return false;
}

// ================================================================================================
// Соединяем костяшки
void CField::Connect(TilePair tiles)
{
    Q_ASSERT_X(tiles.first != tiles.second, "CBoard::checkConnect", "index1 == index2");

    // Проверяем на возможность соединения
    if (!checkConnect(tiles)) {
        // Соединить нельзя - просто выходим
        return;
    } else {
        // Выделить index2
        m_selected.append(tiles.second);
        // Можно соединить - скажем владельцу перерисовать путь
        emit signalStartConnect(tiles);
    }
}

// ================================================================================================
// Проверка, можно ли соединить прямой горизонтальной линией
bool CField::checkDirectHorz(TilePair tiles)
{
    if (checkHorzLine(tiles.first.x(), tiles.second.x(), tiles.first.y(), true)) {
        // Формируем path и выходим
        m_path.append(tiles.first);
        m_path.append(tiles.second);
        return true;
    }

    return false;
}

// ================================================================================================
// Проверка, можно ли соединить прямой вертикальной линией
bool CField::checkDirectVert(TilePair tiles)
{
    if (checkVertLine(tiles.first.x(), tiles.first.y(), tiles.second.y(), true)) {
        // Формируем path и выходим
        m_path.append(tiles.first);
        m_path.append(tiles.second);
        return true;
    }

    return false;
}

// ================================================================================================
// Проверить, можно ли соединить двумя линиями
bool CField::checkDoubleLines(TilePair tiles)
{
    // В начале вертикальная, затем горизонтальная
    if (checkVertLine(tiles.first.x(), tiles.first.y(), tiles.second.y(), false) &&
            checkHorzLine(tiles.first.x(), tiles.second.x(), tiles.second.y(), true)) {
        // Формируем path и выходим
        m_path.append(tiles.first);
        m_path.append(Tile(tiles.first.x(), tiles.second.y()));
        m_path.append(tiles.second);
        return true;
    }

    // В начале горизонтальная, затем вертикальная
    if (checkHorzLine(tiles.first.x(), tiles.second.x(), tiles.first.y(), false) &&
            checkVertLine(tiles.second.x(), tiles.first.y(), tiles.second.y(), true)) {
        // Формируем path и выходим
        m_path.append(tiles.first);
        m_path.append(Tile(tiles.second.x(), tiles.first.y()));
        m_path.append(tiles.second);
        return true;
    }
    return false;
}

// ================================================================================================
// Проверка, можно ли соединить линиями, идущими навстречу
// Горизонтальная/вертикальная/горизонтальная
bool CField::checkTowardsHorz(TilePair tiles)
{
    // Определим, в какую строну идти - index1 слева или справа от index2
    auto direct_x = tiles.first.x() < tiles.second.x() ? 1 : -1;

    // Проверим, насколько можно далеко уйти по горизонтали от костяшек
    auto x1 = farHorz(tiles.first, direct_x, tiles.second.x());
    auto x2 = farHorz(tiles.second, -direct_x, tiles.first.x());

    // Если соединение вырожденное (диапазоны не пересекаются), то выходим
    if ((x2 - x1) * direct_x > 0) return false;

    // Соединение по вертикали может идти только в диаппазоне x1 и x2
    // Идем в этом диапазона и проверяем доступность вертикальной линии
    // Первая же доступная вертикальная линия и будет нужной
    for (auto x = x2; x != x1 + direct_x; x += direct_x) {
        if (checkVertLine(x, tiles.first.y(), tiles.second.y(), false)) {
            // Формируем path и выходим
            m_path.append(tiles.first);
            m_path.append(Tile(x, tiles.first.y()));
            m_path.append(Tile(x, tiles.second.y()));
            m_path.append(tiles.second);
            return true;
        }
    }

    return false;
}

// ================================================================================================
// Проверка, можно ли соединить линиями, идущими навстречу
// Вертикальная/горизонтальная/вертикальная
bool CField::checkTowardsVert(TilePair tiles)
{
    // Определим, в какую строну идти - index1 слева или справа от index2
    auto direct_y = tiles.first.y() < tiles.second.y() ? 1 : -1;

    // Проверим, насколько можно далеко уйти по горизонтали от костяшек
    auto y1 = farVert(tiles.first, direct_y, tiles.second.y());
    auto y2 = farVert(tiles.second, -direct_y, tiles.first.y());

    // Если соединение вырожденное (диапазоны не пересекаются), то выходим
    if ((y2 - y1) * direct_y > 0) return false;

    // Соединение по вертикали может идти только в диаппазоне x1 и x2
    // Идем в этом диапазона и проверяем доступность вертикальной линии
    // Первая же доступная вертикальная линия и будет нужной
    for (auto y = y2; y != y1 + direct_y; y += direct_y) {
        if (checkHorzLine(tiles.first.x(), tiles.second.x(), y, false)) {
            // Формируем path и выходим
            m_path.append(tiles.first);
            m_path.append(Tile(tiles.first.x(), y));
            m_path.append(Tile(tiles.second.x(), y));
            m_path.append(tiles.second);
            return true;
        }
    }

    return false;
}

// ================================================================================================
// Проверить, можно ли соединить тремя горизонтальными линиями,
// идущими в одном направлении
bool CField::checkDirectionHorz(TilePair tiles)
{
    // Направо

    // Проверим, насколько можно далеко уйти по горизонтали от костяшек
    auto x1 = farHorz(tiles.first, 1, m_x + 1);
    auto x2 = farHorz(tiles.second, 1, m_x + 1);

    // Идем от минимального значения до максимального и проверяем вертикаль
    auto x_min = qMax(tiles.first.x(), tiles.second.x());
    auto x_max = qMin(x1, x2);
    for (auto x = x_min; x <= x_max; ++x) {
        if (checkVertLine(x, tiles.first.y(), tiles.second.y(), false)) {
            // Формируем path и выходим
            m_path.append(tiles.first);
            m_path.append(Tile(x, tiles.first.y()));
            m_path.append(Tile(x, tiles.second.y()));
            m_path.append(tiles.second);
            return true;
        }
    }

    // Налево

    // Проверим, насколько можно далеко уйти по горизонтали от костяшек
    x1 = farHorz(tiles.first, -1, -1);
    x2 = farHorz(tiles.second, -1, -1);

    // Идем от максимального значения до минимального и проверяем вертикаль
    x_min = qMin(tiles.first.x(), tiles.second.x());
    x_max = qMax(x1, x2);
    for (auto x = x_min; x >= x_max; --x) {
        if (checkVertLine(x, tiles.first.y(), tiles.second.y(), false)) {
            // Формируем path и выходим
            m_path.append(tiles.first);
            m_path.append(Tile(x, tiles.first.y()));
            m_path.append(Tile(x, tiles.second.y()));
            m_path.append(tiles.second);
            return true;
        }
    }

    return false;
}

// ================================================================================================
// Проверить, можно ли соединить тремя вертикальными линиями,
// идущими в одном направлении
bool CField::checkDirectionVert(TilePair tiles)
{
    // Вниз

    // Проверим, насколько можно далеко уйти по вертикали от костяшек
    auto y1 = farVert(tiles.first, 1, m_y + 1);
    auto y2 = farVert(tiles.second, 1, m_y + 1);

    // Идем от минимального значения до максимального и проверяем горизонталь
    auto y_min = qMax(tiles.first.y(), tiles.second.y());
    auto y_max = qMin(y1, y2);
    for (auto y = y_min; y <= y_max; ++y) {
        if (checkHorzLine(tiles.first.x(), tiles.second.x(), y, false)) {
            // Формируем path и выходим
            m_path.append(tiles.first);
            m_path.append(Tile(tiles.first.x(), y));
            m_path.append(Tile(tiles.second.x(), y));
            m_path.append(tiles.second);
            return true;
        }
    }

    // Вверх

    // Проверим, насколько можно далеко уйти по горизонтали от костяшек
    y1 = farVert(tiles.first, -1, -1);
    y2 = farVert(tiles.second, -1, -1);

    // Идем от максимального значения до минимального и проверяем вертикаль
    y_min = qMin(tiles.first.y(), tiles.second.y());
    y_max = qMax(y1, y2);
    for (auto y = y_min; y >= y_max; --y) {
        if (checkHorzLine(tiles.first.x(), tiles.second.x(), y, false)) {
            // Формируем path и выходим
            m_path.append(tiles.first);
            m_path.append(Tile(tiles.first.x(), y));
            m_path.append(Tile(tiles.second.x(), y));
            m_path.append(tiles.second);
            return true;
        }
    }

    return false;
}

// ================================================================================================
// Проверить горизонтальную линию. Возвращает true, если достигнуть конец и все
// ячейки пустые. Конец должен (true) или не должен (false) быть пустым
// в зависимости от is_last_empty. Начало не проверяется
bool CField::checkHorzLine(int x1, int x2, int y, bool is_last_empty)
{
    // Если выходим за пределы поля - значит истина
    if (y < 0 || y >= m_y) return true;

    auto increment = x1 < x2 ? 1 : -1;
    auto idx = getIndex(x1 + increment, y);
    for (auto x = x1 + increment; x != x2; x += increment, idx += increment) {
        if (m_tiles[idx] != -1) return false;
    }

    // И проверка условия последней костяшки
    return (m_tiles[idx] == -1) ^ is_last_empty;
}

// ================================================================================================
// Проверить горизонтальную линию
bool CField::checkVertLine(int x, int y1, int y2, bool is_last_empty)
{
    // Если выходим за пределы поля - значит истина
    if (x < 0 || x >= m_x) return true;

    auto increment = y1 < y2 ? 1 : -1;
    auto idx = getIndex(x, y1 + increment);
    for (auto y = y1 + increment; y != y2; y += increment, idx += increment * m_x) {
        if (m_tiles[idx] != -1) return false;
    }

    // И проверка условия последней костяшки
    return (m_tiles[idx] == -1) ^ is_last_empty;
}

// ================================================================================================
// Проверить, насколько далеко можно уйти по горизонтали
int CField::farHorz(Tile tile, int direct, int limit)
{
    auto idx = getIndex(tile);
    auto x = tile.x();
    forever {
        idx += direct;
        x += direct;
        // Дошли до конца
        if ((direct < 0 && x < 0) || (direct > 0 && x == m_x)) return x;
        // Дошли до предела или не пустое поле - вернем предыдущую костяшку
        if (m_tiles[idx] != -1 || x == limit) return x - direct;
    }
}

// ================================================================================================
// Проверить, насколько далеко можно уйти по вертикали
int CField::farVert(Tile tile, int direct, int limit)
{
    auto idx = getIndex(tile);
    auto y = tile.y();
    forever {
        idx += direct * m_x;
        y += direct;
        // Дошли до конца
        if ((direct < 0 && y < 0) || (direct > 0 && y == m_y)) return y;
        // Дошли до предела или не пустое поле - вернем предыдущую костяшку
        if (m_tiles[idx] != -1 || y == limit) return y - direct;
    }
}

// ================================================================================================
// Очистить список выделенных элементов
void CField::clearSelected()
{
    m_repaint_list.clear();
    // По всем выделенным элементам - добавить rect в регион обновления
    if (!m_selected.isEmpty()) {
        for (const auto &point : m_selected) {
            m_repaint_list.append(point);
        }
        m_selected.clear();
    }
    emit signalRepaint();
}

// ================================================================================================
// Добавить костяшку в список выделенных
void CField::addSelected(Tile tile)
{
    m_selected.append(tile);
    m_repaint_list.clear();
    m_repaint_list.append(tile);
    emit signalRepaint();
}

// ================================================================================================
// Убрать из выделения. Вызывается, когда всего один элемент
void CField::removeSelected(Tile tile)
{
    Q_ASSERT_X(m_selected.size() == 1, "CBoard::removeSelected", "m_selected.size() != 1");

    m_selected.clear();
    m_repaint_list.clear();
    m_repaint_list.append(tile);
    emit signalRepaint();
}

// ================================================================================================
// Обеспечить решаемыми значениями
void CField::shuffleDecisionVariant()
{
    // Зачем это надо? Потому что поля, сформированные случайным образом,
    // слишком часто являются нерешаемыми.

    // Общая идея такова: создаем копию. Работаем с основным полем (под него
    // заточены все процедуры), а формируем правильный набор в копии.
    // В поле пытаемся удалять все костяшки до тех пор, пока игра зайдет в тупик.
    // перемешиваем оставшиеся костяшки случайным образом и опять снимаем все
    // возможные костяшки. Новые значения костяшек подставляем в копию.
    // И так до тех пор, пока костяшки не закончатся.
    // В этом случае в копии будет правильная комбинация, которую и поместим
    // в основное поле

    m_is_shuffle = true;

    m_current_count = m_tile_count;

    // Копия для накопления правильных результатов
    m_copy_field = m_tiles;
    // Список правильных путей
    m_right_pathes.clear();
    // Список сопоставления старых и новых индексов костяшек
    // Актуально для гравитации
    Field collation(m_tile_count);
    std::iota(collation.begin(), collation.end(), 0);

    // Цикл по снятию костяшек до неснимаемой конфгурации
    while (m_current_count != 0) {
        int old_current_count = m_current_count;
        // Снимаем
        takeOffTiles(collation);
        // Если костяшек не осталось
        if (m_current_count == 0) break;

        // Поместим оставшиеся ячейки в список для того, чтобы перемешать
        Field list;
        for (int i = 0; i < m_tile_count; ++i) {
            if (m_tiles[i] != -1) {
                list.append(i);
            }
        }

        // Существует ситуация, когда перемешивание не дает ничего
        // Это наблюдается когда остается 4 костяшки по диагонали - std::shiffle
        // их не перемешивает. Просто поменяем две оставлшиеся ячейки местами.
        // Для 4-х костяшек это даст результат
        if (m_current_count == old_current_count) {
            qSwap(m_tiles[list[0]], m_tiles[list[2]]);
            qSwap(collation[list[0]], collation[list[2]]);
            qSwap(m_copy_field[collation[list[0]]], m_copy_field[collation[list[2]]]);
        } else {
            // Все остальное перемешаем случайным образом
            shuffleField(list);

            // В списке каждые две ячейки меняем местами. Они указывают на ячейки
            // из m_tiles. Но менять нужно и в m_copy_field, причем в первом поле
            // данные смещены по гравитации, а во втором - нет. В collation
            // для сдвинутых ячеек хранятся их первоначальные координаты (индекс)
            // Соответственно, оттуда их и нужно взять
            // Без гравитации все так же будет работать
            for (int i = 0; i < list.size(); i += 2) {
                // list[i], list[i+1] - индекс ячеек (в m_tiles), которую нужно обменять
                // collation[list[i]], collation[list[i+1]] - тоже в m_copy_field
                qSwap(m_tiles[list[i]], m_tiles[list[i + 1]]);
                qSwap(m_copy_field[collation[list[i]]], m_copy_field[collation[list[i + 1]]]);
            }
        }

        old_current_count = m_current_count;
    }

    // Копию поместим в основное поле
    m_tiles = m_copy_field;
    m_current_count = m_tiles.size();

    // Очистим путь
    m_path.clear();

    m_is_shuffle = false;

}

// ================================================================================================
// Снять с поля все костяшки (в режиме определения решаемых полей)
// Вызвается из fillDecisionField
void CField::takeOffTiles(Field &collation)
{
    TilePair tiles;

    forever {
        // Идем по всем элементам. Если не нашли варианты - выходим
        if (!checkVariants(tiles)) return;
        // Нашли вариант. Cнимаем его костяшки
        clearTiles(tiles, &collation);
        // И сохраним их в списке
        m_right_pathes.append(tiles);
    }
}

// ================================================================================================
// Проверить активность undo/redo и отправить сигнал при изменении
void CField::checkStatusUndoRedo()
{
    // Undo enabled, если есть снятые костяшки, Redo - если текущий не указывает на конец
    emit signalStatusUndoRedo(m_current_undo >= 0, m_current_undo < m_undo.size() - 1);
}

// ================================================================================================
// m_current_undo всегда указывает на костяшку, которой нужно делать undo
// При снятии костяшки m_current_undo будет указывать на последний элемент
// списка m_undo. При redo список не очищается, а костяшки снимаются, но m_current_undo
// сдвигается влево. Таким образом, для redo костяшка для повторения всегда
// будет справа от m_current_undo
void CField::slotUndo()
{
    Q_ASSERT_X(!m_undo.isEmpty(), "CField::undo", "Undo list is empty");

    m_repaint_list.clear();

    // Вытащим две последние костяшки
    auto tile_pair = m_undo[m_current_undo];
    auto tile1 = std::get<0>(tile_pair);
    auto tile2 = std::get<1>(tile_pair);
    auto type = std::get<2>(tile_pair);

    // Если гравитация
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

    // Ячейки вернем на место
    m_tiles[getIndex(tile1)] = type;
    m_tiles[getIndex(tile2)] = type;
    m_current_count += 2;
    // Обе ячейки в группу перерисовки
    m_repaint_list.append(tile1);
    m_repaint_list.append(tile2);

    emit signalRepaint();

    --m_current_undo;

    checkStatusUndoRedo();
}

// ================================================================================================
// Сдвинуть колонку вверх
void CField::columnMoveUp(Tile tile)
{
    auto index = getIndex(Tile(tile.x(), 0));
    auto finish_index = getIndex(tile);
    Q_ASSERT_X(tileType(index) == -1, "CField::unshiftColumn", "Not empty tile");

    forever {
        // Условие выхода: если достигнута ячейка
        if (index == finish_index) {
            m_repaint_list.append(getTile(index));
            break;
        }

        // Если нижняя ячейка не пустая - копируем ее вверх
        if (tileType(index + m_x) != -1) {
            m_tiles[index] = m_tiles[index + m_x];
            m_repaint_list.append(getTile(index));
        }

        index += m_x;
    }
}

// ================================================================================================
void CField::slotRedo()
{
    Q_ASSERT_X(m_current_undo < m_undo.size(), "CField::slotRedo", "Error undo definition");

    // Вытащим пару костяшек, которые нужно убрать
    auto tiles = m_undo[m_current_undo + 1];
    // И удаляем их
    clearTiles(TilePair(std::get<0>(tiles), std::get<1>(tiles)));
    ++m_current_undo;
    checkStatusUndoRedo();
}

