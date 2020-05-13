/*! \file CRoues_simu.cpp
    \brief Classe qui contient toute l'application
*/
#include "CRoues_simu.h"
#include "plateformer_robot.h"
#include "CAsservissement_simu.h"
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
    :m_application(nullptr)
{
    init_model();
}

//___________________________________________________________________________
void CRouesSimu::init(CApplication *application)
{
    m_application = application;
    m_vect_deplacement_G=0;
    m_vect_deplacement_D=0;
    m_registre_Codeur_G=0;
    m_registre_Codeur_D=0;
}

void CRouesSimu::addSteps_Codeur_G(int nbSteps)
{
    m_registre_Codeur_G=m_registre_Codeur_G+nbSteps;
    m_application->m_data_center->write("CodeurRoueG", m_registre_Codeur_G);
}

void CRouesSimu::addSteps_Codeur_D(int nbSteps)
{
    m_registre_Codeur_D=m_registre_Codeur_D+nbSteps;
    m_application->m_data_center->write("CodeurRoueD", m_registre_Codeur_D);
}

//___________________________________________________________________________
/*!
   \brief Applique la puissance au moteur gauche

   \param vitesse la vitesse signee en pourcentage [-100%;+100]
   \return --
*/
void CRouesSimu::AdapteCommandeMoteur_G(float vitesse)
{
    m_cde_roue_G = vitesse;
}

//___________________________________________________________________________
/*!
   \brief Applique la puissance au moteur droit

   \param vitesse la vitesse signee en pourcentage [-100%;+100]
   \return --
*/
void CRouesSimu::AdapteCommandeMoteur_D(float vitesse)
{
    m_cde_roue_D = vitesse;
}


//___________________________________________________________________________
/*!
   \brief Renvoie la position du codeur G

   \param --
   \return la position du codeur
*/
int CRouesSimu::getCodeurG(void)
{
    return m_registre_Codeur_G;
}


//___________________________________________________________________________
/*!
   \brief Renvoie la position du codeur D

   \param --
   \return la position du codeur
*/
int CRouesSimu::getCodeurD(void)
{
    return m_registre_Codeur_D;
}


//___________________________________________________________________________
/*!
   \brief Reset les pas cumules par les 2 ccodeurs G et D

   \param --
   \return --
*/
void CRouesSimu::resetCodeurs(void)
{
    m_registre_Codeur_G=0;
    m_registre_Codeur_D=0;
    memset(&rtDW, 0, sizeof(rtDW)); // RAZ nécessaire aussi des données internes au modèle (sinon, pas de réel RAZ des codeurs et les corrdonnées font un saut à la convergence)

    if (m_application)
    {
        m_application->m_data_center->write("CodeurRoueG", m_registre_Codeur_G);
        m_application->m_data_center->write("CodeurRoueD", m_registre_Codeur_D);
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
    rtU.cde_mot_D = m_cde_roue_D;
    m_application->m_data_center->write("Cde_MotD", m_cde_roue_D);
    rtU.cde_mot_G = m_cde_roue_G;
    m_application->m_data_center->write("Cde_MotG", m_cde_roue_G);

    /* Step the model */
    plateformer_robot_step();

    /* Get model outputs here */
    float K=80;
    m_vect_deplacement_G=K*m_cde_roue_G;
    m_vect_deplacement_D=K*m_cde_roue_D;

    /* Indicate task complete */
    OverrunFlag = false;

    /* Disable interrupts here */
    /* Restore FPU context here (if necessary) */
    /* Enable interrupts here */
}





