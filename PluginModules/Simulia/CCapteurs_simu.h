/*! \file CCapteurs_simu.h
    \brief Classe de base pour la virtualisation des capteurs
*/

#ifndef _CAPTEURS_SIMU_H_
#define _CAPTEURS_SIMU_H_

#include "CCapteursBase.h"

class CApplication;

class CCapteursSimu : public CCapteursBase
{
public :
    CCapteursSimu();

    void init(CApplication *application);

    //! Initialisation des capteurs
    /*virtual*/ void Init(void);
    //! Traitement des capteurs (aqcuisition, filtrage)
    /*virtual*/ void Traitement(void);

    // ____________________________________________
    // Liste tous les capteurs possibles (pour tous les robots)
    /*virtual*/ bool getTirette();

    /*virtual*/ bool getContactRecalageAVG();
    /*virtual*/ bool getContactRecalageAVD();
    /*virtual*/ bool getContactRecalageARG();
    /*virtual*/ bool getContactRecalageARD();

    /*virtual*/ bool getAscenseurButeeHaute();
    /*virtual*/ bool getAscenseurButeeBasse();

    /*virtual*/ float getCapteurPressionKmar();

private :
    CApplication *m_application;
};
#endif


