#include "CApplication.h"
#include "CDataManager.h"
#include "CAscenseur_simu.h"
#include "CGlobale.h"

CAscenseurSimule::CAscenseurSimule()
    : m_simu_position(0)
{
}

//___________________________________________________________________________
void CAscenseurSimule::init(CApplication *application)
{
    m_application = application;
    m_simu_position = 0;
}

// ___________________________________________
void CAscenseurSimule::command_motor(signed char consigne_pourcent)
{
    m_command_motor = consigne_pourcent;
}

// ___________________________________________
bool CAscenseurSimule::is_sensor_high()
{
    return (m_simu_position >=100);
}

// ___________________________________________
bool CAscenseurSimule::is_sensor_low()
{
    return (m_simu_position<=0);
}

// ___________________________________________
// Simule le mouvement mécanique si la moteur est piloté
void CAscenseurSimule::periodicCall()
{
    if (m_command_motor > 0) m_simu_position++;
    else if (m_command_motor < 0) m_simu_position--;
    // else ne rien faire
    CAscenseurBase::periodicCall();

    if (!m_application) return;
    m_application->m_data_center->write("Ascenseur.position", m_simu_position);
}
