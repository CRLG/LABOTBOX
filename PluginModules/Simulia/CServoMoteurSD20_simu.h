/*! \file CServoMoteurSD20_simu.h
    \brief Classe qui contient la simulation du SD20 pour les servos
*/

#ifndef _SERVO_MOTEUR_SD20_SIMU_H_
#define _SERVO_MOTEUR_SD20_SIMU_H_

#include "CServoMoteurSD20Base.h"

class CApplication;
// -----------------------------
//! Classe de gestion des options d'exécution passees en ligne de commande
class CServoMoteurSD20Simu  : public CServoMoteurSD20Base
{
public :
    CServoMoteurSD20Simu();
	
    /*virtual*/ void Init(void);
    /*virtual*/ void CommandePosition(unsigned char numServo, unsigned int position);

    void init(CApplication *application);

    // Fonction à appeler périodiquement
    void simu();

private :
    CApplication *m_application;
    void setDataManager(unsigned char numServo, unsigned int position);
};

#endif  // _SERVO_MOTEUR_SD20_SIMU_H_


