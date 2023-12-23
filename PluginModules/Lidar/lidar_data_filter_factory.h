#ifndef _LIDAR_DATA_FILTER_FACTORY_H_
#define _LIDAR_DATA_FILTER_FACTORY_H_

#include <QStringList>

#include "lidar_data_filter_base.h"

class CLidarDataFilterFactory
{
public:
    explicit CLidarDataFilterFactory();
    virtual ~CLidarDataFilterFactory();

    static CLidarDataFilterBase* createInstance(QString filter_name);
    static QStringList getExisting();
};

#endif // _LIDAR_DATA_FILTER_FACTORY_H_

/*! @} */

