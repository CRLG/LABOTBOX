#include "CApplication.h"
#include "CDataManager.h"
#include "CTelemetres_simu.h"

static const float DEFAULT_DISTANCE = 999e6;

CTelemetresSimu::CTelemetresSimu()
      : m_application(nullptr)
{
    Init();
    setOrigineTelemetre(TELEMETRES_FROM_SIMU);
}

// ___________________________________________________
void CTelemetresSimu::Init()
{
    // Force la création des data dans le DataManager
    updateDataManager("Telemetres.AVG", DEFAULT_DISTANCE);
    updateDataManager("Telemetres.AVD", DEFAULT_DISTANCE);
    updateDataManager("Telemetres.ARG", DEFAULT_DISTANCE);
    updateDataManager("Telemetres.ARD", DEFAULT_DISTANCE);
    updateDataManager("Telemetres.ARGCentre", DEFAULT_DISTANCE);
    updateDataManager("Telemetres.ARDCentre", DEFAULT_DISTANCE);
}

//___________________________________________________________________________
// Les données simulées sont issues récupérées du DataCenter
float CTelemetresSimu::getDistanceAVG()
{
    if (!m_application) return DEFAULT_DISTANCE;
    switch(m_origine_telemetres) {
        case TELEMETRES_FROM_GUI :      return m_gui_distance_avg;
        case TELEMETRES_FROM_SIMUBOT :  return m_application->m_data_center->read("Simubot.Telemetres.AVG").toFloat();
        default :
        case TELEMETRES_FROM_SIMU :     return m_application->m_data_center->read("Telemetres.AVG").toFloat();
    }

}

float CTelemetresSimu::getDistanceAVD()
{
    if (!m_application) return DEFAULT_DISTANCE;
    switch(m_origine_telemetres) {
        case TELEMETRES_FROM_GUI :      return m_gui_distance_avd;
        case TELEMETRES_FROM_SIMUBOT :  return m_application->m_data_center->read("Simubot.Telemetres.AVD").toFloat();
        default :
        case TELEMETRES_FROM_SIMU :     return m_application->m_data_center->read("Telemetres.AVD").toFloat();
    }
}

float CTelemetresSimu::getDistanceARG()
{
    if (!m_application) return DEFAULT_DISTANCE;
    switch(m_origine_telemetres) {
        case TELEMETRES_FROM_GUI :      return m_gui_distance_arg;
        case TELEMETRES_FROM_SIMUBOT :  return m_application->m_data_center->read("Simubot.Telemetres.ARG").toFloat();
        default :
        case TELEMETRES_FROM_SIMU :     return m_application->m_data_center->read("Telemetres.ARG").toFloat();
    }
}

float CTelemetresSimu::getDistanceARD()
{
    if (!m_application) return DEFAULT_DISTANCE;
    switch(m_origine_telemetres) {
        case TELEMETRES_FROM_GUI :      return m_gui_distance_ard;
        case TELEMETRES_FROM_SIMUBOT :  return m_application->m_data_center->read("Simubot.Telemetres.ARD").toFloat();
        default :
        case TELEMETRES_FROM_SIMU :     return m_application->m_data_center->read("Telemetres.ARD").toFloat();
    }
}

float CTelemetresSimu::getDistanceARGCentre()
{
    if (!m_application) return DEFAULT_DISTANCE;
    switch(m_origine_telemetres) {
        case TELEMETRES_FROM_GUI :      return m_gui_distance_arg_centre;
        case TELEMETRES_FROM_SIMUBOT :  return m_application->m_data_center->read("Simubot.Telemetres.ARGCentre").toFloat();
        default :
        case TELEMETRES_FROM_SIMU :     return m_application->m_data_center->read("Telemetres.ARGCentre").toFloat();
    }
}

float CTelemetresSimu::getDistanceARDCentre()
{
    if (!m_application) return DEFAULT_DISTANCE;
    switch(m_origine_telemetres) {
        case TELEMETRES_FROM_GUI :      return m_gui_distance_ard_centre;
        case TELEMETRES_FROM_SIMUBOT :  return m_application->m_data_center->read("Simubot.Telemetres.ARDCentre").toFloat();
        default :
        case TELEMETRES_FROM_SIMU :     return m_application->m_data_center->read("Telemetres.ARDCentre").toFloat();
    }
}

// ============================================================
//                          SIMULATION
// ============================================================
void CTelemetresSimu::init(CApplication *application)
{
    m_application = application;
    Init();
}

void CTelemetresSimu::setOrigineTelemetre(int origine)
{
    m_origine_telemetres = origine;
}

void CTelemetresSimu::setDistancesFromGui(float avg, float avd, float arg, float ard, float arg_centre, float ard_centre)
{
    m_gui_distance_avg = avg;
    m_gui_distance_avd = avd;
    m_gui_distance_arg = arg;
    m_gui_distance_ard = ard;
    m_gui_distance_arg_centre = arg_centre;
    m_gui_distance_ard_centre = ard_centre;
}

// ___________________________________________________
void CTelemetresSimu::updateDataManager(QString dataname, QVariant val)
{
    if (!m_application) return;
    m_application->m_data_center->write(dataname, val);
}
