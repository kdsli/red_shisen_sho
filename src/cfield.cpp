#include "cfield.h"

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

    m_tiles.resize(m_tiles_count);
    m_tiles.fill(-1);

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
}

// ------------------------------------------------------------------------------------------------
// Получить индекс в массиве m_tiles по координатам
int CField::getIndex(int x, int y) const
{
    return y * m_x + x;
}

// ------------------------------------------------------------------------------------------------
// Перемешать массив
void CField::shuffleField(Field &field) const
{
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    auto rnd = std::default_random_engine(seed);
    std::shuffle(field.begin(), field.end(), rnd);
}
