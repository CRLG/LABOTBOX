#ifndef _LIDARDATA_FILTER_TRACKER_H_
#define _LIDARDATA_FILTER_TRACKER_H_

#include "lidar_data_filter_base.h"

class CLidarDataFilterTracker : public CLidarDataFilterBase
{
public:
    CLidarDataFilterTracker();

    /*virtual*/void filter(const CLidarData *data_in, CLidarData *data_out);
    /*virtual*/const char *get_name();
    /*virtual*/const char *get_description();

    //Paramétrage
    double m_d_dist_offset;           // mm
    int m_i_MAX_BLANK;                // discontinuité possible du blob
    int m_i_MIN_COUNT;                // taille minimum du blob
    int m_i_MAX_COUNT;                // taille maximum du blob
    double m_d_MAX_dist;              // zone max de détection
    double m_d_MIN_dist;              // zone min de détection
    double m_d_seuil_filtrage_dist;   // pour fusionner les doublons trops proches en distance
    int m_i_seuil_filtrage_angle;     // pour fusionner les doublons trops proches en angle
    const int m_i_MAX_SAMPLES_THRESHOLD=5;
    double m_d_seuil_gradient;
    double m_d_seuil_facteur_forme;
};

#endif // _LIDARDATA_FILTER_TRACKER_H_
