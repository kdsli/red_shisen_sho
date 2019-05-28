#include "crecordsmanager.h"

#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDataStream>

CRecordsManager *records_managers = nullptr;

// Максимальное количество хранимых рекордов
static const int max_record = 10;

CRecordsManager::CRecordsManager(QObject *parent) : QObject(parent)
{
    // Директория
    auto list = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    if (!list.isEmpty()) {
        m_file_name = list[0];
    } else {
        // Пробуем создать вместе с программой
        m_file_name = qApp->applicationDirPath();
    }

    // Имя файла
    m_file_name += QDir::separator() + qApp->applicationName() + ".records";

    // Доступность
    QFileInfo fi(m_file_name);
    if (!fi.exists()) saveRecords();
    m_record_available = fi.exists() && fi.isWritable() && fi.isReadable();

    loadRecords();
}

// Вернуть список рекордов для определенного типа игры
RecordList &CRecordsManager::getGameRecords(CSettings::FieldType type)
{
    return m_records[type];
}

// Проверить, попадает ли время в таблицу рекордов и добавить если надо
// Возвращает, под каким номером добавлено или -1
int CRecordsManager::checkRecord(CSettings::FieldType type, int game_time)
{
    auto &list = m_records[type];
    Record record;

    // Для пустого списка
    if (list.isEmpty()) {
        list.append(record);
        return 0;
    }

    // Если последний элемент больше
    if (list.back().time > game_time) {
        // Надо вставлять в середину. Найдем куда вставлять
        auto it = std::find_if(list.begin(), list.end(), [game_time](auto const &record){
            return record.time < game_time;
        });
        // Просто вставим в it
        list.insert(it, record);
        // Если размер превысил max_record - просто удалим последний
        if (list.size() == max_record) list.pop_back();
    } else {
        // Время за последним, но если количество меньше максимального -
        // просто добавим в конец
        if (list.size() < max_record) {
            list.append(record);
            return list.size() - 1;
        }
    }

    return -1;
}

// Прочитаь из файла
void CRecordsManager::loadRecords()
{
    QFile file(m_file_name);
    if (!file.open(QIODevice::ReadOnly)) {
        m_record_available = false;
        return;
    }

    QDataStream ds(&file);
    ds.setVersion(QDataStream::Qt_5_0);

    // Читаем количество игр
    int game_count;
    ds >> game_count;

    // И каждую игру
    for (int i = 0; i < game_count; ++i) {
        int game_key;
        int record_count;
        ds >> game_key >> record_count;
        RecordList list;
        m_records.insert(static_cast<CSettings::FieldType>(game_key), list);
        for (int j = 0; j < record_count; ++j) {
            Record record;
            ds >> record.date >> record.name >> record.time >> record.is_gravity;
            list.append(record);
        }
    }
}

// Записать в файл
void CRecordsManager::saveRecords()
{
    QFile file(m_file_name);
    if (!file.open(QIODevice::WriteOnly)) {
        m_record_available = false;
        return;
    }

    QDataStream ds(&file);
    ds.setVersion(QDataStream::Qt_5_0);

    // Общее количество игр
    ds << m_records.size();
    for (auto it = m_records.keyValueBegin(); it != m_records.keyValueEnd(); ++it) {
        auto key = (*it).first;
        auto &list = (*it).second;
        // За каждую игру записать индекс игры и количество записей
        ds << key << list.size();
        // И дальше все записи
        for (const auto &record : list) {
            ds << record.date << record.name << record.time << record.is_gravity;
        }
    }

}
