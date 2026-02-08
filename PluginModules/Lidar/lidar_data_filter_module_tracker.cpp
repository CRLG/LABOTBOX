#include "lidar_data_filter_module_tracker.h"
#include "lidar_data.h"
#include "Lidar_utils.h"

CLidarDataFilterModuleTracker::CLidarDataFilterModuleTracker()
{
    //Paramétrage
    m_dot_size = m_data_manager.createData("dot_size", 20);
    m_d_dist_offset = m_data_manager.createData("d_dist_offset", 43.6);
    m_i_MAX_BLANK = m_data_manager.createData("i_MAX_BLANK", 3);
    m_i_MIN_COUNT = m_data_manager.createData("i_MIN_COUNT", 2);
    m_i_MAX_COUNT = m_data_manager.createData("i_MAX_COUNT", 60);
    m_d_MAX_dist = m_data_manager.createData("d_MAX_dist", 3600.);
    m_d_MIN_dist = m_data_manager.createData("d_MIN_dist", 150.);
    m_d_seuil_filtrage_dist = m_data_manager.createData("d_seuil_filtrage_dist", 150.);
    m_i_seuil_filtrage_angle = m_data_manager.createData("i_seuil_filtrage_angle", 3);
    m_d_seuil_gradient = m_data_manager.createData("d_seuil_gradient", 150.);
    m_d_seuil_facteur_forme = m_data_manager.createData("d_seuil_facteur_forme", 1400.);
}

// _______________________________________________________________
const char *CLidarDataFilterModuleTracker::get_name()
{
    return "Tracker";
}

// _______________________________________________________________
const char *CLidarDataFilterModuleTracker::get_description()
{
    return "Un filtre tracker mis au point par le vénérable Laguiche :)";
}

// _______________________________________________________________
void CLidarDataFilterModuleTracker::filter(const CLidarData *data_in, CLidarData *data_out)
{
    // recopie les valeurs du datamanager dans les données d'entrées du filtre
    m_filter.m_dot_size = m_dot_size->read().toInt();
    m_filter.m_d_dist_offset = m_d_dist_offset->read().toDouble();
    m_filter.m_i_MAX_BLANK = m_i_MAX_BLANK->read().toInt();
    m_filter.m_i_MIN_COUNT = m_i_MIN_COUNT->read().toInt();
    m_filter.m_i_MAX_COUNT = m_i_MAX_COUNT->read().toInt();
    m_filter.m_d_MAX_dist = m_d_MAX_dist->read().toDouble();
    m_filter.m_d_MIN_dist = m_d_MIN_dist->read().toDouble();
    m_filter.m_d_seuil_filtrage_dist = m_d_seuil_filtrage_dist->read().toDouble();
    m_filter.m_i_seuil_filtrage_angle = m_i_seuil_filtrage_angle->read().toInt();
    m_filter.m_d_seuil_gradient = m_d_seuil_gradient->read().toDouble();
    m_filter.m_d_seuil_facteur_forme = m_d_seuil_facteur_forme->read().toDouble();

    // Filtrage
    m_filter.filter(data_in, data_out);
}

