#ifndef CRECORSDMANAGER_H
#define CRECORSDMANAGER_H

#include "csettings.h"

#include <QObject>
#include <QMap>
#include <QDate>

// Запись за конкретный рекорд
struct Record
{
    QDate date;
    QString name;
    int time;         // Количество секунд
    bool is_gravity;
};

// Список рекордов
using TypeRecords = QVector<Record>;
using Records = QMap<CSettings::FieldType, TypeRecords>;

// Класс хранения рекордов программы

class CRecordsManager : public QObject
{
    Q_OBJECT
public:
    CRecordsManager(QObject *parent = nullptr);

    // Вернуть список рекордов для определенного типа игры
    TypeRecords &getGameRecords(CSettings::FieldType);

private:
    Records m_records;
};

extern CRecordsManager records_managers;

#endif // CRECORSDMANAGER_H
