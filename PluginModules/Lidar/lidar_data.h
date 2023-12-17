#ifndef _LIDARDATA_H_
#define _LIDARDATA_H_

class CLidarData
{
public:
    CLidarData();

    static const int MAX_MEASURES_COUNT  = 1024;

    unsigned int m_timestamp;                         // [µsec]
    double m_start_angle;                             // [°]
    double m_angle_step_resolution;                   // [°]
    int m_measures_count;                             // Nombre de mesures
    double m_dist_measures[MAX_MEASURES_COUNT];       // Tableau des mesure de distance
    int m_scan_frequency;                             // [Hz]
    double m_scale_factor;                            //
};

#endif // _LIDARDATA_H_
