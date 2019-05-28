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
using RecordList = QVector<Record>;
using GameRecords = QMap<CSettings::FieldType, RecordList>;

// Класс хранения рекордов программы

class CRecordsManager : public QObject
{
    Q_OBJECT
public:
    CRecordsManager(QObject *parent = nullptr);

    // Вернуть список рекордов для определенного типа игры
    RecordList &getGameRecords(CSettings::FieldType);
    // Проверить, попадает ли время в таблицу рекордов и добавить если надо
    // Возвращает, под каким номером добавлено
    int checkRecord(CSettings::FieldType, int game_time);

private:
    GameRecords m_records;

    void loadRecords();
    void saveRecords();
};

extern CRecordsManager records_managers;

#endif // CRECORSDMANAGER_H
