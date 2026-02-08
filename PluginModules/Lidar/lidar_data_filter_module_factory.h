#ifndef _LIDAR_DATA_FILTER_FACTORY_H_
#define _LIDAR_DATA_FILTER_FACTORY_H_

#include <QStringList>

#include "lidar_data_filter_module_base.h"

class CLidarDataFilterModuleFactory
{
public:
    explicit CLidarDataFilterModuleFactory();
    virtual ~CLidarDataFilterModuleFactory();

    static CLidarDataFilterModuleBase* createInstance(QString filter_name);
    static QStringList getExisting();
};

#endif // _LIDAR_DATA_FILTER_FACTORY_H_

/*! @} */

