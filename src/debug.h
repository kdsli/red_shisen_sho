#ifndef DEBUG_H
#define DEBUG_H

#include <QString>
#include <QVector>
#include <QFile>
#include <QTextStream>

void initDebug(int x, int y);
void PrintDump(const QString &title, QVector<int> &field, int current_count);
void PrintTiles(const QString &title, const QVector<int> &field, const QVector<int> &tiles);
void CheckField(const QVector<int> &, int);

void printStringList(const QStringList &);

template <typename T>
void printVariable(const QString &text, T t)
{
    QFile file("dump.txt");
    file.open(QIODevice::Append);
    QTextStream ts(&file);

    ts << text << " = " << t << endl;
}


#endif // DEBUG_H
