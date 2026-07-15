/*! \file CPoseReconstructeurStandalone.h
 * Reconstructeur de pose AUTONOME pour SimuBot (etape 6 migration v2).
 *
 * COPIE FIDELE de l'algorithme CAsservissementBase::CalculXY (schema "tourne-puis-avance") et de
 * setPosition_XYTeta, SANS aucune dependance a CAsservissementBase / CGlobale / la logique robot.
 *
 * Pourquoi : CSimuBot::updateStepFromSimuBot reconstruit la pose "comme le firmware" (l'asserv fait
 * foi, cf. rapport_simubot.md etape 3bis) via CAsservissementBase::CalculXY. Or CAsservissementBase
 * inclut CGlobale.h -> tire toute la logique robot, seulement disponible dans le build Simulia. Le
 * build LaBotBox (GUI robot reel + BlockBot, sans logique robot) ne peut donc pas la compiler.
 *
 * Choix (assume par l'utilisateur) : dans le build Simulia (define SIMUBOT_ROBOT_LOGIC) on utilise
 * le VRAI CAsservissementBase -> representativite maximale et synchro automatique avec le firmware.
 * Dans le build LaBotBox (define absent) on utilise cette copie autonome, au comportement IDENTIQUE
 * tant que l'algorithme et les constantes ci-dessous restent alignes sur CAsservissementBase. Une
 * evolution future de CAsservissementBase::CalculXY devra etre reportee ici manuellement (Simulia
 * reste la reference de representativite). Cf. CSimuBot.h (typedef CPoseReconstructeur sous #ifdef).
 */
#ifndef CPOSERECONSTRUCTEURSTANDALONE_H
#define CPOSERECONSTRUCTEURSTANDALONE_H

class CPoseReconstructeurStandalone
{
public:
    // Constantes geometriques : MEMES valeurs que CAsservissementBase::* cote build Simulia
    // (definies dans PluginModules/Simulia/CAsservissement_simu.cpp : 0.00325568*20 et 31.6867261).
    // A garder synchro manuellement avec ce fichier pour un comportement identique Simulia/LaBotBox.
    static const float DISTANCE_PAR_PAS_CODEUR_G;
    static const float DISTANCE_PAR_PAS_CODEUR_D;
    static const float VOIE_ROBOT;

    // Entrees (alimentees par CSimuBot, exactement comme pour CAsservissementBase) : distance
    // cumulee par roue [cm], et son offset "precedent" (remis a 0 au raz par CSimuBot).
    float distance_roue_D = 0.f;
    float distance_roue_G = 0.f;
    float distance_roue_D_prec = 0.f;
    float distance_roue_G_prec = 0.f;

    // Sorties : pose reconstruite en repere asservissement (comme CAsservissementBase).
    float X_robot = 0.f;
    float Y_robot = 0.f;
    float angle_robot = 0.f;

    // Fixe la pose (et son etat "precedent"), copie de CAsservissementBase::setPosition_XYTeta.
    void setPosition_XYTeta(float x, float y, float teta);
    // Reconstruit X_robot/Y_robot/angle_robot depuis distance_roue_D/G, copie de la partie pose
    // de CAsservissementBase::CalculXY (les blocs vitesse/filtrage et points A/B, sans effet sur
    // la pose, sont volontairement omis).
    void CalculXY(void);

private:
    // Etat interne miroir des membres de CAsservissementBase utilises par CalculXY.
    float X_robot_prec = 0.f;
    float Y_robot_prec = 0.f;
    float angle_robot_prec = 0.f;
    float ddistance_roue_D = 0.f;
    float ddistance_roue_G = 0.f;
    float ddistance_robot = 0.f;
    float dangle_robot = 0.f;
    float dX_robot = 0.f;
    float dY_robot = 0.f;
    float distance_robot = 0.f;
    float temp_cos_angle_robot = 0.f;
    float temp_sin_angle_robot = 0.f;
};

#endif // CPOSERECONSTRUCTEURSTANDALONE_H
