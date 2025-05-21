/*! \file CServomoteursSimu_simu.cpp
    \brief Classe qui contient les méthodes de simulation du  pour servos
*/
#include "CGlobale.h"
#include "CServomoteurs_simu.h"

#include "CApplication.h"
#include "CDataManager.h"

//___________________________________________________________________________
/*!
   \brief Constructeur

   \param --
   \return --
*/
CServomoteursSimu::CServomoteursSimu()
    :m_application(nullptr)
{
    Init();
}

//___________________________________________________________________________
void CServomoteursSimu::init(CApplication *application)
{
    m_application = application;
}


//___________________________________________________________________________
/*!
   \brief Initialisation des servos pour la configuration matérielle courante
   \return --
*/
void CServomoteursSimu::Init(void)
{
    // Initialisation des données
    for (int i=0; i<NBRE_SERVOS; i++) {
        m_position[i] = 0;
        m_position_utilisateur[i] = m_position[i];
        m_vitesse[i] = 0;
        m_pos_butee_min[i] = 0;
        m_pos_butee_max[i] = 0xFFFF;
        setDataManager(i, m_position[i]);
    }
}


//___________________________________________________________________________
void CServomoteursSimu::CommandeServo(unsigned char numServo, unsigned short position)
{
    if (numServo>NBRE_SERVOS) { return; }
    if (numServo == 0) { return; }
    setDataManager(numServo, position);
}



//___________________________________________________________________________
void CServomoteursSimu::setDataManager(unsigned char numServo, unsigned int position)
{
    numServo = numServo + 1;  // +1 pour correspondre au numéro utilisé en simulation
    QString data_name = "Servo_" + QString::number(numServo) + ".Position";
    if (m_application) {
        m_application->m_data_center->write(data_name, position);
    }
}



