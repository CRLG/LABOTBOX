#include "lidar_data_filter_example.h"
#include "lidar_data.h"

CLidarDataFilterExample::CLidarDataFilterExample()
{
    m_data_seuil_distance_suppression = m_data_manager.createData("seuil_distance_suppression", 500);
    m_data_seuil_distance_bruit_suppression = m_data_manager.createData("seuil_distance_bruit_suppression", 10);
}

// _______________________________________________________________
const char *CLidarDataFilterExample::get_name()
{
    return "Filtre exemple";
}

// _______________________________________________________________
const char *CLidarDataFilterExample::get_description()
{
    return "Un exemple de filtre tout basique, juste pour montrer comment on s'en sert";
}


// _______________________________________________________________
void CLidarDataFilterExample::filter(const CLidarData *data_in, CLidarData *data_out)
{
    if (!data_in)   return;
    if (!data_out)  return;

    *data_out = *data_in; // recopie tous les champs (y compris les données distance)

    // Filtrage
    for (int i=0; i<data_in->m_measures_count; i++) {

        // 1. Elimine tous les points supérieurs à une certaine distance
        if (data_in->m_dist_measures[i] > m_data_seuil_distance_suppression->read().toInt()) {
            data_out->m_dist_measures[i] = POINT_IGNORED;
        }

        // 2. Elimine tous les points très très proche (bruit)
        if (data_in->m_dist_measures[i] < m_data_seuil_distance_bruit_suppression->read().toInt()) {
            data_out->m_dist_measures[i] = POINT_IGNORED;
        }
    }
}

