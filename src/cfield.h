#ifndef CFIELD_H
#define CFIELD_H

#include "common_types.h"

#include <QObject>
#include <QVector>

// Класс, отвечающий за игровое поле

class CField : public QObject
{
    Q_OBJECT
public:
    explicit CField(QObject *parent = nullptr);

    // Размеры поля
    int x() const { return m_x; }
    int y() const { return m_y; }
    int tilesCount() const { return m_tiles_count; }

    // Координаты в поле
    const Field &tiles() const { return m_tiles; }
    // Получить индекс в массиве Field по координатам
    int getIndex(int x, int y) const;
    int getIndex(Tile tile) const;
    // Обратная процедура - получить костяшку (координаты) по индексу
    Tile getTile(int index) const;

    // Получить тип костяшки
    int getTileType(Tile &tile);
    int getTileType(int index);

    // Новая игра
    void newGame(int x, int y, int count_in_type);

private:
    // Размеры поля
    int m_x, m_y;
    int m_count_in_type;
    int m_tiles_count;
    // Поле
    Field m_tiles;
    // Список правильных снятий, вычисленных на этапе определения вариантов
    TilePairList m_right_pathes;
    // Текущий путь
    TileList m_path;
    // Список костяшек, которые должны быть удалены
    TileList m_deleted_list;

    // Перемешать массив
    void shuffleField(Field &tiles) const;

    // Сделать так, чтобы существовал минимум один путь снятия
    void shuffleDecisionVariant();
    // Снять все возможные костяшки (вызывается из shuffleDecisionVariant)
    void takeTilesOff(Field &field, Field &collation, int &currentTile_count);

    // Проверить существование вариантов снятия
    bool checkVariants(const Field &field, TilePair &tiles);
    // Проверка возможности снятия пары костяшек
    bool checkConnect(const Field &field, TilePair tiles);

    // Алгоритмы поиска
    bool checkDirectHorz(const Field &field, TilePair tiles);
    bool checkDirectVert(const Field &field, TilePair tiles);
    bool checkDoubleLines(const Field &field, TilePair tiles);
    bool checkTowardsHorz(const Field &field, TilePair tiles);
    bool checkTowardsVert(const Field &field, TilePair tiles);
    bool checkDirectionHorz(const Field &field, TilePair tiles);
    bool checkDirectionVert(const Field &field, TilePair tiles);

    // Проверка доступности горизонтальной линии (т.е. без встречных костяшек)
    bool checkHorzLine(const Field &field, int x1, int x2, int y, bool is_last_empty) const;
    // Проверка доступности вертикальной линии
    bool checkVertLine(const Field &field, int x, int y1, int y2, bool is_last_empty) const;
    // Проверить, насколько далеко можно уйти по горизонтали
    int farHorz(const Field &field, Tile tile, int direct, int limit) const;
    // Проверить, насколько далеко можно уйти по вертикали
    int farVert(const Field &field, Tile tile, int direct, int limit) const;

    // Удалить костяшки
    void clearTiles(Field &field, TilePair tiles);
    // Сдвинуть колонку вниз
    void columnMoveDown(Field &field, const Tile &);
};

#endif // CFIELD_H
