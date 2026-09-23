#include <cmath>
#include <cstdio>
#include "banc.h"
#include "scenarios.h"
#include "CTacticalEvaluator.h"

// ___________________________________________________________________________
// Etape 3 de l'atelier evitement 2027 : l'evaluation tactique, dans la chaine reelle.
//
// Les proprietes de l'evaluation (criteres, hysteresis, cote libre) sont verifiees a part, sur
// pistes synthetiques : tools/banc_perception. Ici on verifie qu'elle rend le bon verdict au bout
// de la chaine complete -- balayage a 8 Hz, filtre, suivi, projection en repere terrain -- pour
// les situations de reference du document.
//
// Le robot part en (42 ; 171,5) cap terrain -PI/2 : il regarde vers les Y decroissants. « Devant
// lui a 70 cm » se lit donc (42 ; 101,5).
// ___________________________________________________________________________

static const char *nom_niveau(unsigned char n)
{
    switch (n) {
    case MENACE_LIBRE :    return "LIBRE";
    case MENACE_PRUDENCE : return "PRUDENCE";
    case MENACE_RALENTI :  return "RALENTI";
    case MENACE_ARRET :    return "ARRET";
    }
    return "?";
}

static void depart(Banc &banc)
{
    banc.reinitialiser();
    banc.demarrerMatch(0, SM_DatasInterface::EQUIPE_COULEUR_1);
}

//! Adversaire immobile devant le robot, a la distance voulue ; rend le verdict etabli
static SM_DatasInterface *adversaire_immobile(Banc &banc, float distance_cm, float decalage_lateral_cm = 0.f)
{
    depart(banc);
    banc.adversaireEnPositionTerrain(42.f + decalage_lateral_cm, 171.5f - distance_cm);
    banc.simuler(70);                              // 1,4 s : une dizaine de tours de balayage
    return banc.donnees();
}

void scenarios_etape3(Banc &banc)
{
    SM_DatasInterface *d;

    banc.titre("L'echelle des distances, adversaire immobile droit devant");
    d = adversaire_immobile(banc, 70.f);
    printf("     a 70 cm : %-8s D=%.0f cm phi=%.2f rad cote=%+d\n",
           nom_niveau(d->evit_menace), d->evit_D_cm, d->evit_phi_rad, d->evit_cote_libre);
    banc.verifier(d->evit_menace == MENACE_PRUDENCE, "70 cm : prudence");
    banc.verifier(fabsf(d->evit_D_cm - 70.f) < 6.f, "distance rendue coherente");

    d = adversaire_immobile(banc, 40.f);
    printf("     a 40 cm : %-8s D=%.0f cm\n", nom_niveau(d->evit_menace), d->evit_D_cm);
    banc.verifier(d->evit_menace == MENACE_RALENTI, "40 cm : ralenti");

    d = adversaire_immobile(banc, 25.f);
    printf("     a 25 cm : %-8s D=%.0f cm\n", nom_niveau(d->evit_menace), d->evit_D_cm);
    banc.verifier(d->evit_menace == MENACE_ARRET, "25 cm : arret");

    banc.titre("Hors du couloir : aucune menace");
    d = adversaire_immobile(banc, 40.f, 60.f);     // 40 cm devant, 60 cm sur le cote
    printf("     de cote : %-8s\n", nom_niveau(d->evit_menace));
    banc.verifier(d->evit_menace == MENACE_LIBRE, "objet lateral immobile : aucune menace");

    banc.titre("Un adversaire qui vient sur nous");
    depart(banc);
    for (int n = 0; n < 70; n++) {
        // part a 1,2 m devant et remonte vers nous a 60 cm/s
        banc.adversaireEnPositionTerrain(42.f, 171.5f - 120.f + 60.f * n * 0.02f);
        banc.simuler(1);
    }
    d = banc.donnees();
    printf("     verdict : %-8s D=%.0f cm ttc=%.2f s dmin=%.0f cm  (piste V=%.0f cm/s statique=%d)\n",
           nom_niveau(d->evit_menace), d->evit_D_cm, d->evit_ttc_s, d->evit_dmin_cm,
           d->evit_piste_proche_V_cms, d->evit_piste_proche_statique);
    banc.verifier(d->evit_menace == MENACE_ARRET, "il fonce : arret");
    banc.verifier(!d->evit_piste_proche_statique, "la piste est vue mobile");

    banc.titre("Un adversaire qui s'eloigne");
    depart(banc);
    for (int n = 0; n < 70; n++) {
        // part a 50 cm devant et s'en va a 60 cm/s
        banc.adversaireEnPositionTerrain(42.f, 171.5f - 50.f - 60.f * n * 0.02f);
        banc.simuler(1);
    }
    d = banc.donnees();
    printf("     verdict : %-8s D=%.0f cm ttc=%.2f s\n",
           nom_niveau(d->evit_menace), d->evit_D_cm, d->evit_ttc_s);
    banc.verifier(d->evit_menace == MENACE_LIBRE, "il s'eloigne : aucune menace");

    banc.titre("Perte du lidar : le verdict ne reste pas perime");
    d = adversaire_immobile(banc, 25.f);
    banc.verifier(d->evit_menace == MENACE_ARRET, "menace etablie avant la panne");
    banc.statutLidar(LidarUtils::LIDAR_DISCONNECTED);
    banc.simuler(10);
    d = banc.donnees();
    banc.verifier(d->evit_menace == MENACE_LIBRE, "lidar hors service : verdict remis a plat");
    banc.verifier(d->evit_nb_pistes == 0, "pistes oubliees");
}
