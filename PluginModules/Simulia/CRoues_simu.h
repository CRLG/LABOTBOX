/*! \file CRoues.h
        \brief Classe qui contient la gestion des roues motrices gauche et droite
*/

#ifndef _ROUES_SIMU_H_
#define _ROUES_SIMU_H_

#include "CRouesBase.h"

class CApplication;
// -----------------------------
//! Classe de gestion des options d'exécution passees en ligne de commande
class CRouesSimu : public CRouesBase
{
public :
    CRouesSimu();

    /*virtual*/void AdapteCommandeMoteur_G(float vitesse);
    /*virtual*/void AdapteCommandeMoteur_D(float vitesse);
    /*virtual*/int getCodeurG(void);
    /*virtual*/int getCodeurD(void);
    /*virtual*/void resetCodeurs(void);

    void init_model();
    void step_model();

    void init(CApplication *application);

    void addSteps_Codeur_G(int nbSteps);
    void addSteps_Codeur_D(int nbSteps);

    float m_vect_deplacement_G;
    float m_vect_deplacement_D;

private :
    CApplication *m_application;

    int m_registre_Codeur_G;
    int m_registre_Codeur_D;
};

#endif


