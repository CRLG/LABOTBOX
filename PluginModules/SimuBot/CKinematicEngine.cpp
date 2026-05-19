/*! \file CKinematicEngine.cpp
    \brief Implementation du moteur cinematique differentiel analytique.

    Modele :
      dG       = vect_deplacement_G * schedule_lap                (cm, deplacement roue G sur le tick)
      dD       = vect_deplacement_D * schedule_lap                (cm, deplacement roue D sur le tick)
      d_moy    = (dG + dD) / 2                                    (cm, deplacement du centre)
      d_theta  = (dD - dG) / voie                                 (rad, rotation sur le tick)

    Integration arc de cercle exacte (pas Euler grossier qui derive en virage) :
      Si |d_theta| < eps : translation pure dans la direction (teta + d_theta/2).
      Sinon              : rayon R = d_moy / d_theta, integration analytique
                           x += R * (sin(teta + d_theta) - sin(teta))
                           y += R * (cos(teta) - cos(teta + d_theta))

    Convention de signe :
      - teta trigonometrique (sens direct), x vers la droite, y vers le haut.
      - dG > dD  =>  d_theta < 0  =>  rotation vers la droite (sens horaire).

    Pas codeurs (sortie SIL) : bijection mathematique avec l'entree :
      m_delta_roue_G = round(dG / k_pas_codeur_G)
      m_delta_roue_D = round(dD / k_pas_codeur_D)
*/

#include "CKinematicEngine.h"
#include <math.h>

CKinematicEngine::CKinematicEngine()
    : m_x(0.0f),
      m_y(0.0f),
      m_teta(0.0f),
      m_delta_roue_G(0),
      m_delta_roue_D(0)
{
}

void CKinematicEngine::init(float x_init, float y_init, float teta_init)
{
    m_x    = x_init;
    m_y    = y_init;
    m_teta = teta_init;
    m_delta_roue_G = 0;
    m_delta_roue_D = 0;
}

void CKinematicEngine::step(float schedule_lap, float vect_deplacement_G, float vect_deplacement_D)
{
    // Conversion vitesse roue (cm/s) -> deplacement lineaire roue sur le tick (cm).
    const float dG = vect_deplacement_G * schedule_lap;
    const float dD = vect_deplacement_D * schedule_lap;

    const float d_moy   = 0.5f * (dG + dD);
    const float d_theta = (dD - dG) / k_voie_bot;

    if (fabsf(d_theta) < k_eps_dtheta)
    {
        // Translation quasi rectiligne : on prend l'orientation moyenne du tick
        // (teta + d_theta/2) pour rester exact au 2e ordre, sans risquer la division par zero.
        const float teta_mid = m_teta + 0.5f * d_theta;
        m_x += d_moy * cosf(teta_mid);
        m_y += d_moy * sinf(teta_mid);
    }
    else
    {
        // Integration arc de cercle exacte : on suit le cercle instantane
        // de rayon R = d_moy / d_theta sur toute la duree du tick.
        const float R = d_moy / d_theta;
        const float teta_new = m_teta + d_theta;
        m_x +=  R * (sinf(teta_new) - sinf(m_teta));
        m_y +=  R * (cosf(m_teta)   - cosf(teta_new));
    }

    m_teta += d_theta;

    // Pas codeurs simules (sortie principale du contrat SIL).
    // Round vers l'entier le plus proche pour reproduire le comportement d'un codeur reel.
    m_delta_roue_G = (int)roundf(dG / k_pas_codeur_G);
    m_delta_roue_D = (int)roundf(dD / k_pas_codeur_D);
}

void CKinematicEngine::recal(float x, float y, float teta)
{
    // Recalage absolu : ecrase la position sans toucher aux pas codeurs du dernier tick.
    // Equivaut a une correction odometrique externe (vision, balise, etc.).
    m_x    = x;
    m_y    = y;
    m_teta = teta;
}
