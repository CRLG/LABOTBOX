#include "lidar_data_filter_module_example.h"
#include "lidar_data.h"

CLidarDataFilterModuleExample::CLidarDataFilterModuleExample()
{
    m_data_seuil_distance_suppression = m_data_manager.createData("seuil_distance_suppression", 500);
    m_data_seuil_distance_bruit_suppression = m_data_manager.createData("seuil_distance_bruit_suppression", 10);
}

// _______________________________________________________________
const char *CLidarDataFilterModuleExample::get_name()
{
    return "Filtre exemple";
}

// _______________________________________________________________
const char *CLidarDataFilterModuleExample::get_description()
{
    return "Un exemple de filtre tout basique, juste pour montrer comment on s'en sert";
}


// _______________________________________________________________
void CLidarDataFilterModuleExample::filter(const CLidarData *data_in, CLidarData *data_out)
{
    // recopie les valeurs du datamanager dans les données d'entrées du filtre
    m_filter.m_data_seuil_distance_suppression = m_data_seuil_distance_suppression->read().toInt();
    m_filter.m_data_seuil_distance_bruit_suppression = m_data_seuil_distance_bruit_suppression->read().toInt();

    // Filtrage
    m_filter.filter(data_in, data_out);
}

