#ifndef _LIDAR_DATA_FILTER_MODULE_BASE_H_
#define _LIDAR_DATA_FILTER_MODULE_BASE_H_

#include "lidar_data_filter_base.h"
#include "CDataManager.h"

// Clase de base pour tous les filtres Lidar
class CLidarData;
class CLidarDataFilterModuleBase
{
public:
    CLidarDataFilterModuleBase() : m_data_manager("") { }
    virtual ~CLidarDataFilterModuleBase() { }

    virtual const char *get_name()=0;
    virtual const char *get_description() { return ""; } // pas obligatoire
    virtual void filter(const CLidarData *data_in, CLidarData *data_out) = 0;

    CDataManager m_data_manager;
};

#endif // _LIDAR_DATA_FILTER_MODULE_BASE_H_
