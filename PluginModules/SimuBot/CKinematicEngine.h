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

    Cette classe est mono-bot et alignee sur l'API ISimulator visee en etape 2.
    A ce stade elle n'est branchee nulle part : elle coexiste avec CPhysicalEngine.
*/

#ifndef CKINEMATICENGINE_H
#define CKINEMATICENGINE_H

class CKinematicEngine
{
public:
    CKinematicEngine();

    // Reinitialise l'etat (position + angle) en repere terrain.
    // teta en radians, trigonometrique (sens direct, x vers la droite, y vers le haut).
    void init(float x_init, float y_init, float teta_init);

    // Avance la simulation d'un tick.
    // schedule_lap        : duree du tick en secondes (periode de l'asserv embarquee).
    // vect_deplacement_G  : vitesse lineaire roue gauche en cm/s (image de la PWM).
    // vect_deplacement_D  : vitesse lineaire roue droite en cm/s.
    // Met a jour (x, y, teta) par integration arc de cercle exacte,
    // et publie les pas codeurs correspondants (bijection avec l'entree).
    void step(float schedule_lap, float vect_deplacement_G, float vect_deplacement_D);

    // Force la position courante (recalage absolu) sans modifier les compteurs codeurs.
    void recal(float x, float y, float teta);

    // Pas codeurs cumules sur le dernier step() - sortie principale (lue par l'asserv).
    int deltaCodeurG() const { return m_delta_roue_G; }
    int deltaCodeurD() const { return m_delta_roue_D; }

    // Position courante - sortie secondaire (affichage / validation uniquement).
    float x() const    { return m_x; }
    float y() const    { return m_y; }
    float teta() const { return m_teta; }

private:
    // Etat interne en repere terrain unique (cm, rad).
    float m_x;
    float m_y;
    float m_teta;

    // Pas codeurs produits par le dernier step().
    int m_delta_roue_G;
    int m_delta_roue_D;

    // Parametres mecaniques - dupliques depuis CPhysicalEngine.h pour decouplage
    // (CPhysicalEngine sera supprime en etape 3). Static constexpr pour eviter
    // les conflits avec les macros #define existantes dans CPhysicalEngine.h.
    static constexpr float k_voie_bot       = 31.6867261f; // entraxe des roues en cm
    static constexpr float k_pas_codeur_G   = 0.0651136f;  // cm par pas codeur, roue G
    static constexpr float k_pas_codeur_D   = 0.0651136f;  // cm par pas codeur, roue D

    // Seuil pour basculer en integration "ligne droite" (evite division par zero
    // quand le robot ne tourne pas) : delta theta inferieur a ~5.7e-5 degre.
    static constexpr float k_eps_dtheta = 1.0e-6f;
};

#endif // CKINEMATICENGINE_H
