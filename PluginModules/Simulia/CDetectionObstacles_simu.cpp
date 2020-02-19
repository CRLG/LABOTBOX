#include "CApplication.h"
#include "CDataManager.h"
#include "CDetectionObstacles_simu.h"

#include "CGlobale.h"

CDetectionObstaclesSimu::CDetectionObstaclesSimu()
      : m_application(nullptr)
{
    Init();
    setOrigineDetection(OBSTACLES_FROM_SIMU);
}

// ___________________________________________________
void CDetectionObstaclesSimu::Init()
{
    // Force la création des data dans le DataManager
    updateDataManager("DetectionObstacle.AVG", false);
    updateDataManager("DetectionObstacle.AVD", false);
    updateDataManager("DetectionObstacle.ARG", false);
    updateDataManager("DetectionObstacle.ARD", false);
    updateDataManager("DetectionObstacle.isObstacle", false);
    m_gui_detect_avg = false;
    m_gui_detect_avd = false;
    m_gui_detect_arg = false;
    m_gui_detect_ard = false;
}

// ============================================================
//                          SIMULATION
// ============================================================

//___________________________________________________________________________
bool CDetectionObstaclesSimu::isObstacle()
{
    if (!m_application) return false;
    switch(m_origine_detection) {
        case OBSTACLES_FROM_SIMU : return m_application->m_data_center->read("DetectionObstacle.isObstacle").toBool();
        case OBSTACLE_FROM_GUI : return (m_gui_detect_avg||m_gui_detect_avd||m_gui_detect_arg||m_gui_detect_ard);
        case OBSTACLES_FROM_TELEMETRES :
        default :
            return CDetectionObstaclesBase::isObstacle();
    }
}
//___________________________________________________________________________
bool CDetectionObstaclesSimu::isObstacleAVG()
{
    if (!m_application) return false;
    switch(m_origine_detection) {
        case OBSTACLES_FROM_SIMU : return (m_inhibe_detection_AVG == false) && m_application->m_data_center->read("DetectionObstacle.AVG").toBool();
        case OBSTACLE_FROM_GUI : return (m_inhibe_detection_AVG == false) && m_gui_detect_avg;
        case OBSTACLES_FROM_TELEMETRES :
        default :
            return CDetectionObstaclesBase::isObstacleAVG();
    }
}
//___________________________________________________________________________
bool CDetectionObstaclesSimu::isObstacleAVD()
{
    if (!m_application) return false;
    switch(m_origine_detection) {
        case OBSTACLES_FROM_SIMU : return (m_inhibe_detection_AVD == false) && m_application->m_data_center->read("DetectionObstacle.AVD").toBool();
        case OBSTACLE_FROM_GUI : return (m_inhibe_detection_AVD == false) && m_gui_detect_avd;
        case OBSTACLES_FROM_TELEMETRES :
        default :
            return CDetectionObstaclesBase::isObstacleAVD();
    }
}
//___________________________________________________________________________
bool CDetectionObstaclesSimu::isObstacleARG()
{
    if (!m_application) return false;
    switch(m_origine_detection) {
        case OBSTACLES_FROM_SIMU : return (m_inhibe_detection_ARG == false) && m_application->m_data_center->read("DetectionObstacle.ARG").toBool();
        case OBSTACLE_FROM_GUI : return (m_inhibe_detection_ARG == false) && m_gui_detect_arg;
        case OBSTACLES_FROM_TELEMETRES :
        default :
            return CDetectionObstaclesBase::isObstacleARG();
    }
}
//___________________________________________________________________________
bool CDetectionObstaclesSimu::isObstacleARD()
{
    if (!m_application) return false;
    switch(m_origine_detection) {
        case OBSTACLES_FROM_SIMU : return (m_inhibe_detection_ARD == false) && m_application->m_data_center->read("DetectionObstacle.ARD").toBool();
        case OBSTACLE_FROM_GUI : return (m_inhibe_detection_ARD == false) && m_gui_detect_ard;
        case OBSTACLES_FROM_TELEMETRES :
        default :
            return CDetectionObstaclesBase::isObstacleARD();
    }
}

//___________________________________________________________________________
void CDetectionObstaclesSimu::init(CApplication *application)
{
    m_application = application;
    Init();
}

//___________________________________________________________________________
void CDetectionObstaclesSimu::setOrigineDetection(int origine)
{
    m_origine_detection = origine;
}

//___________________________________________________________________________
void CDetectionObstaclesSimu::setObstacleDetecteFromGui(bool avg, bool avd, bool arg, bool ard)
{
    m_gui_detect_avg = avg;
    m_gui_detect_avd = avd;
    m_gui_detect_arg = arg;
    m_gui_detect_ard = ard;
}

// ___________________________________________________
void CDetectionObstaclesSimu::updateDataManager(QString dataname, QVariant val)
{
    if (!m_application) return;
    m_application->m_data_center->write(dataname, val);
}
