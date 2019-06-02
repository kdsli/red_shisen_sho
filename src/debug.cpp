#include "debug.h"

#include <QFile>
#include <QTextStream>
#include <QPoint>
#include <QMap>

#include <QDebug>

// Инициализация отладочных механизмов
void initDebug()
{
    QFile file("dump.txt");
    file.open(QIODevice::WriteOnly);
    file.close();
}

void PrintDump(const QString &title, QVector<int> &field, int _x, int _y, int current_count)
{
    QFile file("dump.txt");
    file.open(QIODevice::Append);
    QTextStream ts(&file);
    ts.setFieldWidth(3);
    ts.setPadChar(' ');

    ts << title << " " << current_count << endl << endl;

    ts << "     ";
    for (int i = 0; i < _x; ++i) {
        ts << i << " ";
    }
    ts << endl;

    int n = 0;
    for (int y = 0; y < _y; ++y) {
        ts << y << "  ";
        for (int x = 0; x < _x; ++x) {
            ts << field[n++] << " ";
        }
        ts << endl;
    }
    ts << endl;

    QMap<int, int> map;
    for (const auto tile : field) {
        map[tile] = map[tile] + 1;
    }
    std::for_each(map.keyValueBegin(), map.keyValueEnd(), [&ts](const auto item){
        ts << QString::number(item.first) + ":" + QString::number(item.second) + ",";
    });
    ts << endl << endl;
}

/*
QPoint getPoint(int index, int x, int y)
{
    return QPoint(index % x, index / x);
}

void PrintTiles(const QString &title, const QVector<int> &field, const QVector<int> &tiles)
{
    QFile file("dump.txt");
    file.open(QIODevice::Append);
    QTextStream ts(&file);

    ts << title << " " << field.size() << endl << endl;

    for (const auto &index : field) {
        auto tile = getPoint(index);
        ts << "(" << index << ":" << tile.x() << "," << tile.y() << ":" << tiles[index] << "),";
    }
    ts << endl << endl;
}
*/

// Проверим, сколько костяшек одного типа находится в поле
void CheckField(const QVector<int> &field, int type_count)
{
    QMap<int, int> map;
    for (const auto tile : field) {
        map[tile] = map[tile] + 1;
    }
    std::for_each(map.keyValueBegin(), map.keyValueEnd(), [type_count](const auto item){
        Q_ASSERT_X(item.second == type_count, "CheckField",
                   QString("Failed count type " + QString::number(item.first)).toLatin1().data());
    });
}

void printStringList(const QStringList &list)
{
    QFile file("dump.txt");
    file.open(QIODevice::Append);
    QTextStream ts(&file);

    for (const auto &item : list) {
        ts <<  item << endl;
    }
}

QTextStream& operator<<(QTextStream& stream, const QRectF& rect) {
    stream << "TopLeft (" << rect.left() << ", " << rect.y() << ") Size (" << rect.width() << ", " << rect.height() << ")";
    return stream;
}

void dumpZValues(const QString &message, const QVector<QGraphicsSvgItem *> &vector, int _x, int _y)
{
    QFile file("dump.txt");
    file.open(QIODevice::Append);
    QTextStream ts(&file);
    ts.setFieldWidth(3);
    ts.setPadChar(' ');

    ts << message << endl << endl;

    ts << "     ";
    for (int i = 0; i < _x; ++i) {
        ts << i << " ";
    }
    ts << endl;

    int n = 0;
    for (int y = 0; y < _y; ++y) {
        ts << y << "  ";
        for (int x = 0; x < _x; ++x) {
            ts << (vector[n] ? vector[n]->zValue() : -1) << " ";
            ++n;
        }
        ts << endl;
    }
    ts << endl;

}
