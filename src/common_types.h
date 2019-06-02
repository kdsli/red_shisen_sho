#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <QVector>
#include <QPoint>

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

// Состояние игры после текущей итерации
enum VariantStatus {vsNormal, vsNotVariant, vsVictory};

#endif // COMMON_TYPES_H
