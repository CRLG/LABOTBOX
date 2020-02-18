/*! \file CTelemetresSimu.h
    \brief Classe de simulation des télémètres
*/

#ifndef _TELEMETRES_SIMU_H_
#define _TELEMETRES_SIMU_H_

#include <QString>
#include <QVariant>

#include "CTelemetresBase.h"

class CApplication;

class CTelemetresSimu : public CTelemetresBase
{
public :
    CTelemetresSimu();

    void init(CApplication *application);
    void Init();

    // Ré_impléméntation des méthodes virtuelles pures
    /*virtual*/ float getDistanceAVG();
    /*virtual*/ float getDistanceAVD();
    /*virtual*/ float getDistanceARG();
    /*virtual*/ float getDistanceARD();

    typedef enum {
        TELEMETRES_FROM_SIMU = 0,
        TELEMETRES_FROM_GUI,
        TELEMETRES_FROM_SIMUBOT,
    }tOrigineTelemetre;
    void setOrigineTelemetre(int origine);
    void setDistancesFromGui(float avg, float avd, float arg, float ard);

private :
    CApplication *m_application;

    int m_origine_telemetres;

    float m_gui_distance_avg;
    float m_gui_distance_avd;
    float m_gui_distance_arg;
    float m_gui_distance_ard;


    void updateDataManager(QString dataname, QVariant val);

};

#endif



