#ifndef _LIDARDATA_FILTER_EXAMPLE_H_
#define _LIDARDATA_FILTER_EXAMPLE_H_

#include "lidar_data_filter_base.h"

class CLidarDataFilterExample : public CLidarDataFilterBase
{
public:
    CLidarDataFilterExample();

    /*virtual*/void filter(const CLidarData *data_in, CLidarData *data_out);
    /*virtual*/const char *get_name();
    /*virtual*/const char *get_description();

    CData *m_data_seuil_distance_suppression;
    CData *m_data_seuil_distance_bruit_suppression;
};

#endif // _LIDARDATA_FILTER_EXAMPLE_H_
