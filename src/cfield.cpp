#include "cfield.h"

#include "csettings.h"
#include "ctilesmanager.h"

#include <random>

CField::CField(QObject *parent) : QObject(parent)
{
}

// ------------------------------------------------------------------------------------------------
// Новое поле
void CField::newGame(int x, int y, int count)
{
    m_x = x;
    m_y = y;
    m_count_in_type = count;
    m_tiles_count = m_x * m_y;
    m_remaining = m_tiles_count;

    m_tiles.resize(m_tiles_count);
    m_tiles.fill(-1);
    m_undo.clear();
    m_current_undo = -1;
    checkStatusUndoRedo();

    // Создадим последовательный список чисел и заполним его последовательными числами
    Field v(m_tiles_count);
    std::iota(v.begin(), v.end(), 0);
    // Перемешаем случайным образом
    shuffleField(v);

    // Распределяем костяшки
    // Возможно, что типов костяшек не хватит заполнить все поле (есть такие поля)
    // тогда начинаем заполнять оставшиеся поля с начала
    int type{0}, n{0}, max_n(m_count_in_type / 2);
    int max_type{tiles_manager->tilesNames().size()};
    for (int i = 0; i < m_tiles_count; i += 2) {
        m_tiles[v[i]] = type;
        m_tiles[v[i+1]] = type;
        if (++n == max_n) {
            n = 0;
            if (++type == max_type) type = 0;
        }
    }

    if (settings->isDecision()) {
        // Перемешать так, чтобы набор костяшек потенциально можно решить
        shuffleDecisionVariant();
    }

    // Сохраним начальное поле
    m_old_tiles = m_tiles;

    // Найдем первый вариант - в пути будет подсказка
    checkVariants(m_tiles, m_hint_tiles);
}

// ------------------------------------------------------------------------------------------------
// Вернуть начальное поле
void CField::restoreField()
{
    m_tiles = m_old_tiles;
    m_remaining = m_tiles_count;
}

// ------------------------------------------------------------------------------------------------
// Соединим две ячейки (если возможно)
void CField::Connect(const TilePair &tiles)
{
    if (!checkConnect(m_tiles, tiles)) return;

    // Заполним Undo
    ++m_current_undo;
    // Обрубим значения, которые осталось после Undo
    m_undo.resize(m_current_undo);
    // Сохраним снятые костяшки в списке ходов
    m_undo.append(UndoItem(tiles.first, tiles.second, m_tiles[getIndex(tiles.first)]));
    checkStatusUndoRedo();

    // Снимем костяшки
    removeTiles(m_tiles, tiles);
    m_remaining -= 2;
    // Соединить можно, в m_path лежит путь для отображения - сказать
    emit signalStartConnect(tiles);
}

// ------------------------------------------------------------------------------------------------
// Проверка состояния игры (есть ли дальше варианты, достигнута ли победа)
VariantStatus CField::getGameStatus()
{
    if (m_remaining == 0) return vsVictory;
    if (!checkVariants(m_tiles, m_hint_tiles)) return vsNotVariant;

    // В пути будет лежать подсказка, а костяшки для подсказки - в m_hint_tiles

    return vsNormal;
}

// ------------------------------------------------------------------------------------------------
// Получить индекс в массиве m_tiles по координатам
int CField::getIndex(int x, int y) const
{
    return y * m_x + x;
}

int CField::getIndex(Tile tile) const
{
    return tile.y() * m_x + tile.x();
}

// ------------------------------------------------------------------------------------------------
// Получить тип костяшки
int CField::getTileType(Tile &tile)
{
    return m_tiles[getIndex(tile)];
}

int CField::getTileType(int index)
{
    return m_tiles[index];
}

// ------------------------------------------------------------------------------------------------
// Получить координаты по индексу
Tile CField::getTile(int index) const
{
    return Tile(index % m_x, index / m_x);
}

// ------------------------------------------------------------------------------------------------
// Перемешать массив
void CField::shuffleField(Field &field) const
{
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    auto rnd = std::default_random_engine(seed);
    std::shuffle(field.begin(), field.end(), rnd);
}

// ------------------------------------------------------------------------------------------------
// Сделать так, чтобы существовал минимум один путь снятия
void CField::shuffleDecisionVariant()
{
    // Зачем это надо? Потому что поля, сформированные случайным образом,
    // слишком часто являются нерешаемыми.

    // Общая идея такова: работаем в копии field. Правильный набор будем формировать
    // еще в одной копии pattern. Еще одна копия collation служит для соответствия
    // старых и новых адресов костяшек, что актуально для гравитации.
    // В поле пытаемся удалять все костяшки до тех пор, пока игра зайдет в тупик.
    // перемешиваем оставшиеся костяшки случайным образом и опять снимаем все
    // возможные костяшки. Новые значения костяшек подставляем в копию pattern.
    // И так до тех пор, пока костяшки не закончатся.
    // В этом случае в копии pattern будет правильная комбинация, которую и поместим
    // в основное поле m_tiles.

    Field field = m_tiles;
    Field pattern = m_tiles;
    // При гравитации при сдвигании костяшек будем сдвигать и collation - там самым
    // сохраним сообтветствие
    Field collation(m_tiles_count);
    std::iota(collation.begin(), collation.end(), 0);

    m_right_pathes.clear();
    int current_count = m_tiles_count;

    // Цикл по снятию костяшек до неснимаемой конфгурации
    forever {
        int old_current_count = current_count;
        // Снимаем костяшки
        takeTilesOff(field, collation, current_count);
        if (current_count == 0) break;

        // Поместим оставшиеся ячейки в список для того, чтобы перемешать
        Field list;
        for (int i = 0; i < m_tiles_count; ++i) {
            if (field[i] != -1) {
                list.append(i);
            }
        }

        // Существует ситуация, когда перемешивание не дает ничего
        // Это наблюдается когда остается 4 костяшки по диагонали и std::shiffle
        // их не перемешивает. Просто поменяем две оставлшиеся ячейки местами.
        // Для 4-х костяшек это даст результат
        if (current_count == old_current_count) {
            qSwap(field[list[0]], field[list[2]]);
            qSwap(collation[list[0]], collation[list[2]]);
            qSwap(pattern[collation[list[0]]], pattern[collation[list[2]]]);
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
                qSwap(field[list[i]], field[list[i + 1]]);
                qSwap(pattern[collation[list[i]]], pattern[collation[list[i + 1]]]);
            }
        }
    }
    // Копию поместим в основное поле
    m_tiles = pattern;

    // Очистим путь
    m_path.clear();
}

// ------------------------------------------------------------------------------------------------
// Снять все возможные костяшки (вызывается из shuffleDecisionVariant)
void CField::takeTilesOff(Field &field, Field &collation, int &current_tile_count)
{
    TilePair tiles;

    forever {
        // Идем по всем элементам. Если не нашли варианты - выходим
        if (!checkVariants(field, tiles)) return;
        // Нашли вариант. Cнимаем его костяшки из обоих полей
        removeTiles(field, tiles);
        removeTiles(collation, tiles);
        current_tile_count -= 2;
        // И сохраним их в списке
        m_right_pathes.append(tiles);
    }
}

// ------------------------------------------------------------------------------------------------
// Проверить наличие вариантов для продолжения
// Одновременно первый найденный вариант будет выдан как подссказка
bool CField::checkVariants(const Field &field, TilePair &tiles)
{
    // Идея: идем не по m_field, а по массиву со случайным образом
    // перемешанными числами от 0 до m_field.size()-1.
    // Этим обеспечим случайный порядок подсказки
    Field v(m_tiles_count);
    // Заполним его последовательными числами
    std::iota(v.begin(), v.end(), 0);
    // Перемешаем случайным образом
    shuffleField(v);

    // Идем по m_field и для каждой костяшки будем проверять возможность
    // соединения со всеми костяшками данного типа, лежащих после текущей
    for (auto k = 0; k < m_tiles_count; ++k) {
        // Костяшку с индексом i будет пытаться соеденить с другими этого же типа
        auto i = v[k];
        auto type = field[i];
        if (type == -1) continue;
        auto first_point = getTile(i);
        // Найти все tile того же типа
        for (auto n = 0; n < m_tiles_count; ++n) {
            if (field[n] != type || n == i) continue;
            // Проверяем возможность
            auto next_point = getTile(n);
            if (checkConnect(field, TilePair(first_point, next_point))) {
                tiles.first = first_point;
                tiles.second = next_point;
                return true;
            }
        }
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Проверка возможности снятия пары костяшек
bool CField::checkConnect(const Field &field, TilePair tiles)
{
    Q_ASSERT_X(field[getIndex(tiles.first)] == field[getIndex(tiles.second)], "CField::checkConnect", "Types is not equivalent");

    m_path.clear();

    // Идея следующая: в начале обработаем прямое соединение (по вертикали и горизонтали)
    if (tiles.first.y() == tiles.second.y() && checkDirectHorz(field, tiles))
        return true;
    if (tiles.first.x() == tiles.second.x() && checkDirectVert(field, tiles))
        return true;
    // При условии что они не лежат на одной прямой
    if (tiles.first.x() != tiles.second.x() && tiles.first.y() != tiles.second.y()) {
        // Проверяем, можно ли соединить двумя линиями
        if (checkDoubleLines(field, tiles)) return true;
        // Далее все тремя линиями
        // Линии, идущие навстречу друг другу (так же по горизонтали и вертикали),
        // при условии что они не лежат на одной прямой
        if (checkTowardsHorz(field, tiles))
            return true;
        if (checkTowardsVert(field, tiles))
            return true;
    }
    // А уже затем идущие в одном направлении
    // По горизонтали
    if (tiles.first.y() != tiles.second.y() && checkDirectionHorz(field, tiles))
        return true;
    // По вертикали
    if (tiles.first.x() != tiles.second.x() && checkDirectionVert(field, tiles))
        return true;

    return false;
}

// ------------------------------------------------------------------------------------------------
// Проверка, можно ли соединить прямой горизонтальной линией
bool CField::checkDirectHorz(const Field &field, TilePair tiles)
{
    if (checkHorzLine(field, tiles.first.x(), tiles.second.x(), tiles.first.y(), true)) {
        // Формируем path и выходим
        m_path.append(tiles.first);
        m_path.append(tiles.second);
        return true;
    }

    return false;
}

// ------------------------------------------------------------------------------------------------
// Проверка, можно ли соединить прямой вертикальной линией
bool CField::checkDirectVert(const Field &field, TilePair tiles)
{
    if (checkVertLine(field, tiles.first.x(), tiles.first.y(), tiles.second.y(), true)) {
        // Формируем path и выходим
        m_path.append(tiles.first);
        m_path.append(tiles.second);
        return true;
    }

    return false;
}

// ------------------------------------------------------------------------------------------------
// Проверить, можно ли соединить двумя линиями
bool CField::checkDoubleLines(const Field &field, TilePair tiles)
{
    // В начале вертикальная, затем горизонтальная
    if (checkVertLine(field, tiles.first.x(), tiles.first.y(), tiles.second.y(), false) &&
            checkHorzLine(field, tiles.first.x(), tiles.second.x(), tiles.second.y(), true)) {
        // Формируем path и выходим
        m_path.append(tiles.first);
        m_path.append(Tile(tiles.first.x(), tiles.second.y()));
        m_path.append(tiles.second);
        return true;
    }

    // В начале горизонтальная, затем вертикальная
    if (checkHorzLine(field, tiles.first.x(), tiles.second.x(), tiles.first.y(), false) &&
            checkVertLine(field, tiles.second.x(), tiles.first.y(), tiles.second.y(), true)) {
        // Формируем path и выходим
        m_path.append(tiles.first);
        m_path.append(Tile(tiles.second.x(), tiles.first.y()));
        m_path.append(tiles.second);
        return true;
    }
    return false;
}

// ------------------------------------------------------------------------------------------------
// Проверка, можно ли соединить линиями, идущими навстречу
// Горизонтальная/вертикальная/горизонтальная
bool CField::checkTowardsHorz(const Field &field, TilePair tiles)
{
    // Определим, в какую строну идти - index1 слева или справа от index2
    auto direct_x = tiles.first.x() < tiles.second.x() ? 1 : -1;

    // Проверим, насколько можно далеко уйти по горизонтали от костяшек
    auto x1 = farHorz(field, tiles.first, direct_x, tiles.second.x());
    auto x2 = farHorz(field, tiles.second, -direct_x, tiles.first.x());

    // Если соединение вырожденное (диапазоны не пересекаются), то выходим
    if ((x2 - x1) * direct_x > 0) return false;

    // Соединение по вертикали может идти только в диаппазоне x1 и x2
    // Идем в этом диапазона и проверяем доступность вертикальной линии
    // Первая же доступная вертикальная линия и будет нужной
    for (auto x = x2; x != x1 + direct_x; x += direct_x) {
        if (checkVertLine(field, x, tiles.first.y(), tiles.second.y(), false)) {
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

// ------------------------------------------------------------------------------------------------
// Проверка, можно ли соединить линиями, идущими навстречу
// Вертикальная/горизонтальная/вертикальная
bool CField::checkTowardsVert(const Field &field, TilePair tiles)
{
    // Определим, в какую строну идти - index1 слева или справа от index2
    auto direct_y = tiles.first.y() < tiles.second.y() ? 1 : -1;

    // Проверим, насколько можно далеко уйти по горизонтали от костяшек
    auto y1 = farVert(field, tiles.first, direct_y, tiles.second.y());
    auto y2 = farVert(field, tiles.second, -direct_y, tiles.first.y());

    // Если соединение вырожденное (диапазоны не пересекаются), то выходим
    if ((y2 - y1) * direct_y > 0) return false;

    // Соединение по вертикали может идти только в диаппазоне x1 и x2
    // Идем в этом диапазона и проверяем доступность вертикальной линии
    // Первая же доступная вертикальная линия и будет нужной
    for (auto y = y2; y != y1 + direct_y; y += direct_y) {
        if (checkHorzLine(field, tiles.first.x(), tiles.second.x(), y, false)) {
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

// ------------------------------------------------------------------------------------------------
// Проверить, можно ли соединить тремя горизонтальными линиями,
// идущими в одном направлении
bool CField::checkDirectionHorz(const Field &field, TilePair tiles)
{
    // Направо

    // Проверим, насколько можно далеко уйти по горизонтали от костяшек
    auto x1 = farHorz(field, tiles.first, 1, m_x + 1);
    auto x2 = farHorz(field, tiles.second, 1, m_x + 1);

    // Идем от минимального значения до максимального и проверяем вертикаль
    auto x_min = qMax(tiles.first.x(), tiles.second.x());
    auto x_max = qMin(x1, x2);
    for (auto x = x_min; x <= x_max; ++x) {
        if (checkVertLine(field, x, tiles.first.y(), tiles.second.y(), false)) {
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
    x1 = farHorz(field, tiles.first, -1, -1);
    x2 = farHorz(field, tiles.second, -1, -1);

    // Идем от максимального значения до минимального и проверяем вертикаль
    x_min = qMin(tiles.first.x(), tiles.second.x());
    x_max = qMax(x1, x2);
    for (auto x = x_min; x >= x_max; --x) {
        if (checkVertLine(field, x, tiles.first.y(), tiles.second.y(), false)) {
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

// ------------------------------------------------------------------------------------------------
// Проверить, можно ли соединить тремя вертикальными линиями,
// идущими в одном направлении
bool CField::checkDirectionVert(const Field &field, TilePair tiles)
{
    // Вниз

    // Проверим, насколько можно далеко уйти по вертикали от костяшек
    auto y1 = farVert(field, tiles.first, 1, m_y + 1);
    auto y2 = farVert(field, tiles.second, 1, m_y + 1);

    // Идем от минимального значения до максимального и проверяем горизонталь
    auto y_min = qMax(tiles.first.y(), tiles.second.y());
    auto y_max = qMin(y1, y2);
    for (auto y = y_min; y <= y_max; ++y) {
        if (checkHorzLine(field, tiles.first.x(), tiles.second.x(), y, false)) {
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
    y1 = farVert(field, tiles.first, -1, -1);
    y2 = farVert(field, tiles.second, -1, -1);

    // Идем от максимального значения до минимального и проверяем вертикаль
    y_min = qMin(tiles.first.y(), tiles.second.y());
    y_max = qMax(y1, y2);
    for (auto y = y_min; y >= y_max; --y) {
        if (checkHorzLine(field, tiles.first.x(), tiles.second.x(), y, false)) {
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

// ------------------------------------------------------------------------------------------------
// Проверка доступности горизонтальной линии (т.е. без встречных костяшек)
bool CField::checkHorzLine(const Field &field, int x1, int x2, int y, bool is_last_empty) const
{
    // Если выходим за пределы поля - значит истина
    if (y < 0 || y >= m_y) return true;

    auto increment = x1 < x2 ? 1 : -1;
    auto idx = getIndex(x1 + increment, y);
    for (auto x = x1 + increment; x != x2; x += increment, idx += increment) {
        if (field[idx] != -1) return false;
    }

    // И проверка условия последней костяшки
    return (field[idx] == -1) ^ is_last_empty;
}

// ------------------------------------------------------------------------------------------------
// Проверка доступности вертикальной линии
bool CField::checkVertLine(const Field &field, int x, int y1, int y2, bool is_last_empty) const
{
    // Если выходим за пределы поля - значит истина
    if (x < 0 || x >= m_x) return true;

    auto increment = y1 < y2 ? 1 : -1;
    auto idx = getIndex(x, y1 + increment);
    for (auto y = y1 + increment; y != y2; y += increment, idx += increment * m_x) {
        if (field[idx] != -1) return false;
    }

    // И проверка условия последней костяшки
    return (field[idx] == -1) ^ is_last_empty;
}

// ------------------------------------------------------------------------------------------------
// Проверить, насколько далеко можно уйти по горизонтали
int CField::farHorz(const Field &field, Tile tile, int direct, int limit) const
{
    auto idx = getIndex(tile);
    auto x = tile.x();
    forever {
        idx += direct;
        x += direct;
        // Дошли до конца
        if ((direct < 0 && x < 0) || (direct > 0 && x == m_x)) return x;
        // Дошли до предела или не пустое поле - вернем предыдущую костяшку
        if (field[idx] != -1 || x == limit) return x - direct;
    }
}

// ------------------------------------------------------------------------------------------------
// Проверить, насколько далеко можно уйти по вертикали
int CField::farVert(const Field &field, Tile tile, int direct, int limit) const
{
    auto idx = getIndex(tile);
    auto y = tile.y();
    forever {
        idx += direct * m_x;
        y += direct;
        // Дошли до конца
        if ((direct < 0 && y < 0) || (direct > 0 && y == m_y)) return y;
        // Дошли до предела или не пустое поле - вернем предыдущую костяшку
        if (field[idx] != -1 || y == limit) return y - direct;
    }
}

// ------------------------------------------------------------------------------------------------
// Удалить костяшки
void CField::removeTiles(Field &field, TilePair tiles)
{
    field[getIndex(tiles.first)] = -1;
    field[getIndex(tiles.second)] = -1;

    if (!settings->isGravity()) return;

    // Гравитация

    m_deleted_list.clear();
    // Отдельный случай - если костяшки лежат друг над другом по вертикали
    if (tiles.first.x() == tiles.second.x()) {
        // Просто сдвинуть колонку начиная с верхней
        columnMoveDown(field, tiles.first.y() < tiles.second.y() ? tiles.first : tiles.second);
        columnMoveDown(field, tiles.first.y() < tiles.second.y() ? tiles.second : tiles.first);
    } else {
        // Снять обычным образом
        columnMoveDown(field, tiles.first);
        columnMoveDown(field, tiles.second);
    }
}

// ------------------------------------------------------------------------------------------------
// Сдвинуть колонку вниз
void CField::columnMoveDown(Field &field, const Tile &tile)
{
    auto index = getIndex(tile);
    forever {
        m_deleted_list.append(getTile(index));
        auto up_index = index - m_x;
        // Если пытаемся сдвинуть верхнюю костяшку или встретили пустую клетку - выходим
        if (up_index < 0) {
            // То вместо нее вставляем пустую и выходим
            field[index] = -1;
            m_deleted_list.append(getTile(index));
            break;
        }
        // Смещаем
        field[index] = field[up_index];
        if (field[index] == -1) break;
        index -= m_x;
    }
}

// ------------------------------------------------------------------------------------------------
// Проверить активность undo/redo и отправить сигнал при изменении
void CField::checkStatusUndoRedo()
{
    // Undo enabled, если есть снятые костяшки, Redo - если текущий не указывает на конец
    emit signalStatusUndoRedo(m_current_undo >= 0, m_current_undo < m_undo.size() - 1);
}

// ------------------------------------------------------------------------------------------------
// Сдвинуть колонку вверх (для Undo)
void CField::columnMoveUp(Tile tile)
{
    auto index = getIndex(Tile(tile.x(), 0));
    auto finish_index = getIndex(tile);
    Q_ASSERT_X(m_tiles[index] == -1, "CField::unshiftColumn", "Not empty tile");

    forever {
        // Условие выхода: если достигнута ячейка
        if (index == finish_index) break;

        // Если нижняя ячейка не пустая - копируем ее вверх
        if (m_tiles[index + m_x] != -1)
            m_tiles[index] = m_tiles[index + m_x];

        index += m_x;
    }
}

// ------------------------------------------------------------------------------------------------
// m_current_undo всегда указывает на костяшку, которой нужно делать undo
// При снятии костяшки m_current_undo будет указывать на последний элемент
// списка m_undo. При redo список не очищается, а костяшки снимаются, но m_current_undo
// сдвигается влево. Таким образом, для redo костяшка для повторения всегда
// будет справа от m_current_undo
UndoItem CField::doUndo()
{
    Q_ASSERT_X(!m_undo.isEmpty(), "CField::doUndo", "Undo list is empty");

    // Вытащим две последние костяшки
    auto undo_item = m_undo[m_current_undo];
    auto tile1 = std::get<0>(undo_item);
    auto tile2 = std::get<1>(undo_item);
    auto type = std::get<2>(undo_item);

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
    m_remaining += 2;

    --m_current_undo;

    checkStatusUndoRedo();

    return undo_item;
}

// ------------------------------------------------------------------------------------------------
TilePair CField::doRedo()
{
    Q_ASSERT_X(m_current_undo < m_undo.size(), "CField::doRedo", "Error undo definition");

    // Вытащим пару костяшек, которые нужно убрать
    auto undo_item = m_undo[m_current_undo + 1];
    auto tiles = TilePair(std::get<0>(undo_item), std::get<1>(undo_item));
    // И удаляем их
    removeTiles(m_tiles, tiles);
    m_remaining -= 2;

    ++m_current_undo;
    checkStatusUndoRedo();

    return tiles;
}

