/*! \file CGlobale.h
        \brief Classe qui contient toute l'application
*/
#ifndef _GLOBALE_H_
#define _GLOBALE_H_

#include "CAsservissement.h"
#include "powerelectrobotsimu.h"

// -----------------------------
//! Classe de gestion des options d'exécution passees en ligne de commande
class CGlobale {
public :
    //! L'asservissement de vitesse/position du robot
    // ATTENTION : l'instance de la classe asservisement doit être mise après l'instance de eeprom car CAsservissement utilise CEEPROM dans son constructeur

    CAsservissement m_asservissement;
    //! Carte PowerElectrobot
    PowerElectrobotSimu m_power_electrobot;

    CGlobale();
    ~CGlobale();
};


extern CGlobale Application;


#endif 



