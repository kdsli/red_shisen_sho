#include "crecordsmanager.h"

#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include <QGraphicsSvgItem>

CRecordsManager *records_manager = nullptr;

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

    // Если директории нет - создадим ее
    QDir dir(m_file_name);
    if (!dir.exists())
        dir.mkpath(m_file_name);

    // Имя файла
    m_file_name += QDir::separator() + qApp->applicationName() + ".records";

    // Доступность
    QFileInfo fi(m_file_name);
    if (!fi.exists()) saveRecords();
    m_record_available = fi.exists() && fi.isWritable() && fi.isReadable();

    loadRecords();
}

// Вернуть список рекордов для определенного типа игры
RecordList &CRecordsManager::gameRecords(GameType type)
{
    return m_records[type];
}

// Проверить, попадает ли время в таблицу рекордов и добавить если надо
// Возвращает, под каким номером добавлено или -1
int CRecordsManager::checkRecord(int game_time)
{
    auto &list = m_records[settings->currentGameType()];
    Record record;
    record.time = game_time;
    record.is_gravity = settings->isGravity();
    record.name = settings->userName();

    // Для пустого списка
    if (list.isEmpty()) {
        list.append(record);
        saveRecords();
        return 0;
    }

    // Если последний элемент больше
    if (list.back().time > game_time) {
        // Надо вставлять в середину. Найдем куда вставлять
        auto it = std::find_if(list.begin(), list.end(), [game_time](auto const &record){
            return record.time > game_time;
        });
        int dist = static_cast<int>(std::distance(list.begin(), it));
        // Просто вставим в it
        list.insert(it, record);
        // Если размер превысил max_record - просто удалим последний
        if (list.size() == max_record) list.pop_back();
        saveRecords();
        return dist;
    } else {
        // Время за последним, но если количество меньше максимального -
        // просто добавим в конец
        if (list.size() < max_record) {
            list.append(record);
            saveRecords();
            return list.size() - 1;
        }
    }

    return -1;
}

int CRecordsManager::maxRecord() const
{
    return max_record;
}

// Прочитать из файла
void CRecordsManager::loadRecords()
{
    QFile file(m_file_name);
    if (!file.open(QIODevice::ReadOnly)) {
        m_record_available = false;
        return;
    }

    QDataStream ds(&file);
    ds.setVersion(QDataStream::Qt_5_0);

    // Читаем весь файл в буфер
    QByteArray ba;
    ds >> ba;

    // Super decode!
    QByteArray dst = QByteArray::fromBase64(ba);

    // А дальше разбираем уже поток из массива
    QDataStream buffer(&dst, QIODevice::ReadOnly);
    buffer.setVersion(QDataStream::Qt_5_0);

    // Читаем количество игр
    int game_count;
    buffer >> game_count;

    // И каждую игру
    for (int i = 0; i < game_count; ++i) {
        int game_key;
        int record_count;
        buffer >> game_key >> record_count;
        RecordList list;
        for (int j = 0; j < record_count; ++j) {
            Record record;
            buffer >> record.date >> record.name >> record.time >> record.is_gravity;
            list.append(record);
        }
        m_records.insert(static_cast<GameType>(game_key), list);
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

    // Запишем в QByteArray
    QByteArray ba;
    QDataStream buffer(&ba, QIODevice::WriteOnly);
    buffer.setVersion(QDataStream::Qt_5_0);

    // Общее количество игр
    buffer << m_records.size();
    for (auto it = m_records.keyValueBegin(); it != m_records.keyValueEnd(); ++it) {
        auto key = (*it).first;
        auto &list = (*it).second;
        // За каждую игру записать индекс игры и количество записей
        buffer << key << list.size();
        // И дальше все записи
        for (const auto &record : list) {
            buffer << record.date << record.name << record.time << record.is_gravity;
        }
    }

    // А буфер в файл - super encoding!
    ds << ba.toBase64();
}

