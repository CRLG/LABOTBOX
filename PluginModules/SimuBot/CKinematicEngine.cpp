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

// Borne v dans [lo, hi]. Robuste si lo > hi (rayon robot > demi-terrain) : on renvoie
// le centre du terrain plutot qu'une valeur incoherente.
static inline float clampToRange(float v, float lo, float hi)
{
    if (lo > hi) return 0.5f * (lo + hi);
    if (v < lo)  return lo;
    if (v > hi)  return hi;
    return v;
}

CKinematicEngine::CKinematicEngine()
    : m_x(0.0f),
      m_y(0.0f),
      m_teta(0.0f),
      m_delta_roue_G(0),
      m_delta_roue_D(0),
      m_vG_actual(0.0f),
      m_vD_actual(0.0f),
      m_motor_tau(0.15f),   // ~ equivalent de l'amortissement Box2D (linearDamping=7)
      m_x_min(0.0f),
      m_y_min(0.0f),
      m_x_max(300.0f),   // terrain 2026 : 300 x 200 cm (cf. X_TERRAIN/Y_TERRAIN)
      m_y_max(200.0f),
      m_robot_radius(15.0f)
{
}

void CKinematicEngine::init(float x_init, float y_init, float teta_init)
{
    m_x    = x_init;
    m_y    = y_init;
    m_teta = teta_init;
    m_delta_roue_G = 0;
    m_delta_roue_D = 0;
    // Reset de l'etat d'inertie : a l'init le robot est a l'arret.
    m_vG_actual = 0.0f;
    m_vD_actual = 0.0f;
}

void CKinematicEngine::setTerrain(float x_min, float y_min, float x_max, float y_max)
{
    m_x_min = x_min;
    m_y_min = y_min;
    m_x_max = x_max;
    m_y_max = y_max;
}

void CKinematicEngine::setRobotRadius(float radius)
{
    m_robot_radius = radius;
}

void CKinematicEngine::setMotorTau(float tau)
{
    m_motor_tau = tau;
}

void CKinematicEngine::step(float schedule_lap, float vect_deplacement_G, float vect_deplacement_D)
{
    // --- 0. Inertie moteur (asserv EN VITESSE) ---
    // La vitesse roue reelle ne rejoint pas instantanement la consigne : filtre premier ordre
    // de constante de temps m_motor_tau. Indispensable pour ne pas faire pomper la boucle
    // vitesse de l'asserv (Box2D fournissait cet effet via son amortissement). Forme exacte
    // pour une entree constante sur le tick : v <- v_cmd + (v - v_cmd) * exp(-dt/tau).
    if (m_motor_tau > 0.0f)
    {
        const float decay = expf(-schedule_lap / m_motor_tau);
        m_vG_actual = vect_deplacement_G + (m_vG_actual - vect_deplacement_G) * decay;
        m_vD_actual = vect_deplacement_D + (m_vD_actual - vect_deplacement_D) * decay;
    }
    else
    {
        m_vG_actual = vect_deplacement_G;
        m_vD_actual = vect_deplacement_D;
    }

    // --- 1. Vitesse roue reelle -> deplacement roue sur le tick (cm) ---
    const float dG = m_vG_actual * schedule_lap;
    const float dD = m_vD_actual * schedule_lap;

    const float d_moy   = 0.5f * (dG + dD);
    const float d_theta = (dD - dG) / k_voie_bot;

    // --- 2. Pose candidate (avant prise en compte des bordures) ---
    const float teta_old = m_teta;
    float x_cand = m_x;
    float y_cand = m_y;

    if (fabsf(d_theta) < k_eps_dtheta)
    {
        // Translation quasi rectiligne : on prend l'orientation moyenne du tick
        // (teta + d_theta/2) pour rester exact au 2e ordre, sans risquer la division par zero.
        const float teta_mid = teta_old + 0.5f * d_theta;
        x_cand += d_moy * cosf(teta_mid);
        y_cand += d_moy * sinf(teta_mid);
    }
    else
    {
        // Integration arc de cercle exacte : on suit le cercle instantane
        // de rayon R = d_moy / d_theta sur toute la duree du tick.
        const float R = d_moy / d_theta;
        const float teta_new = teta_old + d_theta;
        x_cand += R * (sinf(teta_new) - sinf(teta_old));
        y_cand += R * (cosf(teta_old) - cosf(teta_new));
    }
    const float teta_cand = teta_old + d_theta;

    // --- 3. Collision bordure (modele disque) : on borne le centre au terrain ---
    // L'orientation n'est pas contrainte (le contact polygonal coin/pivot est repousse
    // a l'etape 3). Le clamp peut reduire la translation effective.
    const float x_real    = clampToRange(x_cand, m_x_min + m_robot_radius, m_x_max - m_robot_radius);
    const float y_real    = clampToRange(y_cand, m_y_min + m_robot_radius, m_y_max - m_robot_radius);
    const float teta_real = teta_cand;
    const bool  clamped   = (x_real != x_cand) || (y_real != y_cand);

    // --- 4. Pas codeurs = image du deplacement REELLEMENT effectue ---
    // Les codeurs du vrai robot sont sur des roues folles independantes des roues
    // motrices : ils mesurent le deplacement reel du chassis, jamais la consigne moteur.
    if (!clamped)
    {
        // Espace libre : pose reelle == pose candidate -> bijection exacte avec la
        // consigne (garantit la fidelite des trajectoires droite / cercle / octogone).
        m_delta_roue_G = (int)roundf(dG / k_pas_codeur_G);
        m_delta_roue_D = (int)roundf(dD / k_pas_codeur_D);
    }
    else
    {
        // Contact bordure : on reconstruit l'avance reelle du centre (projetee sur le
        // cap moyen du tick) puis on revient aux roues par cinematique inverse.
        // La rotation n'etant pas contrainte (disque), d_theta_real == d_theta.
        const float d_theta_real  = teta_real - teta_old;
        const float cap_moyen     = teta_old + 0.5f * d_theta_real;
        const float d_center_real = (x_real - m_x) * cosf(cap_moyen)
                                  + (y_real - m_y) * sinf(cap_moyen);

        // Cinematique inverse : dD = d_center + d_theta*voie/2 ; dG = d_center - d_theta*voie/2.
        const float dG_real = d_center_real - 0.5f * d_theta_real * k_voie_bot;
        const float dD_real = d_center_real + 0.5f * d_theta_real * k_voie_bot;
        m_delta_roue_G = (int)roundf(dG_real / k_pas_codeur_G);
        m_delta_roue_D = (int)roundf(dD_real / k_pas_codeur_D);
    }

    // --- 5. Validation de la nouvelle pose ---
    m_x    = x_real;
    m_y    = y_real;
    m_teta = teta_real;
}

void CKinematicEngine::recal(float x, float y, float teta)
{
    // Recalage absolu : ecrase la position sans toucher aux pas codeurs du dernier tick.
    // Equivaut a une correction odometrique externe (vision, balise, etc.).
    m_x    = x;
    m_y    = y;
    m_teta = teta;
}
