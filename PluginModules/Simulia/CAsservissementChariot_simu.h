/*! \file CAsservissementChariotSimu.h
    \brief Classe qui contient l'asservissement vitesse/position du chariot du kmar
*/

#ifndef _ASSERVISSEMENT_CHARIOT_SIMU_H_
#define _ASSERVISSEMENT_CHARIOT_SIMU_H_

#include "CAsservissementChariotBase.h"

class CApplication;

class CAsservissementChariotSimu : public CAsservissementChariotBase
{
public :
    CAsservissementChariotSimu();

    void init(CApplication *application);
    virtual void Init();
    virtual void setConsigne(int pos);

    // Ré_impléméntation des méthodes virtuelles pures
    // en lien avec le hardware
    virtual bool isButeeBasse();
    virtual bool isButeeHaute();
    virtual int getPositionCodeur();
    virtual void resetPositionCodeur(int pos);
    virtual void commandeMoteur(float pourcent);

    void simu();
private :
    CApplication *m_application;

    float m_simu_position_codeur_chariot;
    float m_simu_commande_moteur_memo;
    void updateDataManager(QString dataname, QVariant val);
};

#endif



