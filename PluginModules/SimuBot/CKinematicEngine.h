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

    // Parametres mecaniques - dupliques depuis CPhysicalEngine.h pour decouplage
    // (CPhysicalEngine sera supprime en etape 3). Static constexpr pour eviter
    // les conflits avec les macros #define existantes dans CPhysicalEngine.h.
    // IMPORTANT : ces 3 constantes DOIVENT etre identiques a celles de l'asserv qui ferme
    // la boucle SIL en mode SIMU, c.-a-d. CAsservissement_simu.cpp (et NON le firmware
    // embarque CAsservissement.cpp). L'asserv reconstruit la distance par
    // distance_roue = getCodeur() * DISTANCE_PAR_PAS_CODEUR : si le moteur emet ses pas
    // avec un k different de celui que l'asserv relit, la position ET la vitesse percues
    // sont faussees du rapport des deux k (le sprite part en vrille). Valeurs firmware
    // 27/04/2025 (0.003189567 / 0.003200126 / 30.7) volontairement NON utilisees ici :
    // l'asserv simu tourne encore sur les valeurs GROSBOT historiques ci-dessous.
    static constexpr float k_voie_bot       = 31.6867261f; // entraxe des roues en cm (= VOIE_ROBOT de CAsservissement_simu)
    static constexpr float k_pas_codeur_G   = 0.0651136f;  // cm par pas codeur, roue G (= DISTANCE_PAR_PAS_CODEUR_G de CAsservissement_simu)
    static constexpr float k_pas_codeur_D   = 0.0651136f;  // cm par pas codeur, roue D (= DISTANCE_PAR_PAS_CODEUR_D de CAsservissement_simu)

    // Seuil pour basculer en integration "ligne droite" (evite division par zero
    // quand le robot ne tourne pas) : delta theta inferieur a ~5.7e-5 degre.
    static constexpr float k_eps_dtheta = 1.0e-6f;
};

#endif // CKINEMATICENGINE_H
