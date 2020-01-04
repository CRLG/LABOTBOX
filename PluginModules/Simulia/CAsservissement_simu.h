/*! \file CAsservissement.h
    \brief Classe qui contient l'asservissement vitesse/position du robot
*/

#ifndef _ASSERVISSEMENT_SIMU_H_
#define _ASSERVISSEMENT_SIMU_H_

#include "CAsservissementBase.h"

class CApplication;

// -----------------------------
class CAsservissementSimule : public CAsservissementBase
{
public :
    //! Constructeur / destructeur
    CAsservissementSimule();

    // Pour la simulation
    void init(CApplication *application);

    //! Réinitilise tous les paramètres et valeurs
    /*virtual*/ void Init(void);

    // Prototype des fonctions
    /*virtual */void CommandeMouvementXY(float x, float y);
    /*virtual */void CommandeMouvementDistanceAngle(float distance, float angle);
    /*virtual */void CommandeManuelle(float cde_G, float cde_D);
    /*virtual */void CommandeMouvementXY_A(float x, float y);
    /*virtual */void CommandeMouvementXY_B(float x, float y);
    /*virtual */void CommandeMouvementXY_TETA(float x, float y, float teta);
    /*virtual */void CommandeVitesseMouvement(float vit_avance, float vit_angle);
    /*virtual */void setPosition_XYTeta(float x, float y, float teta);
    /*virtual */void setIndiceSportivite(float idx);

private :
    CApplication *m_application;
};


#endif


