//			Asservissement pour la simulation
//
/*! \file CAsservissementSimule.cpp

*/
#include "CAsservissement_simu.h"
#include "CGlobale.h"
#include "CApplication.h"

#include <QDebug>

// Constantes spécifiques au robot
const float CAsservissementBase::DISTANCE_PAR_PAS_CODEUR_G = 0.00325568f*20;  // valeur par défaut reprise de GROSBOT
const float CAsservissementBase::DISTANCE_PAR_PAS_CODEUR_D = 0.00325568f*20;
const float CAsservissementBase::VOIE_ROBOT = 31.6867261f;

// Cartos spécifique au robo
const float CAsservissementBase::ini_conv_erreur_dist_vitesse_cur_x[NBRE_POINTS_CARTO_ERREUR] = {-40, -20, -10, -4, -2, -1, 0, 1, 2, 4, 10, 20, 40};							// [cm]
const float CAsservissementBase::ini_conv_erreur_dist_vitesse_1_cur[NBRE_POINTS_CARTO_ERREUR] = {-50, -35, -20, -8, -2, -1, 0, 1, 2, 8, 20, 35, 50};						// [cm/s] Carto perfo 0% environ 30cm/s2
const float CAsservissementBase::ini_conv_erreur_dist_vitesse_2_cur[NBRE_POINTS_CARTO_ERREUR] = {-130, -100, -55, -25, -8, -4, 0, 4, 8, 25, 55, 100, 130};

const float CAsservissementBase::ini_conv_erreur_angle_vitesse_cur_x[NBRE_POINTS_CARTO_ERREUR] = {-1.6, -0.8, -0.4, -0.2, -0.1, -0.05, 0, 0.05, 0.1, 0.2, 0.4, 0.8, 1.6};	// [rad]
const float CAsservissementBase::ini_conv_erreur_angle_vitesse_1_cur[NBRE_POINTS_CARTO_ERREUR] = {-4, -2.8, -2, -1.4, -1, -0.5, 0, 0.5, 1, 1.4, 2, 2.8, 4};					// [rad/s] Carto perfo 0% environ 5rad/s2
const float CAsservissementBase::ini_conv_erreur_angle_vitesse_2_cur[NBRE_POINTS_CARTO_ERREUR] = {-8, -5.65, -4, -2.82, -2, -0.5, 0, 0.5, 2, 2.82, 4, 5.65, 8};				// [rad/s] Carto perfo 100% environ 20rad/s2


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
void CAsservissementSimule::Init(void)
{
    CAsservissementBase::Init();

    // re-initialisation des paramètres de l'asservissement pour la simu
    cde_max = 100;				// %	Commande maximum normalisée pour saturer la régulation
    cde_min = -100 ;			// %	Commande minimum normalisée pour saturer la régulation
    kp_distance =  1.6;//1.7;		// 		Gain proportionnel pour la régulation en distance
    ki_distance =  2.5;//3.0;		// 		Gain intégral pour la régulation en distance
    kp_angle =  15;			// 		Gain proportionnel pour la régulation en angle
    ki_angle =  5.5;			// 		Gain intégral pour la régulation en angle
    k_angle = 0.5;				//		Coeff de filtrage pour le terme dérivé
    seuil_conv_distance =  1;	// cm	Erreur en dessous de laquelle on considère que le robot est en position sur la distance
    seuil_conv_angle =  0.1;	// rad	Erreur en dessous de laquelle on considère que le robot est en position sur l'angle
    compteur_max = 3;			// 		Nombre de coups d'horloge (N*te) avant de confirmer que le robot est en position

    // Initialisation des zones mortes
    zone_morte_D = 0;
    zone_morte_G = 0;
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
