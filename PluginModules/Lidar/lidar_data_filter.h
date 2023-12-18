#ifndef _LIDARDATA_FILTER_H_
#define _LIDARDATA_FILTER_H_

class CLidarData;
class CLidarDataFilter
{
public:
    CLidarDataFilter();

    void filter(const CLidarData *in_raw_data, CLidarData *out_filtered_data);

    double m_seuil_distance_suppression;
    double m_seuil_distance_bruit_suppression;

    const int POINT_IGNORED = 99999999;
};

#endif // _LIDARDATA_FILTER_H_
