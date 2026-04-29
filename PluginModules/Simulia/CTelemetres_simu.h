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

    // Ré_impléméntation des méthodes virtuelles pures
    /*virtual*/ float getDistanceAVG() override;
    /*virtual*/ float getDistanceAVD() override;
    /*virtual*/ float getDistanceARG() override;
    /*virtual*/ float getDistanceARD() override;
    /*virtual*/ float getDistanceARGCentre() override;
    /*virtual*/ float getDistanceARDCentre() override;
    /*virtual*/ bool init() override;
    /*virtual*/ void periodicTask() override;


    typedef enum {
        TELEMETRES_FROM_SIMU = 0,
        TELEMETRES_FROM_GUI,
        TELEMETRES_FROM_SIMUBOT,
    }tOrigineTelemetre;
    void setOrigineTelemetre(int origine);
    void setDistancesFromGui(float avg, float avd, float arg, float ard, float arg_centre, float ard_centre);

private :
    CApplication *m_application;

    int m_origine_telemetres;

    float m_gui_distance_avg;
    float m_gui_distance_avd;
    float m_gui_distance_arg;
    float m_gui_distance_ard;
    float m_gui_distance_arg_centre;
    float m_gui_distance_ard_centre;


    void updateDataManager(QString dataname, QVariant val);

};

#endif



