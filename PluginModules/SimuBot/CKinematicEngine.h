/*! \file CKinematicEngine.h
    \brief Moteur cinematique analytique pour un robot a 2 roues (etape 1 migration SimuBot v2).

    Cinematique differentielle pure, sans Box2D : par construction mathematique,
    pas d'effet "crabe" (zero composante laterale de la vitesse).

    Contrat Software-In-the-Loop (cf. rapport_simubot.md, section 6bis) :
    - Entree     : consigne moteur (image de la PWM), via vect_deplacement_G/D.
    - Sortie principale : pas codeurs simules (deltaCodeurG/D), lus par l'asserv firmware.
    - Sortie secondaire : position (x, y, teta) pour AFFICHAGE et validation uniquement.
    Ne jamais court-circuiter la boucle PWM -> simulateur -> pas codeurs -> asserv.

    Convention d'unite pour vect_deplacement_G/D : vitesse lineaire roue en cm/s.
    Le pas de temps est passe explicitement (schedule_lap, en secondes) -> dG = vect * dt.

    Cette classe est mono-bot et implemente l'interface ISimulator (etape 2).
    Branchee dans CSimuBot via la cle EEPROM "engine" (cf. rapport_simubot.md etape 2),
    en cohabitation avec CPhysicalEngine (supprime a l'etape 3).

    Collision bordure (etape 2) : modele disque (rayon englobant). On borne le centre
    du robot au terrain. Le modele de contact polygonal "coin qui accroche -> pivot ->
    calage" est repousse a l'etape 3 (collisions geometriques maison). Quand le clamp
    reduit le deplacement, les pas codeurs refletent le deplacement REELLEMENT effectue
    (codeurs sur roues folles independantes des roues motrices), pas la consigne moteur.
*/

#ifndef CKINEMATICENGINE_H
#define CKINEMATICENGINE_H

#include "ISimulator.h"
#include "RobotGeometrySimu.h"  // etape 8quater : source unique voie / pas codeurs

class CCollisionEngine; // collisions geometriques maison (etape 3), branchees optionnellement

class CKinematicEngine : public ISimulator
{
public:
    CKinematicEngine();

    // Reinitialise l'etat (position + angle) en repere terrain.
    // teta en radians, trigonometrique (sens direct, x vers la droite, y vers le haut).
    void init(float x_init, float y_init, float teta_init) override;

    // Avance la simulation d'un tick.
    // schedule_lap        : duree du tick en secondes (periode de l'asserv embarquee).
    // vect_deplacement_G  : vitesse lineaire roue gauche en cm/s (image de la PWM).
    // vect_deplacement_D  : vitesse lineaire roue droite en cm/s.
    // Met a jour (x, y, teta) par integration arc de cercle exacte (bornee au terrain),
    // et publie les pas codeurs correspondant au deplacement reellement effectue.
    void step(float schedule_lap, float vect_deplacement_G, float vect_deplacement_D) override;

    // Force la position courante (recalage absolu) sans modifier les compteurs codeurs.
    void recal(float x, float y, float teta) override;

    // Pas codeurs cumules sur le dernier step() - sortie principale (lue par l'asserv).
    int deltaCodeurG() const override { return m_delta_roue_G; }
    int deltaCodeurD() const override { return m_delta_roue_D; }

    // Position courante - sortie secondaire (affichage / validation uniquement).
    float x() const    override { return m_x; }
    float y() const    override { return m_y; }
    float teta() const override { return m_teta; }

    // Configuration collision bordure (hors interface ISimulator) :
    // limites du terrain en cm et rayon du cercle englobant le robot.
    // NB : utilises seulement par le modele disque de repli (collision engine absent).
    void setTerrain(float x_min, float y_min, float x_max, float y_max);
    void setRobotRadius(float radius);

    // Branche le moteur de collisions geometriques maison (etape 3). Quand il est present,
    // step() resout les collisions polygonales (bordures + elements de jeu, SAT) via ce
    // moteur et ignore le clamp disque ci-dessus. nullptr => repli sur le modele disque.
    // CKinematicEngine ne prend PAS la propriete du pointeur (detenu par CSimuBot).
    void setCollisionEngine(CCollisionEngine* engine) { m_collision = engine; }

    // Constante de temps moteur (s) du filtre premier ordre sur la vitesse roue.
    // L'asserv embarque est un asservissement EN VITESSE : sans inertie, la vitesse roue
    // rejoint instantanement la consigne et la boucle pompe (petits deplacements / convergence).
    // 0 => reponse instantanee (pas d'inertie).
    void setMotorTau(float tau);

private:
    // Etat interne en repere terrain unique (cm, rad).
    float m_x;
    float m_y;
    float m_teta;

    // Pas codeurs produits par le dernier step().
    int m_delta_roue_G;
    int m_delta_roue_D;

    // Modele de codeur incremental : on suit une distance cumulee (double, cf. step()) et le
    // nombre de pas cumule correspondant ; le delta emis est la DIFFERENCE de ce compteur.
    // C'est ce qui evite de perdre la fraction de pas non franchie a la fin de chaque tick
    // (biais systematique de -2,33 % mesure a 20 cm/s avant correctif). Remis a zero par init(),
    // JAMAIS par recal() (recalage de pose sans toucher aux codeurs, par contrat).
    double m_dist_cumul_G;
    double m_dist_cumul_D;
    long   m_steps_cumul_G;
    long   m_steps_cumul_D;

    // Inertie moteur (asserv en vitesse) : vitesse roue REELLE (cm/s), filtree premier ordre
    // vers la consigne, et constante de temps associee (s).
    float m_vG_actual;
    float m_vD_actual;
    float m_motor_tau;

    // Bornes terrain (cm) et rayon robot (cm) pour la collision bordure minimale.
    float m_x_min;
    float m_y_min;
    float m_x_max;
    float m_y_max;
    float m_robot_radius;

    // Moteur de collisions geometriques (etape 3), non possede. nullptr => modele disque.
    CCollisionEngine* m_collision;

    // Parametres mecaniques. ETAPE 8quater : ils ne sont plus recopies ici mais DERIVES de la
    // source unique RobotGeometrySimu.h -- ce moteur EMET les pas codeurs (steps = distance /
    // k_pas_codeur) que l'asserv RELIT (distance = steps * DISTANCE_PAR_PAS_CODEUR) : les deux
    // cotes de la boucle SIL ne peuvent donc plus diverger. Les alias ci-dessous sont conserves
    // pour ne rien changer au code de CKinematicEngine.cpp. Ne PAS y remettre de litteral, ni
    // les aligner sur le firmware embarque : lire l'en-tete de RobotGeometrySimu.h avant toute
    // modification (une tentative d'alignement firmware a deja casse la boucle d'un facteur ~20).
    static constexpr float k_voie_bot       = RobotGeometrySimu::VOIE_ROBOT;
    static constexpr float k_pas_codeur_G   = RobotGeometrySimu::DISTANCE_PAR_PAS_CODEUR_G;
    static constexpr float k_pas_codeur_D   = RobotGeometrySimu::DISTANCE_PAR_PAS_CODEUR_D;

    // Seuil pour basculer en integration "ligne droite" (evite division par zero
    // quand le robot ne tourne pas) : delta theta inferieur a ~5.7e-5 degre.
    static constexpr float k_eps_dtheta = 1.0e-6f;
};

#endif // CKINEMATICENGINE_H
