/*! \file CServomoteurs_simu.h
    \brief Classe qui contient la gestion des roues motrices gauche et droite
*/

#ifndef _SERVOMOTEURS_SIMU_H_
#define _SERVOMOTEURS_SIMU_H_

#include "CServomoteursBase.h"

class CApplication;

// -----------------------------
//! Classe de gestion des options d'exécution passees en ligne de commande
class CServomoteursSimu : public CServomoteursBase
{
public :
    CServomoteursSimu();

    /*virtual*/ void CommandeServo(unsigned char numServo, unsigned short position_usec);
    void Init();

    void init(CApplication *application);

    // Fonction à appeler périodiquement
    void simu();

private :
    CApplication *m_application;

    //! Memorise la position relle envoyee au servo
    unsigned int m_position[NBRE_SERVOS];
    //! Memorise la consigne de position demandée par l'applicatif
    unsigned int m_position_utilisateur[NBRE_SERVOS];

    void setDataManager(unsigned char numServo, unsigned int position);
};

#endif  // _SERVOMOTEURS_SIMU_H_
