/*! \file CDetectionObstaclesSimu.h
    \brief Classe de simulation de la détection d'obstacles
*/

#ifndef _DETECTION_OBSTACLES_SIMU_H_
#define _DETECTION_OBSTACLES_SIMU_H_

#include <QString>
#include <QVariant>

#include "CDetectionObstaclesBase.h"

class CApplication;

class CDetectionObstaclesSimu : public CDetectionObstaclesBase
{
public :
    CDetectionObstaclesSimu();

    void init(CApplication *application);
    void Init();

    // Ré_impléméntation des méthodes virtuelles pures
    /*virtual*/ bool isObstacle();
    /*virtual*/ bool isObstacleAVG();
    /*virtual*/ bool isObstacleAVD();
    /*virtual*/ bool isObstacleARG();
    /*virtual*/ bool isObstacleARD();

    typedef enum {
        OBSTACLES_FROM_SIMU = 0,
        OBSTACLE_FROM_GUI,
        OBSTACLES_FROM_TELEMETRES,
    }tOrigineDetectionObstacle;
    void setOrigineDetection(int origine);
    void setObstacleDetecteFromGui(bool avg, bool avd, bool arg, bool ard);

private :
    CApplication *m_application;

    int m_origine_detection;

    bool m_gui_detect_avg;
    bool m_gui_detect_avd;
    bool m_gui_detect_arg;
    bool m_gui_detect_ard;

    void updateDataManager(QString dataname, QVariant val);
};

#endif



