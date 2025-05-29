#ifndef _ASCENSEUR_SIMU_H_
#define _ASCENSEUR_SIMU_H_

#include "CAscenseurBase.h"

class CApplication;

// ====================================================
//
// ====================================================
class CAscenseurSimule : public CAscenseurBase
{
public:
    CAscenseurSimule();

    /*virtual*/void command_motor(signed char consigne_pourcent);  // consigne_pource positif pour le sens monter / négatif pour le sens descente
    /*virtual*/bool is_sensor_high();  // capteur de butée haute
    /*virtual*/bool is_sensor_low();   // capteur de butée basse

    /*virtual*/void periodicCall();

    void init(CApplication *application);
private :
    CApplication *m_application;

    int m_simu_position;
    int m_command_motor;
};

#endif // _ASCENSEUR_SIMU_H_
