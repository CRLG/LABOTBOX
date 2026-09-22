#include "banc_data_manager.h"

CDataManager::~CDataManager()
{
    qDeleteAll(m_map_data);
}

void CDataManager::write(QString varname, QVariant val)
{
    CData *data = getData(varname);
    if (data == nullptr) {
        m_map_data.insert(varname, new CData(varname, val));
    }
    else {
        data->write(val);       // emet valueChanged : les abonnes du plugin (CLidarSimu...) sont notifies
    }
}

QVariant CDataManager::read(QString varname)
{
    return isExist(varname) ? getData(varname)->read() : QVariant("");
}

CData *CDataManager::getData(QString varname, bool force_creation)
{
    if (force_creation && !isExist(varname)) {
        m_map_data.insert(varname, new CData(varname, QVariant()));
    }
    return m_map_data.value(varname, nullptr);
}

bool CDataManager::isExist(QString varname)
{
    return m_map_data.contains(varname);
}
