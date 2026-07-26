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

    Pas codeurs (sortie SIL) : modele de codeur INCREMENTAL, qui compte une position absolue.
    On cumule la distance roue, on en deduit un nombre de pas cumule, et on emet sa DIFFERENCE :
      m_dist_cumul_G  += dG                                       (cm, distance roue cumulee)
      steps_cumul_G    = lround(m_dist_cumul_G / k_pas_codeur_G)   (pas cumules)
      m_delta_roue_G   = steps_cumul_G - steps_cumul_G_precedent   (pas emis sur le tick)
    Arrondir chaque tick isolement (ancienne formule round(dG / k_pas_codeur_G)) perdait la
    fraction de pas non franchie, d'ou un biais systematique a vitesse constante : -2,33 % sur
    la distance et sur l'angle a 20 cm/s avec dt = 20 ms. Avec le compteur cumule, l'erreur reste
    bornee a +/- 1 pas quelle que soit la duree du match.
*/

#include "CKinematicEngine.h"
#include "CCollisionEngine.h"
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
      m_dist_cumul_G(0.0),
      m_dist_cumul_D(0.0),
      m_steps_cumul_G(0),
      m_steps_cumul_D(0),
      m_vG_actual(0.0f),
      m_vD_actual(0.0f),
      m_motor_tau(0.15f),   // ~ equivalent de l'amortissement Box2D (linearDamping=7)
      m_x_min(0.0f),
      m_y_min(0.0f),
      m_x_max(300.0f),   // terrain 2026 : 300 x 200 cm (cf. X_TERRAIN/Y_TERRAIN)
      m_y_max(200.0f),
      m_robot_radius(15.0f),
      m_collision(nullptr)
{
}

void CKinematicEngine::init(float x_init, float y_init, float teta_init)
{
    m_x    = x_init;
    m_y    = y_init;
    m_teta = teta_init;
    m_delta_roue_G = 0;
    m_delta_roue_D = 0;
    // Compteurs codeurs cumules remis a zero : init() est un raz complet de la simulation
    // (nouveau match), pas un recalage. Le residu de pas du match precedent ne doit pas
    // deborder sur le suivant. NB : recal() ne les touche PAS, par contrat.
    m_dist_cumul_G = 0.0;
    m_dist_cumul_D = 0.0;
    m_steps_cumul_G = 0;
    m_steps_cumul_D = 0;
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

    // --- 3. Collision : on corrige la pose candidate pour qu'elle ne penetre aucun obstacle ---
    // Deux modeles selon la configuration :
    //  - Collision engine present (etape 3) : collisions polygonales (bordures + elements
    //    de jeu, SAT). Le robot est repousse hors des obstacles ; l'orientation n'est pas
    //    contrainte (resolution en translation, coherent avec le modele de l'etape 2).
    //  - Sinon : repli modele disque (etape 2) : on borne le centre au terrain.
    float x_real = x_cand;
    float y_real = y_cand;
    const float teta_real = teta_cand;
    if (m_collision != nullptr)
    {
        int hitId = -1;
        m_collision->resolve(x_real, y_real, teta_real, &hitId);
    }
    else
    {
        x_real = clampToRange(x_cand, m_x_min + m_robot_radius, m_x_max - m_robot_radius);
        y_real = clampToRange(y_cand, m_y_min + m_robot_radius, m_y_max - m_robot_radius);
    }
    const bool clamped = (x_real != x_cand) || (y_real != y_cand);

    // --- 4. Pas codeurs = image du deplacement REELLEMENT effectue ---
    // Les codeurs du vrai robot sont sur des roues folles independantes des roues
    // motrices : ils mesurent le deplacement reel du chassis, jamais la consigne moteur.
    // dG_eff/dD_eff = deplacement roue reellement realise sur le tick ; il alimente le
    // compteur cumule en fin de bloc (cf. emission des pas plus bas).
    float dG_eff = dG;
    float dD_eff = dD;
    if (!clamped)
    {
        // Espace libre : pose reelle == pose candidate -> bijection exacte avec la
        // consigne (garantit la fidelite des trajectoires droite / cercle / octogone).
    }
    else
    {
        // Contact (bordure ou element) : on reconstruit l'avance reelle du centre (projetee
        // sur le cap moyen du tick) puis on revient aux roues par cinematique inverse.
        // La rotation n'etant pas contrainte, d_theta_real == d_theta.
        const float d_theta_real  = teta_real - teta_old;
        const float cap_moyen     = teta_old + 0.5f * d_theta_real;
        const float d_center_real = (x_real - m_x) * cosf(cap_moyen)
                                  + (y_real - m_y) * sinf(cap_moyen);

        // Cinematique inverse : dD = d_center + d_theta*voie/2 ; dG = d_center - d_theta*voie/2.
        dG_eff = d_center_real - 0.5f * d_theta_real * k_voie_bot;
        dD_eff = d_center_real + 0.5f * d_theta_real * k_voie_bot;

        // --- CONTRAINTE NON-HOLONOME (roues silicone larges : ripage lateral quasi nul) ---
        // La resolution geometrique de collision (CCollisionEngine) peut deplacer le chassis
        // LATERALEMENT (glissement perpendiculaire au cap) pour le sortir d'un obstacle. Ce
        // mouvement lateral n'est PAS physique pour ce robot (les roues motrices grippantes
        // l'en empechent) ET il n'est pas mesurable par l'odometrie 2 roues (les codeurs
        // ci-dessus ne l'encodent pas -> ils ne voient que l'avance le long du cap). Garder
        // x_real/y_real ferait donc DIVERGER la pose physique de la croyance de l'asservissement
        // (reconstruite depuis ces memes codeurs) : c'est l'origine du "sprite fantome" qui
        // deplace les elements de jeu a une position decalee de l'affichage.
        // On projette donc la pose commise sur le cap : le chassis n'avance QUE le long de son
        // axe de roulement, coherent avec les codeurs. En espace libre c'est un no-op exact
        // (le deplacement en arc est deja porte par le cap : la corde d'un arc bissecte l'angle).
        // Un contact oblique peut laisser un leger chevauchement residuel (le robot s'arrete au
        // lieu de glisser) : assume, second ordre (on recode la strategie pour eviter le contact).
        x_real = m_x + d_center_real * cosf(cap_moyen);
        y_real = m_y + d_center_real * sinf(cap_moyen);
    }

    // --- 5. Emission des pas codeurs : delta d'un COMPTEUR CUMULE ---
    // Un codeur incremental compte une position ABSOLUE : la fraction de pas non encore
    // franchie a la fin d'un tick n'est pas perdue, elle est reprise au tick suivant. On
    // reproduit exactement cela -- distance cumulee -> nombre de pas cumule -> difference --
    // au lieu d'arrondir chaque tick isolement.
    //
    // POURQUOI C'EST IMPORTANT : arrondir tick par tick introduit un biais SYSTEMATIQUE des que
    // la partie fractionnaire tombe toujours du meme cote, ce qui est le cas a vitesse constante.
    // Exemple mesure a 20 cm/s avec dt = 20 ms : un tick vaut 6,14 pas -> 6 emis, soit -2,33 %
    // sur la distance ET sur l'angle, a chaque tick, cumulatif sur tout le match. La croyance de
    // l'asserv (reconstruite depuis ces pas, cf. etape 3bis) derivait donc de la pose physique
    // pour une raison purement numerique, sans contrepartie physique -- un vrai codeur ne perd
    // pas ses fractions. Avec le compteur cumule, l'erreur reste bornee a +/- 1 pas (0,065 cm)
    // quelle que soit la duree, au lieu de croitre indefiniment.
    //
    // Accumulateurs en double : sur un match entier la distance cumulee atteint plusieurs
    // milliers de cm, ou la resolution d'un float (~1e-3 cm a 1e4 cm) deviendrait comparable a
    // la fraction de pas que l'on cherche justement a preserver.
    m_dist_cumul_G += dG_eff;
    m_dist_cumul_D += dD_eff;
    const long steps_cumul_G = lround(m_dist_cumul_G / (double)k_pas_codeur_G);
    const long steps_cumul_D = lround(m_dist_cumul_D / (double)k_pas_codeur_D);
    m_delta_roue_G = (int)(steps_cumul_G - m_steps_cumul_G);
    m_delta_roue_D = (int)(steps_cumul_D - m_steps_cumul_D);
    m_steps_cumul_G = steps_cumul_G;
    m_steps_cumul_D = steps_cumul_D;

    // --- 6. Validation de la nouvelle pose ---
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
