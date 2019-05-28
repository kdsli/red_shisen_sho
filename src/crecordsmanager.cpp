#include "crecordsmanager.h"

CRecordsManager::CRecordsManager(QObject *parent) : QObject(parent)
{
    loadRecords();
}

// Вернуть список рекордов для определенного типа игры
RecordList &CRecordsManager::getGameRecords(CSettings::FieldType type)
{
    return m_records[type];
}

// Проверить, попадает ли время в таблицу рекордов и добавить если надо
// Возвращает, под каким номером добавлено
int CRecordsManager::checkRecord(CSettings::FieldType type, int game_time)
{
    auto &list = m_records[type];
}

void CRecordsManager::loadRecords()
{

}

void CRecordsManager::saveRecords()
{

}
