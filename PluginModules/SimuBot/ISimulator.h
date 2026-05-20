/*! \file ISimulator.h
    \brief Interface abstraite d'un moteur de simulation mono-robot (etape 2 migration SimuBot v2).

    Contrat Software-In-the-Loop (cf. rapport_simubot.md, sections 6bis et 8 etape 2) :
    - Entree            : consigne moteur (image de la PWM), via les vitesses roue vect_G/vect_D.
                          JAMAIS une position cible.
    - Sortie principale : pas codeurs simules (deltaCodeurG/D), lus par l'asserv firmware
                          pour fermer la boucle de regulation.
    - Sortie secondaire : pose (x, y, teta) pour AFFICHAGE et validation uniquement,
                          JAMAIS pour fermer la boucle.

    Une instance modelise UN seul robot. Le robot adverse (bot2) est une autre instance,
    alimentee par une instance Simulia externe (cf. CSimuBot::on_active_external_robot2).

    Implementations :
    - CKinematicEngine : cinematique differentielle analytique (cible v2, sans crabe).
    - CPhysicalEngine  : moteur Box2D historique bi-robot ; il N'implemente PAS cette
                         interface (il gere le monde dual + elements de jeu + recal et part
                         a l'etape 3). CSimuBot le pilote concretement tant qu'il existe.
*/

#ifndef ISIMULATOR_H
#define ISIMULATOR_H

class ISimulator
{
public:
    virtual ~ISimulator() = default;

    // Reinitialise la pose (repere terrain unique, teta en radians trigonometriques).
    virtual void init(float x_init, float y_init, float teta_init) = 0;

    // Avance la simulation d'un tick.
    // schedule_lap  : duree du tick en secondes (periode de l'asserv embarquee).
    // vect_G/vect_D : vitesse lineaire roue gauche/droite en cm/s (image de la PWM).
    virtual void step(float schedule_lap, float vect_G, float vect_D) = 0;

    // Recalage absolu de la pose (correction odometrique externe), sans toucher aux codeurs.
    virtual void recal(float x, float y, float teta) = 0;

    // Sortie principale SIL : pas codeurs cumules sur le dernier step().
    virtual int deltaCodeurG() const = 0;
    virtual int deltaCodeurD() const = 0;

    // Sortie secondaire : pose courante (affichage / validation uniquement).
    virtual float x() const = 0;
    virtual float y() const = 0;
    virtual float teta() const = 0;
};

#endif // ISIMULATOR_H
