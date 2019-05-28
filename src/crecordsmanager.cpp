#include "crecordsmanager.h"

CRecordsManager::CRecordsManager(QObject *parent) : QObject(parent)
{

}

TypeRecords &CRecordsManager::getGameRecords(CSettings::FieldType type)
{
    return m_records{type};
}
