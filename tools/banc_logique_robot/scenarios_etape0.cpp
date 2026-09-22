#include <cstdio>
#include "banc.h"
#include "scenarios.h"
#include "Lidar_utils.h"

void trace_match(Banc &banc, int duree_s)
{
    banc.reinitialiser();
    banc.demarrerMatch(0, SM_DatasInterface::EQUIPE_COULEUR_1);
    for (int t = 0; t < duree_s * 5; t++) {        // une ligne toutes les 200 ms
        banc.passagesModele(10);
        SM_InputsInterface *in = banc.entrees();
        SM_DatasInterface *d = banc.donnees();
        printf("t=%5.1fs X=%7.1f Y=%7.1f th=%6.2f  Xt=%6.1f Yt=%6.1f  sens_franc=%+.0f conv=%d evit=%d obst=%d\n",
               t * 0.2 + 0.2, in->X_robot, in->Y_robot, in->angle_robot,
               in->X_robot_terrain, in->Y_robot_terrain, d->evit_dernier_sens_franc,
               in->Convergence, d->evitementEnCours, in->obstacleDetecte);
    }
}

// ___________________________________________________________________________
// Etape 0 de l'atelier evitement 2027 : assainissement de la chaine de detection existante.
//
// Contexte commun : match demarre en couleur 1, strategie par defaut. Le robot est au depart
// (X=42, Y=171,5 en repere terrain, cap asserv -PI/2) avec une consigne vers l'avant : pour la
// detection il est en marche avant (le plugin seul ne le deplace pas, SimuBot etant absent).
// Les obstacles sont injectes comme le fait SimuBot : Lidar.ObstacleN.Distance [mm] / Angle [deg].
//
// Le mode "avant" (option --avant) ne joue que les verifications portant sur SM_InputsInterface,
// dont la disposition n'a pas change : il permet de passer la meme suite sur un plugin compile
// AVANT l'etape 0 et de constater les defauts. Les verifications sur SM_DatasInterface (champ de
// bits, evitement, telemetrie) n'ont de sens que sur un plugin a jour.
// ___________________________________________________________________________

static bool g_mode_avant = false;
void scenarios_mode_avant(bool avant) { g_mode_avant = avant; }

static void depart(Banc &banc)
{
    banc.reinitialiser();
    banc.demarrerMatch(0, SM_DatasInterface::EQUIPE_COULEUR_1);
}

void scenarios_etape0(Banc &banc)
{
    SM_InputsInterface *in;
    SM_DatasInterface *d;
    const bool complet = !g_mode_avant;

    // ---------------------------------------------------------------- defaut n°3
    banc.titre("Defaut 3 - un objet DERRIERE le robot ne doit pas compter en marche avant");
    depart(banc);
    banc.obstacle(1, 150, 180);                  // 15 cm derriere, dans le terrain
    banc.passagesModele(15);
    in = banc.entrees();
    banc.verifier(!in->obstacleDetecte_non_filtre, "aucune detection brute d'un objet situe derriere");
    banc.verifier(!in->obstacleDetecte, "pas d'entree en evitement pour un objet situe derriere");
    banc.verifier(!in->obstacle_ARG && !in->obstacle_ARD, "quadrants arriere non leves");

    banc.titre("Defaut 3 - un objet DEVANT le robot compte toujours en marche avant");
    depart(banc);
    banc.obstacle(1, 300, 0);                    // 30 cm droit devant
    banc.passagesModele(6);
    in = banc.entrees(); d = banc.donnees();
    banc.verifier(in->obstacleDetecte, "entree en evitement apres le filtre de confirmation");
    banc.verifier(in->obstacle_AVG, "quadrant avant gauche leve (angle 0)");
    if (complet) {
        banc.verifier(d->evit_detection_obstacle_bitfield == 2, "champ de bits = 2 (AVG)");
    }

    // ---------------------------------------------------------------- defaut n°2
    banc.titre("Defaut 2 - les quadrants de plusieurs points se cumulent");
    depart(banc);
    banc.obstacle(1, 300, 20);                   // avant gauche
    banc.obstacle(2, 300, -20);                  // avant droit
    banc.passagesModele(6);
    in = banc.entrees(); d = banc.donnees();
    banc.verifier(in->obstacle_AVG && in->obstacle_AVD, "AVG et AVD leves ensemble");
    if (complet) {
        banc.verifier(d->evit_detection_obstacle_bitfield == 3, "champ de bits = 3 (AVG + AVD)");
        banc.verifier(d->nombre_obstacles_presents == 2, "deux obstacles comptes");
    }

    // ---------------------------------------------------------------- defaut n°1
    banc.titre("Defaut 1 - la voie liberee est vue immediatement");
    depart(banc);
    banc.obstacle(1, 300, 0);
    banc.passagesModele(6);
    banc.aucunObstacle();
    banc.passagesModele(1);
    in = banc.entrees(); d = banc.donnees();
    banc.verifier(!in->obstacle_AVG && !in->obstacle_AVD && !in->obstacle_ARG && !in->obstacle_ARD,
                  "quadrants retombes des le passage suivant");
    if (complet) {
        banc.verifier(d->evit_detection_obstacle_bitfield == 0, "champ de bits retombe a 0");
    }

    if (complet) {
        banc.titre("Defaut 1 - l'evitement se termine des que la voie est libre");
        depart(banc);
        banc.obstacle(1, 300, 0);
        banc.passagesModele(6);
        d = banc.donnees();
        banc.verifier(d->evitementEnCours, "evitement engage");
        banc.aucunObstacle();                    // l'adversaire s'en va pendant l'arret
        banc.passagesModele(60);                 // 1,2 s : EVITEMENT_INIT dure 1 s
        banc.verifier(!d->evitementEnCours,
                      "evitement termine en moins de 1,2 s (auparavant ~2 s : passage par ATTENDRE)");
    }

    // ---------------------------------------------------------------- defaut n°7
    if (complet) {
        banc.titre("Defaut 7 - filtre de disparition (lidar)");
        depart(banc);
        banc.obstacle(1, 300, 0);
        banc.passagesModele(6);
        banc.aucunObstacle();                    // un scan manque...
        bool toujours_la = true;
        for (int i = 0; i < 5; i++) { banc.passagesModele(1); toujours_la &= banc.entrees()->obstacleDetecte; }
        banc.verifier(toujours_la, "obstacleDetecte maintenu pendant une absence de 100 ms");
        banc.obstacle(1, 300, 0);                // ...puis l'obstacle revient
        banc.passagesModele(1);
        banc.verifier(banc.entrees()->obstacleDetecte, "pas de retombee sur une absence breve");
        banc.aucunObstacle();
        banc.passagesModele(12);
        banc.verifier(!banc.entrees()->obstacleDetecte, "retombee apres 240 ms d'absence (seuil 200 ms)");
    }

    banc.titre("Non-regression - filtre de confirmation a l'apparition inchange");
    depart(banc);
    banc.obstacle(1, 300, 0);
    banc.passagesModele(3);                      // 3 passages : le seuil exige strictement plus
    bool jamais = !banc.entrees()->obstacleDetecte;
    banc.aucunObstacle();
    banc.passagesModele(5);
    jamais &= !banc.entrees()->obstacleDetecte;
    banc.verifier(jamais, "un obstacle vu 3 passages seulement ne declenche pas l'evitement");

    // ---------------------------------------------------------------- defaut n°4
    banc.titre("Defaut 4 - seuils du couloir (constantes, valeurs inchangees)");
    depart(banc);
    banc.obstacle(1, 520, 0);                    // 52 cm : au-dela de SEUIL_DETECTION_LIDAR (50)
    banc.passagesModele(6);
    banc.verifier(!banc.entrees()->obstacleDetecte_non_filtre, "52 cm devant : ignore");
    depart(banc);
    banc.obstacle(1, 480, 0);
    banc.passagesModele(6);
    banc.verifier(banc.entrees()->obstacleDetecte_non_filtre, "48 cm devant : detecte");
    depart(banc);
    banc.obstacle(1, 400, 70);                   // ecart lateral 37,6 cm > 35
    banc.passagesModele(6);
    banc.verifier(!banc.entrees()->obstacleDetecte_non_filtre, "40 cm a 70 deg (37,6 cm lateral) : hors couloir");
    depart(banc);
    banc.obstacle(1, 400, 55);                   // ecart lateral 32,8 cm < 35
    banc.passagesModele(6);
    banc.verifier(banc.entrees()->obstacleDetecte_non_filtre, "40 cm a 55 deg (32,8 cm lateral) : dans le couloir");

    // ---------------------------------------------------------------- defaut n°6
    if (complet) {
        banc.titre("Defaut 6 - telemetrie de l'obstacle le plus proche");
        depart(banc);
        banc.obstacle(1, 400, 10);
        banc.obstacle(2, 250, -15);
        banc.passagesModele(6);
        d = banc.donnees();
        banc.verifier(d->distance_premier_obstacle_detecte == 250, "distance du plus proche = 250 mm");
        banc.verifier(d->angle_premier_obstacle_detecte == -15, "angle du plus proche = -15 deg");
    }

    // ---------------------------------------------------------------- defaut n°5
    banc.titre("Defaut 5 - le lidar sans balayage (externe, simule) alimente la detection");
    depart(banc);
    banc.obstacle(1, 300, 0);
    banc.passagesModele(6);
    banc.verifier(banc.entrees()->obstacleDetecte,
                  "detection a partir de la seule liste d'obstacles (auparavant : jamais)");

    banc.titre("Non-regression - lidar deconnecte : repli sur les capteurs US");
    depart(banc);
    banc.statutLidar(LidarUtils::LIDAR_DISCONNECTED);
    banc.obstacle(1, 300, 0);                    // le lidar « voit » un obstacle, mais il est hors service
    banc.passagesModele(6);
    banc.verifier(!banc.entrees()->obstacleDetecte, "l'obstacle du lidar hors service est ignore");
}
