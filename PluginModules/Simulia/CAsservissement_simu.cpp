//			Asservissement pour la simulation
//
/*! \file CAsservissementSimule.cpp

*/
#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

#include "math.h"
#include "CAsservissement_simu.h"
#include "CGlobale.h"
#include "CApplication.h"

#include <QDebug>


//___________________________________________________________________________
 /*!
   \brief Constructeur

   \param --
   \return --
*/
CAsservissementSimule::CAsservissementSimule()
{
  Init();
}


void CAsservissementSimule::init(CApplication *application)
{
    m_application = application;
}

//___________________________________________________________________________
 /*!
   \brief Initialisation

   \param --
   \return --
*/
const float CAsservissementBase::DISTANCE_PAR_PAS_CODEUR_G = 0.00325568f;  // valeur par défaut reprise de GROSBOT
const float CAsservissementBase::DISTANCE_PAR_PAS_CODEUR_D = 0.00325568f;
const float CAsservissementBase::VOIE_ROBOT = 31.6867261f;
void CAsservissementSimule::Init(void)
{
    CAsservissementBase::Init();
}

// ************************************************************************************
// Fonctions externes de haut niveau pour demander des mouvements
// ************************************************************************************

// ----------------------------------------------------------
// Décodage de la trame et affectation des variables
void CAsservissementSimule::CommandeMouvementXY(float x, float y)
{
    CAsservissementBase::CommandeMouvementXY(x, y);
}

// -------------------------------------
// Décodage de la trame et affectation des variables
void CAsservissementSimule::CommandeMouvementDistanceAngle(float distance, float angle)
{
    CAsservissementBase::CommandeMouvementDistanceAngle(distance, angle);
} 

// -------------------------------------
// Décodage de la trame et affectation des variables
void CAsservissementSimule::CommandeManuelle(float cde_G, float cde_D)
{
    CAsservissementBase::CommandeManuelle(cde_G, cde_D);
    qDebug() << "CommandeManuelle(" << cde_G << ", " << cde_D << ")";
}

// ----------------------------------------------------------
// Décodage de la trame et affectation des variables 
void CAsservissementSimule::CommandeMouvementXY_A(float x, float y)
{
    CAsservissementBase::CommandeMouvementXY_A(x, y);
}

// ----------------------------------------------------------
// Décodage de la trame et affectation des variables
void CAsservissementSimule::CommandeMouvementXY_B(float x, float y)
{
    CAsservissementBase::CommandeMouvementXY_B(x, y);
}


// -------------------------------------
// Décodage de la trame et affectation des variables
void CAsservissementSimule::CommandeVitesseMouvement(float vit_avance, float vit_angle)
{
    CAsservissementBase::CommandeVitesseMouvement(vit_avance, vit_angle);
}

// ----------------------------------------------------------
// Décodage de la trame et affectation des variables
void CAsservissementSimule::CommandeMouvementXY_TETA(float x, float y, float teta)
{
    CAsservissementBase::CommandeMouvementXY_TETA(x, y, teta);
}
// -----------------------------------------------------------------------------------------------------------------------
// Fonction qui initialise la position du robot avec des valeurs données
void CAsservissementSimule::setPosition_XYTeta(float x, float y, float teta)
{
    CAsservissementBase::setPosition_XYTeta(x, y, teta);
}

void CAsservissementSimule::setIndiceSportivite(float idx)
{
    CAsservissementBase::setIndiceSportivite(idx);
}
