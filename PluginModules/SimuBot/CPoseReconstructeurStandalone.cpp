/*! \file CPoseReconstructeurStandalone.cpp
 * Voir CPoseReconstructeurStandalone.h : copie autonome et fidele de CAsservissementBase::CalculXY
 * / setPosition_XYTeta, utilisee dans le build LaBotBox (sans logique robot). Toute divergence avec
 * CAsservissementBase ferait diverger LaBotBox de Simulia : garder aligne.
 */
#include "CPoseReconstructeurStandalone.h"
#include "math.h"

// MEMES valeurs que CAsservissementBase::* cote build Simulia (CAsservissement_simu.cpp).
// ETAPE 8quater : valeurs reprises de la source unique PluginModules/RobotGeometrySimu.h, plus
// aucun litteral ici. Cette copie autonome (build LaBotBox) reste ainsi alignee par CONSTRUCTION
// sur CAsservissement_simu (build Simulia) et sur CKinematicEngine, qui emet les pas codeurs.
#include "RobotGeometrySimu.h"
const float CPoseReconstructeurStandalone::DISTANCE_PAR_PAS_CODEUR_G = RobotGeometrySimu::DISTANCE_PAR_PAS_CODEUR_G;
const float CPoseReconstructeurStandalone::DISTANCE_PAR_PAS_CODEUR_D = RobotGeometrySimu::DISTANCE_PAR_PAS_CODEUR_D;
const float CPoseReconstructeurStandalone::VOIE_ROBOT = RobotGeometrySimu::VOIE_ROBOT;

void CPoseReconstructeurStandalone::setPosition_XYTeta(float x, float y, float teta)
{
    // Copie de CAsservissementBase::setPosition_XYTeta.
    X_robot = x;
    X_robot_prec = x;
    Y_robot = y;
    Y_robot_prec = y;
    angle_robot = teta;
    angle_robot_prec = teta;
}

void CPoseReconstructeurStandalone::CalculXY(void)
{
    // COPIE FIDELE de la partie "pose" de CAsservissementBase::CalculXY.
    // FACTEUR_CONV_DELTA_DIST_VERS_ANGLE d'origine = (1 / VOIE_ROBOT).
    // Les blocs vitesse (vitesse_avance/rotation + filtrage) et points A/B, sans effet sur
    // X_robot/Y_robot/angle_robot, sont volontairement omis (ils ne sont pas lus par CSimuBot).

    // On ne prend que les pas realises entre deux appels dans ddistance_roue_X.
    ddistance_roue_D = distance_roue_D - distance_roue_D_prec;
    distance_roue_D_prec = distance_roue_D; // offset remis a 0 par CSimuBot au raz

    ddistance_roue_G = distance_roue_G - distance_roue_G_prec;
    distance_roue_G_prec = distance_roue_G;

    // Estimations elementaires de la distance et de la rotation realisees par le robot.
    ddistance_robot = (ddistance_roue_D + ddistance_roue_G)*0.5f;
    dangle_robot = (ddistance_roue_D - ddistance_roue_G)*(1.0f / VOIE_ROBOT); // conversion distance -> angle [rad]

    // Angle robot en cumulant les rotations elementaires, normalise ]-pi ; pi] (litteraux 3.14/6.28
    // repris a l'identique de CAsservissementBase pour un comportement STRICTEMENT identique).
    angle_robot = dangle_robot + angle_robot_prec;
    distance_robot = (distance_roue_D + distance_roue_G)*0.5f;

    if (angle_robot > 3.14f)
    {
        angle_robot = angle_robot - 6.28f;
    }
    if (angle_robot <= -3.14f)
    {
        angle_robot = angle_robot + 6.28f;
    }
    angle_robot_prec = angle_robot;

    temp_cos_angle_robot = cos(angle_robot);
    temp_sin_angle_robot = sin(angle_robot);

    // Deplacements elementaires en X et Y, puis cumul de la position.
    dX_robot = ddistance_robot*temp_cos_angle_robot;
    dY_robot = ddistance_robot*temp_sin_angle_robot;

    X_robot = dX_robot + X_robot_prec;
    X_robot_prec = X_robot;

    Y_robot = dY_robot + Y_robot_prec;
    Y_robot_prec = Y_robot;
}
