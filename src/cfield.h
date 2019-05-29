#ifndef CFIELD_H
#define CFIELD_H

#include "csettings.h"
#include "ctileset.h"

#include <QObject>
#include <QVector>
#include <QPair>
#include <QPoint>
#include <tuple>

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

// Запись за тип поля
struct FieldRec
{
    // Размеры поля
    quint16 x;
    quint16 y;
    quint16 count;  // Количество одинаковых костяшек для каждого типа
};

// Класс поля. Определяет базовые операции с полем. Работает в основном
// с непревывной координатой и полем как массивом

class CField : public QObject
{
    Q_OBJECT

    friend class CGame;
public:
    CField(CTileSet *ts, QObject *parent = nullptr);

    // Инициализация поля
    void newGame(GameType field_type);
    // Очистить поле
    void clearGame();

    // Размеры поля
    int x() const { return m_x; }
    int y() const { return m_y; }
    // Общее количество плиток на поле
    int tileCount() const { return m_tile_count; }
    // Количество оставшихся плиток !!!
    int currentCount() const { return m_current_count; }

    // Список костяшек для перерисовки
    const TileList &repaintList() const { return m_repaint_list; }
    // Текущий путь
    const TileList &path() const { return m_path; }

    inline void clearPath() { m_path.clear(); }
    void repeatGame();

    // Получить индекс в массиве m_field по координатам
    int getIndex(Tile) const;
    int getIndex(int x, int y) const;
    // Обратная процедура - получить костяшку (координаты) по индексу
    Tile getTile(int) const;

    // Тип костяшки по координатам и индексу
    int tileType(const Tile &) const;
    int tileType(int x, int y) const;
    int tileType(int index) const;

    // Признак, выделена ли костяшка
    bool isSelected(Tile) const;
    bool isSelected(int x, int y) const;

    // Нажатие левой кнопки мыши
    void mouseLeft(Tile);
    // Нажатие правой кнопки мыши
    void mouseRight(Tile);

    // Получить подсказку / проверить варианты
    bool checkVariants(TilePair &tiles);
    // Завершить удаление костяшек
    void cancelRemoveTiles(TilePair tiles);

    // Добавить в путь i-ю порцию из демонстрации
    void setDemostrationPath(int index);
    // Снять костяшки (для демонстрации)
    void clearDemostrationTiles(int index);

public slots:
    // Undo / redo
    void slotUndo();
    void slotRedo();

signals:
    // Сигнал перерисовать указанные костяшки
    void signalRepaint();
    // Сигнал отобразить путь
    void signalStartConnect(TilePair tiles);
    // Сигнал вариантов больше нет
    void signalNotVariants();
    // Сигнал победы
    void signalVictory();
    // Согнал изменения активности Undo/Redo
    void signalStatusUndoRedo(bool is_undo, bool is_redo);

private:
    CTileSet *m_ts;
    Field m_tiles;
    // Форматы полей
    QHash<GameType, FieldRec> m_field_types;
    // Размеры поля
    int m_x, m_y;
    // Общее число костяшек
    int m_tile_count;
    // Количество костяшек одного типа в поле
    int m_count_in_type;
    // Текущее количество костяшек на поле
    int m_current_count;
    // Список выделенных элементов
    TileList m_selected;
    // Список костяшек для перерисовки
    TileList m_repaint_list;
    // Текущий путь
    TileList m_path;
    // Сохраненное начальное поле
    Field m_saved_field;
    // Признак того, что идет смешивание. Нужно, чтобы запретить перерисовку
    bool m_is_shuffle = false;
    // Список правильных снятий, вычисленных на этапе определения вариантов
    TilePairList m_right_pathes;
    // Копия для расчета
    Field m_copy_field;
    // Место для хранения последовательности снятия костяшек. Используется для Undo
    // Каждое снятие помещает в сюда два QPoint и тип ячейки, который сняли
    UndoList m_undo;
    // Текущее положение в списке m_undo
    int m_current_undo;

    // Новая игра, общие настройки
    void doNewGame();
    // Перемешать массив
    void shuffleField(Field &field) const;

    // Алгоритмы поиска
    bool checkDirectHorz(TilePair tiles);
    bool checkDirectVert(TilePair tiles);
    bool checkDoubleLines(TilePair tiles);
    bool checkTowardsHorz(TilePair tiles);
    bool checkTowardsVert(TilePair tiles);
    bool checkDirectionHorz(TilePair tiles);
    bool checkDirectionVert(TilePair tiles);

    // Проверка доступности горизонтальной линии (т.е. без встречных костяшек)
    bool checkHorzLine(int x1, int x2, int y, bool is_last_empty);
    // Проверка доступности вертикальной линии
    bool checkVertLine(int x, int y1, int y2, bool is_last_empty);
    // Проверить, насколько далеко можно уйти по горизонтали
    int farHorz(Tile tile, int direct, int limit);
    // Проверить, насколько далеко можно уйти по вертикали
    int farVert(Tile tile, int direct, int limit);

    // Работа со списком выделенных костяшек
    void clearSelected();
    void addSelected(Tile);
    void removeSelected(Tile);

    // Проверка возможности соединения двух костяшек
    bool checkConnect(TilePair tiles);
    // Соединим две ячейки (если возможно)
    void Connect(TilePair tiles);

    // Удалить костяшки
    void clearTiles(TilePair tiles, Field * = nullptr);
    // Сдвинуть колонку вниз
    void columnMoveDown(const Tile &, Field *collation);
    // Сдвинуть колонку вверх
    void columnMoveUp(Tile);

    // Перемешать так, чтобы обеспечить решаемость игровой комбинации
    void shuffleDecisionVariant();
    // Снять все костяшки с поля
    void takeOffTiles(Field &collation);

    // Проверить активность undo/redo и отправить сигнал при изменении
    void checkStatusUndoRedo();
};

#endif // CFIELD_H
