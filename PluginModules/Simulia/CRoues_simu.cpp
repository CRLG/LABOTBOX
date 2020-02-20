/*! \file CRoues_simu.cpp
    \brief Classe qui contient toute l'application
*/
#include "CRoues_simu.h"
#include "plateformer_robot.h"
#include <QDebug>
#include "CApplication.h"
#include "CDataManager.h"
//___________________________________________________________________________
/*!
   \brief Constructeur

   \param --
   \return --
*/
CRouesSimu::CRouesSimu()
    :m_application(nullptr),
      m_force_blocage_roue_G(false),
      m_force_blocage_roue_D(false)
{
    init_model();
}

//___________________________________________________________________________
void CRouesSimu::init(CApplication *application)
{
    m_application = application;
}


//___________________________________________________________________________
void CRouesSimu::forceBlocageRoueG(bool state)
{
    m_force_blocage_roue_G = state;
}
void CRouesSimu::forceBlocageRoueD(bool state)
{
    m_force_blocage_roue_D = state;
}

//___________________________________________________________________________
/*!
   \brief Applique la puissance au moteur gauche

   \param vitesse la vitesse signee en pourcentage [-100%;+100]
   \return --
*/
void CRouesSimu::AdapteCommandeMoteur_G(float vitesse)
{
    if (m_force_blocage_roue_G) {
        m_cde_roue_G = 0.f;
    }
    else {
        m_cde_roue_G = vitesse;
    }
    //qDebug() << "Moteur G = " << m_cde_roue_G;
    rtU.cde_mot_G = m_cde_roue_G;
    m_application->m_data_center->write("Cde_MotG", m_cde_roue_G);
}


//___________________________________________________________________________
/*!
   \brief Applique la puissance au moteur droit

   \param vitesse la vitesse signee en pourcentage [-100%;+100]
   \return --
*/
void CRouesSimu::AdapteCommandeMoteur_D(float vitesse)
{
    if (m_force_blocage_roue_D) {
        m_cde_roue_D = 0.f;
    }
    else {
        m_cde_roue_D = vitesse;
    }
    //qDebug() << "Moteur D = " << m_cde_roue_D;
    rtU.cde_mot_D = m_cde_roue_D;
    m_application->m_data_center->write("Cde_MotD", m_cde_roue_D);
}


//___________________________________________________________________________
/*!
   \brief Renvoie la position du codeur G

   \param --
   \return la position du codeur
*/
int CRouesSimu::getCodeurG(void)
{
    return rtY.RegistrecodeurG;
}


//___________________________________________________________________________
/*!
   \brief Renvoie la position du codeur D

   \param --
   \return la position du codeur
*/
int CRouesSimu::getCodeurD(void)
{
    return rtY.RegistrecodeurD;
}


//___________________________________________________________________________
/*!
   \brief Reset les pas cumules par les 2 ccodeurs G et D

   \param --
   \return --
*/
void CRouesSimu::resetCodeurs(void)
{
    rtY.RegistrecodeurG = 0;
    rtY.RegistrecodeurD = 0;
    memset(&rtDW, 0, sizeof(rtDW)); // RAZ nécessaire aussi des données internes au modèle (sinon, pas de réel RAZ des codeurs et les corrdonnées font un saut à la convergence)

    if (m_application) {
        m_application->m_data_center->write("CodeurRoueG", rtY.RegistrecodeurG);
        m_application->m_data_center->write("CodeurRoueD", rtY.RegistrecodeurD);
    }
}


// =======================================================================
//      Modèle comportemental des moteurs (matlab + génération de code)
// =======================================================================
void CRouesSimu::init_model()
{
    resetCodeurs();
    plateformer_robot_initialize();
    memset(&rtDW, 0, sizeof(rtDW));
    memset(rtM, 0, sizeof(RT_MODEL));
    memset(&rtU, 0, sizeof(rtU));
    memset(&rtY, 0, sizeof(rtY));
}

void CRouesSimu::step_model()
{
    static boolean_T OverrunFlag = false;

    /* Disable interrupts here */

    /* Check for overrun */
    if (OverrunFlag) {
        rtmSetErrorStatus(rtM, "Overrun");
        return;
    }

    OverrunFlag = true;

    /* Save FPU context here (if necessary) */
    /* Re-enable timer or interrupt here */
    /* Set model inputs here */

    // IHM -> Entrées modele
    rtU.ubatt = 15; //[V]

    // Mémorise l'ancienne position pour forcer le blocage
    real_T old_CodeurRoueG = rtY.RegistrecodeurG;
    real_T old_CodeurRoueD = rtY.RegistrecodeurD;

    /* Step the model */
    plateformer_robot_step();

    // Gestion du blocage de la roue
    if (m_force_blocage_roue_G) {
        rtY.RegistrecodeurG = old_CodeurRoueG;
    }
    if (m_force_blocage_roue_D) {
        rtY.RegistrecodeurD = old_CodeurRoueD;
    }

    /* Get model outputs here */
    m_application->m_data_center->write("CodeurRoueG", rtY.RegistrecodeurG);
    m_application->m_data_center->write("CodeurRoueD", rtY.RegistrecodeurD);

    /* Indicate task complete */
    OverrunFlag = false;

    /* Disable interrupts here */
    /* Restore FPU context here (if necessary) */
    /* Enable interrupts here */
}





