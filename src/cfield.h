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

    // Поле
    const Field &tiles() const { return m_tiles; }

    // Получить индекс в массиве m_tiles по координатам
    int getIndex(int x, int y) const;

    void newGame(int x, int y, int count_in_type);

private:
    // Размеры поля
    int m_x, m_y;
    int m_count_in_type;
    int m_tiles_count;
    // Поле
    Field m_tiles;

    // Перемешать массив
    void shuffleField(Field &tiles) const;
};

#endif // CFIELD_H
