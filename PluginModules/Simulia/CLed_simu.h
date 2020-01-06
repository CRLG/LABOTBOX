/*! \file CLedSimu.h
    \brief Classe de base pour la gestion d'une LED
*/

#ifndef _LED_SIMU_H_
#define _LED_SIMU_H_

#include "CLedBase.h"

#include <QString>

class CApplication;
// ============================================================
//        Gestion d'une LED
// ============================================================
//! Classe de gestion d'une LED
//! Virtuelle pure (méthodes en lien avec le hardware à ré-impléménter sur la cible)
class CLedSimu : public CLedBase
{
public :
    CLedSimu(QString name);


    void Init();
    void init(CApplication *application);
protected :
    // Méthodes vrituelles pures à ré-implémenter pour faire le lien avec le hardware
    /*virtual*/ void _write(bool val);
    /*virtual*/ bool _read();
private :
    QString m_name;
    bool m_state;
    CApplication *m_application;
};

#endif


