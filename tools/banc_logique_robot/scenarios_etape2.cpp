#include <cmath>
#include <cstdio>
#include "banc.h"
#include "scenarios.h"

// ___________________________________________________________________________
// Etape 2 de l'atelier evitement 2027 : le suivi temporel, dans la chaine reelle.
//
// Les proprietes du suivi lui-meme (association, vitesse, hysteresis, extrapolation) sont
// verifiees a part, sur donnees synthetiques : tools/banc_perception. Ici on verifie qu'il tient
// dans la chaine complete -- balayage synthetise a 8 Hz, filtre, projection en repere terrain avec
// la VRAIE pose du robot -- et surtout qu'il distingue ce qui bouge de ce qui ne bouge pas quand
// c'est NOTRE robot qui se deplace.
//
// Le robot est deplace en injectant des pas codeurs, comme le fait SimuBot.
// ___________________________________________________________________________

static void depart(Banc &banc)
{
    banc.reinitialiser();
    banc.demarrerMatch(0, SM_DatasInterface::EQUIPE_COULEUR_1);
}

void scenarios_etape2(Banc &banc)
{
    banc.titre("Le banc sait deplacer le robot (prealable a tout le reste)");
    depart(banc);
    const float x0 = banc.entrees()->X_robot_terrain;
    const float y0 = banc.entrees()->Y_robot_terrain;
    banc.simuler(50, 40.f, 0.f);                       // 1 s a 40 cm/s
    const float parcouru = sqrtf((banc.entrees()->X_robot_terrain - x0)*(banc.entrees()->X_robot_terrain - x0)
                               + (banc.entrees()->Y_robot_terrain - y0)*(banc.entrees()->Y_robot_terrain - y0));
    printf("     distance parcourue en 1 s : %.1f cm (40 attendus)\n", parcouru);
    banc.verifier(fabsf(parcouru - 40.f) < 4.f, "le robot avance de 40 cm en une seconde");

    banc.titre("Adversaire immobile, robot immobile : une piste statique");
    depart(banc);
    banc.adversaireEnPositionTerrain(42.f, 71.5f);     // 1 m devant le robot au depart
    banc.simuler(60);                                  // 1,2 s : une dizaine de tours de balayage
    SM_DatasInterface *d = banc.donnees();
    printf("     pistes=%d  piste proche : X=%.1f Y=%.1f V=%.1f cm/s statique=%d  age scan=%d ms\n",
           d->evit_nb_pistes, d->evit_piste_proche_X_cm, d->evit_piste_proche_Y_cm,
           d->evit_piste_proche_V_cms, d->evit_piste_proche_statique, d->evit_age_scan_ms);
    banc.verifier(d->evit_nb_pistes == 1, "une seule piste suivie");
    banc.verifier(fabsf(d->evit_piste_proche_X_cm - 42.f) < 6.f
               && fabsf(d->evit_piste_proche_Y_cm - 71.5f) < 6.f,
                  "position de la piste retrouvee dans le repere terrain");
    banc.verifier(d->evit_piste_proche_V_cms < 10.f, "vitesse sous le seuil statique");
    banc.verifier(d->evit_piste_proche_statique, "piste declaree statique");
    banc.verifier(d->evit_age_scan_ms <= 140, "le balayage a moins de 140 ms (lidar a 8 Hz)");

    banc.titre("Adversaire immobile, NOTRE robot avance vers lui");
    depart(banc);
    banc.adversaireEnPositionTerrain(42.f, 71.5f);
    banc.simuler(60, 40.f, 0.f);                       // 1,2 s a 40 cm/s : on se rapproche de 48 cm
    d = banc.donnees();
    printf("     piste proche : X=%.1f Y=%.1f V=%.1f cm/s statique=%d\n",
           d->evit_piste_proche_X_cm, d->evit_piste_proche_Y_cm,
           d->evit_piste_proche_V_cms, d->evit_piste_proche_statique);
    banc.verifier(fabsf(d->evit_piste_proche_Y_cm - 71.5f) < 8.f,
                  "la piste reste a sa place : c'est nous qui avancons");
    banc.verifier(d->evit_piste_proche_V_cms < 10.f,
                  "vitesse sous le seuil statique malgre notre deplacement");
    banc.verifier(d->evit_piste_proche_statique, "piste toujours declaree statique");

    banc.titre("Adversaire immobile, NOTRE robot tourne sur lui-meme");
    depart(banc);
    banc.adversaireEnPositionTerrain(42.f, 71.5f);
    banc.simuler(60, 0.f, 1.0f);                       // 1,2 s a 1 rad/s
    d = banc.donnees();
    printf("     piste proche : X=%.1f Y=%.1f V=%.1f cm/s statique=%d\n",
           d->evit_piste_proche_X_cm, d->evit_piste_proche_Y_cm,
           d->evit_piste_proche_V_cms, d->evit_piste_proche_statique);
    banc.verifier(d->evit_nb_pistes >= 1, "la piste survit a la rotation");
    banc.verifier(d->evit_piste_proche_V_cms < 10.f, "vitesse sous le seuil statique");
    banc.verifier(d->evit_piste_proche_statique, "piste toujours declaree statique");

    banc.titre("Adversaire qui traverse a 40 cm/s, notre robot immobile");
    depart(banc);
    const float y_adv = 71.5f;
    for (int n = 0; n < 80; n++) {
        banc.adversaireEnPositionTerrain(42.f + 40.f * n * 0.02f, y_adv);
        banc.simuler(1);
    }
    d = banc.donnees();
    printf("     piste proche : X=%.1f V=%.1f cm/s statique=%d\n",
           d->evit_piste_proche_X_cm, d->evit_piste_proche_V_cms, d->evit_piste_proche_statique);
    banc.verifier(fabsf(d->evit_piste_proche_V_cms - 40.f) < 10.f,
                  "vitesse rendue a 40 cm/s pres de 10 cm/s");
    banc.verifier(!d->evit_piste_proche_statique, "piste declaree mobile");

    banc.titre("L'adversaire disparait : la piste survit un instant, puis est perdue");
    depart(banc);
    banc.adversaireEnPositionTerrain(42.f, 71.5f);
    banc.simuler(60);
    banc.retirerAdversaire();
    banc.simuler(20);                                  // 400 ms : environ 3 tours de balayage
    banc.verifier(banc.donnees()->evit_nb_pistes == 1, "piste conservee apres 3 tours sans mesure");
    banc.simuler(40);                                  // 800 ms de plus
    banc.verifier(banc.donnees()->evit_nb_pistes == 0, "piste perdue au-dela du seuil");
}
