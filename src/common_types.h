#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <QVector>
#include <QPoint>

// Запись за тип поля
struct FieldRec
{
    // Размеры поля
    quint16 x;
    quint16 y;
    // Количество одинаковых костяшек для каждого типа
    quint16 count;
};

// Поле - cписок костяшек (непрерывный вектор)
using Field = QVector<int>;
// Одна костяшка (координата)
using Tile = QPoint;
// Список костяшек
using TileList = QVector<Tile>;
// Пара костяшек (координат)
using TilePair = QPair<Tile, Tile>;
// Список пар костяшек
using TilePairList = QVector<TilePair>;
// Элемент списка Undo и сам список
using UndoItem = std::tuple<Tile, Tile, int>;
using UndoList = QVector<UndoItem>;

// Список координат сцены. Используется для определения координат пути
using CoordList = QVector<QPointF>;

// Состояние игры
enum GameState { gsEmpty, gsGame, gsPause, gsVictory, gsNotVariants, gsDemostration };

// Состояние игры после текущей итерации
enum VariantStatus {vsNormal, vsNotVariant, vsVictory};

// Секунды в строку
QString getSecondsString(int seconds);

#endif // COMMON_TYPES_H
