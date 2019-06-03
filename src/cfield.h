#ifndef CFIELD_H
#define CFIELD_H

#include "common_types.h"

#include <QObject>
#include <QVector>

// Класс, отвечающий за игровое поле и логику

class CField : public QObject
{
    Q_OBJECT
    friend class CBoard;
    friend class CScene;
public:
    explicit CField(QObject *parent = nullptr);

    // Новая игра
    void newGame(int x, int y, int count);

    // Вернуть начальное поле
    void restoreField();

    // Соединим две ячейки (если возможно)
    void Connect(const TilePair &tiles);

    // Проверка состояния игры (есть ли дальше варианты, достигнута ли победа)
    VariantStatus getGameStatus();

    UndoItem doUndo();
    TilePair doRedo();

signals:
    // Сигнал снять костяшки и отобразить путь
    void signalStartConnect(const TilePair &tiles);
    // Сигнал изменения активности Undo/Redo
    void signalStatusUndoRedo(bool is_undo, bool is_redo);

private:
    // Размеры поля
    int m_x{0}, m_y{0};
    int m_count_in_type{0};
    int m_tiles_count{0};
    // Текущее количество костяшек
    int m_remaining{0};
    // Поле
    Field m_tiles;
    // Начальное поле (для начала игра с начала и демонстрации)
    Field m_old_tiles;
    // Список правильных снятий, вычисленных на этапе определения вариантов
    TilePairList m_right_pathes;
    // Текущий путь
    TileList m_path;
    // Список костяшек, которые должны быть удалены
    TileList m_deleted_list;
    // Список костяшек для подсказки
    TilePair m_hint_tiles;
    // Место для хранения последовательности снятия костяшек. Используется для Undo
    // Каждое снятие помещает в сюда два QPoint и тип ячейки, который сняли
    UndoList m_undo;
    // Текущее положение в списке m_undo
    int m_current_undo{0};

    // Получить индекс в массиве Field по координатам
    int getIndex(int x, int y) const;
    int getIndex(Tile tile) const;
    // Обратная процедура - получить костяшку (координаты) по индексу
    Tile getTile(int index) const;

    // Получить тип костяшки
    int getTileType(Tile &tile);
    int getTileType(int index);

    // Перемешать массив
    void shuffleField(Field &field) const;

    // Сделать так, чтобы существовал минимум один путь снятия
    void shuffleDecisionVariant();
    // Снять все возможные костяшки (вызывается из shuffleDecisionVariant)
    void takeTilesOff(Field &field, Field &collation, int &current_tile_count);

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
    void removeTiles(Field &field, TilePair tiles);
    // Сдвинуть колонку вниз
    void columnMoveDown(Field &field, const Tile &);

    // Проверить активность undo/redo и отправить сигнал при изменении
    void checkStatusUndoRedo();

    // Сдвинуть колонку вверх (для Undo)
    void columnMoveUp(Tile tile);
};

#endif // CFIELD_H
