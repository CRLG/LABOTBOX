#include <cmath>
#include <cstdio>
#include "banc.h"
#include "scenarios.h"
#include "CTacticalEvaluator.h"
#include "ConfigSpecifiqueCoupe.h"

// ___________________________________________________________________________
// Etape 4 de l'atelier evitement 2027 : la strategie d'evitement AE, echelle de phases reentrante.
//
// Ce qui est verifie ici -- et qui ne peut l'etre nulle part ailleurs -- c'est le COMPORTEMENT au
// bout de la chaine complete : l'echelle monte d'une marche par entree dans l'evitement, la mission
// reprend la main entre deux marches, la voie qui se degage remet tout a plat, et les niveaux bas
// ne declenchent aucune manoeuvre.
//
// La strategie AE n'est portee que par les strategies d'HOMOLOGATION (1 et 2) ; la strategie 0
// (PAR_DEFAUT) garde le comportement historique. Les suites des etapes 0 a 3 jouent toutes en
// strategie 0 : elles verifient donc, par construction, la non-regression de l'existant.
//
// Le robot part en (42 ; 171,5) cap terrain -PI/2 : il regarde vers les Y decroissants.
// ___________________________________________________________________________

static const char *nom_marche(unsigned char e)
{
    switch (e) {
    case SM_DatasInterface::ETAT_AE_LIBRE :     return "LIBRE";
    case SM_DatasInterface::ETAT_AE_PRUDENCE :  return "PRUDENCE";
    case SM_DatasInterface::ETAT_AE_RALENTI :   return "RALENTI";
    case SM_DatasInterface::ETAT_AE_ARRET :     return "ARRET";
    case SM_DatasInterface::ETAT_AE_GENTLEMAN : return "GENTLEMAN";
    case SM_DatasInterface::ETAT_AE_ESQUIVE :   return "ESQUIVE";
    case SM_DatasInterface::ETAT_AE_BLOCAGE :   return "BLOCAGE";
    }
    return "?";
}

//! Depart en strategie d'homologation : c'est celle qui porte AE
static void depart_ae(Banc &banc)
{
    banc.reinitialiser();
    banc.demarrerMatch(STRATEGIE_HOMOLO1, SM_DatasInterface::EQUIPE_COULEUR_1);
}

//! Joue des passages jusqu'a ce que la marche de l'echelle atteigne au moins "marche"
//! \return le nombre de passages consommes, ou -1 si la marche n'est pas atteinte
static int attendreMarche(Banc &banc, unsigned char marche, int passages_max = 400)
{
    for (int i = 0; i < passages_max; i++) {
        banc.passagesModele(1);
        if (banc.donnees()->evit_ae_state >= marche) return i;
    }
    return -1;
}

void scenarios_etape4(Banc &banc)
{
    SM_DatasInterface *d = banc.donnees();

    // ___________________________________________________________________
    banc.titre("Les niveaux bas ne declenchent aucune manoeuvre");
    depart_ae(banc);
    banc.adversaireEnPositionTerrain(42.f, 171.5f - 70.f);     // 70 cm droit devant : PRUDENCE
    banc.simuler(70);
    printf("     a 70 cm : marche=%-9s evitement=%d\n", nom_marche(d->evit_ae_state), (int)d->evitementEnCours);
    banc.verifier(d->evit_menace == MENACE_PRUDENCE, "70 cm : menace PRUDENCE");
    banc.verifier(d->evit_ae_state == SM_DatasInterface::ETAT_AE_PRUDENCE, "marche PRUDENCE posee par IA");
    banc.verifier(!d->evitementEnCours, "aucune entree en evitement : la mission continue");

    banc.adversaireEnPositionTerrain(42.f, 171.5f - 40.f);     // 40 cm : RALENTI
    banc.simuler(70);
    banc.verifier(d->evit_menace == MENACE_RALENTI, "40 cm : menace RALENTI");
    banc.verifier(d->evit_ae_state == SM_DatasInterface::ETAT_AE_RALENTI, "marche RALENTI");
    banc.verifier(!d->evitementEnCours, "toujours aucune manoeuvre a 40 cm");

    // ___________________________________________________________________
    banc.titre("L'echelle monte d'une marche par entree dans l'evitement");
    depart_ae(banc);
    banc.adversaireEnPositionTerrain(42.f, 171.5f - 25.f);     // 25 cm droit devant : ARRET
    int p = attendreMarche(banc, SM_DatasInterface::ETAT_AE_ARRET);
    printf("     arret atteint en %d passages (marche=%s)\n", p, nom_marche(d->evit_ae_state));
    banc.verifier(p >= 0, "l'arret est atteint");
    banc.verifier(d->evitementEnCours, "l'evitement est bien en cours pendant l'arret");

    // La marche suivante ne peut etre atteinte que par une NOUVELLE entree dans l'evitement :
    // c'est tout le principe de la reentrance. On verifie donc que l'evitement se termine avant.
    bool evitement_relache = false;
    for (int i = 0; i < 400; i++) {
        banc.passagesModele(1);
        if (!d->evitementEnCours) evitement_relache = true;
        if (d->evit_ae_state >= SM_DatasInterface::ETAT_AE_GENTLEMAN) break;
    }
    printf("     marche suivante=%s (evitement relache entre-temps : %d)\n",
           nom_marche(d->evit_ae_state), (int)evitement_relache);
    banc.verifier(evitement_relache, "la mission reprend la main entre deux marches");
    banc.verifier(d->evit_ae_state >= SM_DatasInterface::ETAT_AE_GENTLEMAN, "l'echelle monte au gentleman move");

    int p_esquive = attendreMarche(banc, SM_DatasInterface::ETAT_AE_ESQUIVE);
    printf("     esquive atteinte en %d passages\n", p_esquive);
    banc.verifier(p_esquive >= 0, "l'echelle monte jusqu'a l'esquive");
    int p_blocage = attendreMarche(banc, SM_DatasInterface::ETAT_AE_BLOCAGE);
    banc.verifier(p_blocage >= 0, "puis jusqu'au blocage, l'adversaire ne bougeant pas");

    // ___________________________________________________________________
    banc.titre("Le blocage n'est pas definitif : l'echelle redescend pour retenter");
    // Au bout de TIMEOUT_AE_BLOCAGE_MS passes en blocage, on repart du bas des manoeuvres.
    bool redescendue = false;
    for (int i = 0; i < 800; i++) {
        banc.passagesModele(1);
        if (d->evit_ae_state == SM_DatasInterface::ETAT_AE_ARRET) { redescendue = true; break; }
    }
    banc.verifier(redescendue, "le blocage finit par relancer les manoeuvres");

    // ___________________________________________________________________
    banc.titre("La voie qui se degage remet l'echelle a plat");
    banc.retirerAdversaire();
    banc.simuler(80);
    printf("     adversaire retire : marche=%s menace=%d\n", nom_marche(d->evit_ae_state), (int)d->evit_menace);
    banc.verifier(d->evit_menace == MENACE_LIBRE, "plus de menace");
    banc.verifier(d->evit_ae_state == SM_DatasInterface::ETAT_AE_LIBRE, "echelle remise a zero");
    banc.verifier(d->evit_ae_chrono_blocage_ms == 0, "temps de blocage oublie");

    // Un nouvel adversaire repart donc du bas de l'echelle, et non du blocage precedent
    banc.adversaireEnPositionTerrain(42.f, 171.5f - 25.f);
    p = attendreMarche(banc, SM_DatasInterface::ETAT_AE_ARRET);
    banc.verifier(p >= 0, "le prochain adversaire est negocie depuis le debut");
    banc.verifier(d->evit_ae_state == SM_DatasInterface::ETAT_AE_ARRET, "premiere marche a nouveau : l'arret");

    // ___________________________________________________________________
    banc.titre("Geometrie des manoeuvres : les vetos de bordure");
    // Au milieu du terrain, les deux manoeuvres sont ouvertes. Attention : la zone de DEPART n'est
    // pas un "plein terrain" -- elle est adossee a une bordure, et le recul y est refuse a juste
    // titre (verifie plus bas).
    depart_ae(banc);
    banc.placerRobotTerrain(150.f, 100.f, -(float)M_PI/2.f);
    banc.adversaireEnPositionTerrain(150.f, 70.f);
    banc.simuler(70);
    printf("     en plein terrain : recul=%d esquive=%d cote=%+d cap_esquive=%.2f rad\n",
           (int)d->evit_recul_possible, (int)d->evit_esquive_possible,
           d->evit_cote_libre, d->evit_esquive_cap_rad);
    banc.verifier(d->evit_recul_possible, "loin des bords : le recul est possible");
    banc.verifier(d->evit_esquive_possible, "et l'esquive aussi");
    banc.verifier(d->evit_cote_libre != 0, "un cote libre a ete choisi");
    // Le cap d'esquive doit etre decale du cote libre, et d'assez pour sortir du couloir
    const float ecart_cap = d->evit_esquive_cap_rad - banc.entrees()->angle_robot;
    printf("     ecart de cap = %+.2f rad pour un cote %+d\n", ecart_cap, d->evit_cote_libre);
    banc.verifier((ecart_cap * (float)d->evit_cote_libre) > 0.f, "le cap d'esquive part du cote libre");
    banc.verifier(fabsf(ecart_cap) > fabsf(d->evit_phi_rad), "et depasse l'angle de l'adversaire");
    banc.verifier(fabsf(ecart_cap) <= (ECART_AE_ESQUIVE_MAX_RAD + 0.01f),
                  "l'esquive reste une esquive, pas un demi-tour");

    // Le robot part adosse a la bordure du fond : y reculer est refuse
    depart_ae(banc);
    banc.adversaireEnPositionTerrain(42.f, 171.5f - 30.f);
    banc.simuler(70);
    printf("     en zone de depart : recul=%d\n", (int)d->evit_recul_possible);
    banc.verifier(!d->evit_recul_possible, "zone de depart : le recul vers le fond est refuse");

    // Robot adosse a la bordure basse : reculer l'y enfoncerait -> veto
    banc.reinitialiser();
    banc.demarrerMatch(STRATEGIE_HOMOLO1, SM_DatasInterface::EQUIPE_COULEUR_1);
    banc.placerRobotTerrain(42.f, 15.f, -(float)M_PI/2.f);   // cap vers Y decroissants, dos au bord
    banc.adversaireEnPositionTerrain(42.f, 15.f - 25.f);
    banc.simuler(70);
    printf("     adosse au bord : recul=%d\n", (int)d->evit_recul_possible);
    banc.verifier(!d->evit_recul_possible, "accule a la bordure : le recul est refuse");

    // ___________________________________________________________________
    banc.titre("L'alea porte sur le temps, et reste borne");
    depart_ae(banc);
    banc.adversaireEnPositionTerrain(42.f, 171.5f - 25.f);
    unsigned long tempo_min = 0xFFFFFFFFUL, tempo_max = 0;
    for (int i = 0; i < 40; i++) {
        banc.passagesModele(1);
        const unsigned long t = d->evit_ae_tempo_arret_ms;
        if (t < tempo_min) tempo_min = t;
        if (t > tempo_max) tempo_max = t;
    }
    printf("     temporisation d'arret : de %lu a %lu ms (attendu dans [%d ; %d])\n",
           tempo_min, tempo_max, TIMEOUT_AE_ARRET_MS - ALEA_AE_ARRET_MS, TIMEOUT_AE_ARRET_MS + ALEA_AE_ARRET_MS);
    banc.verifier(tempo_min >= (unsigned long)(TIMEOUT_AE_ARRET_MS - ALEA_AE_ARRET_MS), "temporisation jamais trop courte");
    banc.verifier(tempo_max <= (unsigned long)(TIMEOUT_AE_ARRET_MS + ALEA_AE_ARRET_MS), "temporisation jamais trop longue");
    banc.verifier(tempo_max > tempo_min, "la temporisation varie bien d'un arret a l'autre");

    // ___________________________________________________________________
    banc.titre("Perte du lidar : retour au comportement historique");
    banc.statutLidar(LidarUtils::LIDAR_ERROR);
    banc.simuler(20);
    printf("     lidar perdu : marche=%s menace=%d\n", nom_marche(d->evit_ae_state), (int)d->evit_menace);
    banc.verifier(d->evit_ae_state == SM_DatasInterface::ETAT_AE_LIBRE, "echelle remise a plat");
    banc.verifier(d->evit_menace == MENACE_LIBRE, "verdict tactique remis a plat");
    banc.statutLidar(LidarUtils::LIDAR_OK);

    // ___________________________________________________________________
    banc.titre("Non-regression : la strategie par defaut ignore AE");
    banc.reinitialiser();
    banc.demarrerMatch(STRATEGIE_PAR_DEFAUT, SM_DatasInterface::EQUIPE_COULEUR_1);
    banc.adversaireEnPositionTerrain(42.f, 171.5f - 25.f);
    banc.simuler(70);
    printf("     strategie par defaut : choix=%d marche=%s\n",
           (int)d->evit_choix_strategie, nom_marche(d->evit_ae_state));
    banc.verifier(d->evit_choix_strategie == SM_DatasInterface::STRATEGIE_EVITEMENT_ATTENDRE,
                  "la strategie par defaut reste ATTENDRE");
    banc.verifier(d->evit_ae_state == SM_DatasInterface::ETAT_AE_LIBRE,
                  "l'echelle AE n'est pas alimentee");
}
