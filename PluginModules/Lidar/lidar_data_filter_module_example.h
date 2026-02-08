#ifndef _LIDAR_DATA_FILTER_MODULE_EXAMPLE_H_
#define _LIDAR_DATA_FILTER_MODULE_EXAMPLE_H_

#include "lidar_data_filter_module_base.h"
#include "lidar_data_filter_example.h"

class CLidarDataFilterModuleExample : public CLidarDataFilterModuleBase
{
public:
    CLidarDataFilterModuleExample();

    /*virtual*/void filter(const CLidarData *data_in, CLidarData *data_out) override;
    /*virtual*/const char *get_name() override;
    /*virtual*/const char *get_description() override;

    CData *m_data_seuil_distance_suppression;
    CData *m_data_seuil_distance_bruit_suppression;

    CLidarDataFilterExample m_filter;
};

#endif // _LIDAR_DATA_FILTER_MODULE_EXAMPLE_H_
