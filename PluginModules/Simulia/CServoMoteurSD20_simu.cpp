/*! \file CServoMoteurSD20Simu_simu.cpp
    \brief Classe qui contient les méthodes de simulation du SD20 pour servos
*/
#include "CGlobale.h"
#include "CServoMoteurSD20_simu.h"

#include "CApplication.h"
#include "CDataManager.h"

//___________________________________________________________________________
 /*!
   \brief Constructeur

   \param --
   \return --
*/
CServoMoteurSD20Simu::CServoMoteurSD20Simu()
    :m_application(nullptr)
{
  Init();
}

//___________________________________________________________________________
void CServoMoteurSD20Simu::init(CApplication *application)
{
    m_application = application;
}


//___________________________________________________________________________
 /*!
   \brief Initialisation des servos pour la configuration matérielle courante
   \return --
*/
void CServoMoteurSD20Simu::Init(void)
{
    // Initialisation des données
    for (int i=0; i<NBRE_SERVOS_SD20; i++) {
        m_position[i] = 0;
        m_position_utilisateur[i] = m_position[i];
        m_vitesse[i] = 0;
        m_pos_butee_min[i] = 0;
        m_pos_butee_max[i] = 0xFFFF;
        setDataManager(i, m_position[i]);
    }
}


//___________________________________________________________________________
void CServoMoteurSD20Simu::CommandePosition(unsigned char numServo, unsigned int position)
{
    if (numServo>NBRE_SERVOS_SD20) { return; }
    if (numServo == 0) { return; }

    numServo = numServo - 1;  // -1 car numServo varie entre 1 et 20 (et pas entre 0 et 19)

    // gestion des butées min et max
    position = saturePositionButees(numServo, position, m_pos_butee_min[numServo], m_pos_butee_max[numServo]);
    m_position_utilisateur[numServo] = position;
}

//___________________________________________________________________________
// Simule un déplacement du servo jusqu'à la position attendue
void CServoMoteurSD20Simu::simu(void)
{
  unsigned char i=0;
	
  for (i=0; i<NBRE_SERVOS_SD20; i++) {
      if (m_position[i] < m_position_utilisateur[i]) {
          m_position[i]++;
          setDataManager(i, m_position[i]);
      }
      else if (m_position[i] > m_position_utilisateur[i]) {
          m_position[i]--;
          setDataManager(i, m_position[i]);
      }
      // else : ne rien faire, le servo est déjà à la position attendue
  }
}

//___________________________________________________________________________
void CServoMoteurSD20Simu::setDataManager(unsigned char numServo, unsigned int position)
{
    numServo = numServo + 1;  // +1 pour correspondre au numéro utilisé en simulation
    QString data_name = "ServoSD20_" + QString::number(numServo) + ".Position";
    if (m_application) {
        m_application->m_data_center->write(data_name, position);
    }
}



