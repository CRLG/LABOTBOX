/*! \file CGlobale.h
        \brief Classe qui contient toute l'application
*/
#ifndef _GLOBALE_H_
#define _GLOBALE_H_

#include "CAsservissement_simu.h"
#include "CRoues_simu.h"
#include "powerelectrobotsimu.h"
#include "CServoMoteurAX_simu.h"
#include "CServoMoteurSD20_simu.h"
#include "CLed_simu.h"
#include "CLeds.h"
#include "MessengerXbeeNetwork_simu.h"
#include "CAsservissementChariot_simu.h"
#include "CTelemetres_simu.h"
#include "CLidar_simu.h"
#include "CDetectionObstacles_simu.h"
#include "CCapteurs_simu.h"
#include "kmar.h"
#include "ia.h"



// -----------------------------
//! Classe de gestion des options d'exécution passees en ligne de commande
class CGlobaleSimule {
public :
    CCapteursSimu               m_capteurs;
    CAsservissementSimule       m_asservissement;
    CRouesSimu                  m_roues;
    PowerElectrobotSimu         m_power_electrobot;
    CServoMoteurAXSimu          m_servos_ax;
    CServoMoteurSD20Simu        m_servos_sd20;
    MessengerXbeeNetworkSimu    m_messenger_xbee_ntw;
    CAsservissementChariotSimu  m_asservissement_chariot;
    CTelemetresSimu             m_telemetres;
    CLidarSimu                  m_lidar;
    CDetectionObstaclesSimu     m_detection_obstacles;
    CKmar                       m_kmar;
    IA                          m_modelia;

    // Gestion des LED
    CLedSimu m_led1;  // Ne pas utiliser directement led1...4 dans le modèle, c'est juste pour la simulation
    CLedSimu m_led2;
    CLedSimu m_led3;
    CLedSimu m_led4;
    CLeds m_leds;

    CGlobaleSimule();
    ~CGlobaleSimule();

};


extern CGlobaleSimule Application;


#endif 



