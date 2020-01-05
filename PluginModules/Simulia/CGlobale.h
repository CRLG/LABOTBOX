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


// -----------------------------
//! Classe de gestion des options d'exécution passees en ligne de commande
class CGlobaleSimule {
public :
    CAsservissementSimule m_asservissement;
    CRouesSimu m_roues;
    //! Carte PowerElectrobot
    PowerElectrobotSimu m_power_electrobot;
    CServoMoteurAXSimu m_servos_ax;
    CServoMoteurSD20Simu m_servos_sd20;

    CGlobaleSimule();
    ~CGlobaleSimule();
};


extern CGlobaleSimule Application;


#endif 



