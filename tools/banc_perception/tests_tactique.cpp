// Couche 3 : evaluation tactique (CTacticalEvaluator).
//
// Le banc construit des pistes en alimentant le suivi comme le ferait le filtre, puis demande son
// verdict a l'evaluation. Les quatre situations de reference du document d'architecture sont
// jouees : l'adversaire s'eloigne, il croise devant, il fonce, il est arrete en travers.
#include <cmath>
#include <cstdio>
#include "outils_banc.h"
#include "CObstacleTracker.h"
#include "CTacticalEvaluator.h"

static const float PI_F = 3.14159265f;
static const unsigned long PERIODE_SCAN_MS = 125;

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

//! Observation d'un objet du terrain par un robot pose en (xr, yr, cap)
static void observer(CLidarBlobs &objets, float xr, float yr, float cap, float xo, float yo,
                     bool douteux = false)
{
    const float dx = xo - xr, dy = yo - yr;
    float angle = atan2f(dy, dx) - cap;
    while (angle > PI_F)  angle -= 2.f*PI_F;
    while (angle < -PI_F) angle += 2.f*PI_F;
    objets.append(angle * 180.f / PI_F, sqrtf(dx*dx + dy*dy) * 10.f, 12.f, douteux);
}

/*! Joue une situation complete et rend le verdict.
    \param x_obj, y_obj position initiale de l'adversaire
    \param vx_obj, vy_obj sa vitesse [cm/s]
    \param x_rob, y_rob notre position (fixe pendant la sequence : le robot avance "en pensee",
           c'est sa vitesse qui est donnee a l'evaluation)
    \param cap notre cap de trajectoire
    \param vitesse notre vitesse d'avance
*/
static tMenace jouer(CTacticalEvaluator &tactique, float x_obj, float y_obj, float vx_obj, float vy_obj,
                     float x_rob, float y_rob, float cap, float vitesse, bool douteux = false,
                     int tours = 10)
{
    CObstacleTracker suivi;
    tactique.init();
    CLidarBlobs objets;
    unsigned long date = 1000;
    for (int tour = 0; tour < tours; tour++) {
        const float t = tour * (PERIODE_SCAN_MS / 1000.f);
        objets.clear();
        observer(objets, x_rob, y_rob, cap, x_obj + vx_obj*t, y_obj + vy_obj*t, douteux);
        suivi.nouveauScan(objets, x_rob, y_rob, cap, date);
        tactique.evaluer(suivi, x_rob, y_rob, cap, vitesse);
        date += PERIODE_SCAN_MS;
    }
    return tactique.menace();
}

void tests_tactique()
{
    CTacticalEvaluator tactique;
    tMenace m;
    char libelle[200];

    // Notre robot : en (100, 100), cap 0 (vers les X croissants), 40 cm/s.
    const float XR = 100.f, YR = 100.f, CAP = 0.f, V = 40.f;

    // ----------------------------------------------------------------
    titre("Les quatre situations de reference du document");

    m = jouer(tactique, 160.f, 100.f, 80.f, 0.f, XR, YR, CAP, V);   // devant, mais plus rapide que nous
    printf("     il s'eloigne          : %-8s D=%.0f cm ttc=%.2f s dmin=%.0f cm\n",
           nom_niveau(m.niveau), m.D_cm, m.ttc_s, m.dmin_cm);
    verifier(m.niveau == MENACE_LIBRE, "il s'eloigne : aucune menace");

    m = jouer(tactique, 160.f, 100.f, 0.f, 60.f, XR, YR, CAP, V);   // traverse devant nous
    printf("     il croise devant      : %-8s D=%.0f cm ttc=%.2f s dmin=%.0f cm\n",
           nom_niveau(m.niveau), m.D_cm, m.ttc_s, m.dmin_cm);
    verifier(m.niveau == MENACE_LIBRE, "il croise en degageant : aucune menace");

    m = jouer(tactique, 200.f, 100.f, -60.f, 0.f, XR, YR, CAP, V);  // vient droit sur nous
    printf("     il vient sur nous     : %-8s D=%.0f cm ttc=%.2f s dmin=%.0f cm\n",
           nom_niveau(m.niveau), m.D_cm, m.ttc_s, m.dmin_cm);
    verifier(m.niveau == MENACE_ARRET, "il fonce : arret");
    verifier(m.ttc_s >= 0.f && m.ttc_s < 2.f, "temps avant contact coherent");

    m = jouer(tactique, 140.f, 100.f, 0.f, 0.f, XR, YR, CAP, V);    // arrete en travers a 40 cm
    printf("     arrete en travers     : %-8s D=%.0f cm statique=%d\n",
           nom_niveau(m.niveau), m.D_cm, m.statique);
    verifier(m.niveau == MENACE_RALENTI, "arrete a 40 cm dans le couloir : ralenti");
    verifier(m.statique, "la piste est vue comme immobile");

    // ----------------------------------------------------------------
    titre("L'echelle des distances pour un obstacle immobile");
    struct { float x; unsigned char attendu; } echelle[] = {
        { 190.f, MENACE_PRUDENCE },   // 90 cm : au-dela du seuil de prudence (80)
        { 170.f, MENACE_PRUDENCE },   // 70 cm
        { 140.f, MENACE_RALENTI  },   // 40 cm
        { 125.f, MENACE_ARRET    },   // 25 cm
    };
    for (int i = 0; i < 4; i++) {
        m = jouer(tactique, echelle[i].x, 100.f, 0.f, 0.f, XR, YR, CAP, V);
        snprintf(libelle, sizeof(libelle), "obstacle immobile a %.0f cm : %s",
                 echelle[i].x - XR, nom_niveau(m.niveau));
        if (i == 0) verifier(m.niveau == MENACE_PRUDENCE || m.niveau == MENACE_LIBRE, libelle);
        else verifier(m.niveau == echelle[i].attendu, libelle);
    }

    // ----------------------------------------------------------------
    titre("Hors du couloir et hors du terrain");
    m = jouer(tactique, 140.f, 160.f, 0.f, 0.f, XR, YR, CAP, V);    // 40 cm devant, 60 cm sur le cote
    printf("     de cote               : %-8s\n", nom_niveau(m.niveau));
    verifier(m.niveau == MENACE_LIBRE, "objet immobile hors du couloir : aucune menace");

    m = jouer(tactique, 140.f, 205.f, 0.f, 0.f, XR, YR, PI_F/2.f, V); // au-dela de la bordure Y
    verifier(m.niveau == MENACE_LIBRE, "objet hors terrain : ignore");

    // ----------------------------------------------------------------
    titre("Un objet douteux menace comme les autres (cas de l'homologation)");
    m = jouer(tactique, 125.f, 100.f, 0.f, 0.f, XR, YR, CAP, V, true);
    printf("     objet douteux a 25 cm : %-8s douteux=%d\n", nom_niveau(m.niveau), m.forme_douteuse);
    verifier(m.niveau == MENACE_ARRET, "objet douteux et proche : arret");
    verifier(m.forme_douteuse, "le doute est remonte avec le verdict");

    // ----------------------------------------------------------------
    titre("Hysteresis : le niveau ne vibre pas autour d'un seuil");
    {
        CObstacleTracker suivi;
        tactique.init();
        CLidarBlobs objets;
        unsigned long date = 1000;
        int changements = 0;
        unsigned char precedent = MENACE_LIBRE;
        for (int tour = 0; tour < 40; tour++) {
            // obstacle immobile qui oscille de +/- 2 cm autour du seuil RALENTI (50 cm)
            const float x = 150.f + ((tour % 2) ? 2.f : -2.f);
            objets.clear();
            observer(objets, XR, YR, CAP, x, 100.f);
            suivi.nouveauScan(objets, XR, YR, CAP, date);
            tactique.evaluer(suivi, XR, YR, CAP, V);
            if (tour > 6) {              // apres l'etablissement de la piste
                if (tactique.menace().niveau != precedent) changements++;
            }
            precedent = tactique.menace().niveau;
            date += PERIODE_SCAN_MS;
        }
        printf("     changements de niveau sur 33 tours : %d\n", changements);
        verifier(changements <= 1, "un seul changement de niveau malgre l'oscillation");
    }

    // ----------------------------------------------------------------
    titre("Cote ou s'ecarter");
    m = jouer(tactique, 140.f, 110.f, 0.f, 0.f, XR, YR, CAP, V);    // objet legerement a gauche
    printf("     objet a gauche        : cote libre = %+d\n", m.cote_libre);
    verifier(m.cote_libre == -1, "objet a gauche : on s'ecarte a droite");
    m = jouer(tactique, 140.f, 90.f, 0.f, 0.f, XR, YR, CAP, V);     // objet legerement a droite
    verifier(m.cote_libre == +1, "objet a droite : on s'ecarte a gauche");
    m = jouer(tactique, 140.f, 20.f, 0.f, 0.f, 100.f, 25.f, CAP, V); // nous sommes contre la bordure Y basse
    printf("     colle a la bordure    : cote libre = %+d\n", m.cote_libre);
    verifier(m.cote_libre == +1, "contre une bordure : on s'ecarte du cote qui degage");
}
