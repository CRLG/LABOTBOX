#include "lidar_data_filter.h"
#include "lidar_data.h"

CLidarDataFilter::CLidarDataFilter()
{
    m_seuil_distance_suppression = 500;         //[mm]
    m_seuil_distance_bruit_suppression = 10;    // [mm]
}

// _______________________________________________________________
void CLidarDataFilter::filter(const CLidarData *in_raw_data, CLidarData *out_filtered_data)
{
    if (!in_raw_data)       return;
    if (!out_filtered_data) return;

    *out_filtered_data = *in_raw_data; // recopie tous les champs (y compris les données distance)

    // Filtrage
    for (int i=0; i<in_raw_data->m_measures_count; i++) {

        // 1. Elimine tous les points supérieurs à une certaine distance
        if (in_raw_data->m_dist_measures[i] > m_seuil_distance_suppression) {
            out_filtered_data->m_dist_measures[i] = POINT_IGNORED;
        }

        // 2. Elimine tous les points très très proche (bruit)
        if (in_raw_data->m_dist_measures[i] < m_seuil_distance_bruit_suppression) {
            out_filtered_data->m_dist_measures[i] = POINT_IGNORED;
        }
    }
}
