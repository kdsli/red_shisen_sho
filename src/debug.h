#ifndef DEBUG_H
#define DEBUG_H

#include <QString>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QRectF>
#include <QGraphicsSvgItem>

void initDebug();
void PrintDump(const QString &title, QVector<int> &field, int x, int y, int current_count);
//void PrintTiles(const QString &title, const QVector<int> &field, const QVector<int> &tiles);
void CheckField(const QVector<int> &, int);

void printStringList(const QStringList &);

void dumpZValues(const QString &message, const QVector<QGraphicsSvgItem *> &vector, int _x, int _y);

template <typename T>
void printVariable(const QString &text, T t)
{
    QFile file("dump.txt");
    file.open(QIODevice::Append);
    QTextStream ts(&file);

    ts << text << " = " << t << endl;
}

QTextStream& operator<<(QTextStream& stream, const QRectF& rect);


#endif // DEBUG_H
