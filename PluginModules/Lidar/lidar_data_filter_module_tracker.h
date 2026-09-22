#ifndef _LIDAR_DATA_FILTER_MODULE_TRACKER_H_
#define _LIDAR_DATA_FILTER_MODULE_TRACKER_H_

#include "lidar_data_filter_module_base.h"
#include "lidar_data_filter_tracker.h"

class CLidarDataFilterModuleTracker : public CLidarDataFilterModuleBase
{
public:
    CLidarDataFilterModuleTracker();

    /*virtual*/void filter(const CLidarData *data_in, CLidarData *data_out) override;
    /*virtual*/const char *get_name() override;
    /*virtual*/const char *get_description() override;

    //Paramétrage
    CData* m_d_dist_offset;           // mm
    CData* m_i_MAX_BLANK;                // discontinuité possible du blob
    CData* m_i_MIN_COUNT;                // taille minimum du blob
    CData* m_i_MAX_COUNT;                // taille maximum du blob
    CData* m_d_MAX_dist;              // zone max de détection
    CData* m_d_MIN_dist;              // zone min de détection
    CData* m_d_seuil_filtrage_dist;   // pour fusionner les doublons trops proches en distance
    CData* m_i_seuil_filtrage_angle;     // pour fusionner les doublons trops proches en angle
    const int m_i_MAX_SAMPLES_THRESHOLD=5;
    CData* m_d_seuil_gradient;
    // Courbes enveloppes du facteur de forme : demi-diagonales [mm] du plus petit et du plus grand
    // mat balise admis (remplacent la marge unique d_seuil_facteur_forme, qui ne bornait que d'un cote)
    CData* m_d_R_mini;
    CData* m_d_R_maxi;
    CData* m_dot_size;

    CLidarDataFilterTracker m_filter;
};

#endif // _LIDAR_DATA_FILTER_MODULE_TRACKER_H_
