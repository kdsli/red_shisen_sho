#ifndef DEBUG_H
#define DEBUG_H

#include <QString>
#include <QVector>

void initDebug(int x, int y);
void PrintDump(const QString &title, const QVector<int> &field, int current_count);
void PrintTiles(const QString &title, const QVector<int> &field, const QVector<int> &tiles);
void CheckField(const QVector<int> &, int);

#endif // DEBUG_H
