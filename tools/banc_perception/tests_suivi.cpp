// Couche 2 : suivi temporel des objets (CObstacleTracker).
//
// Le banc joue des sequences de tours de balayage, en donnant a chaque tour ce que verrait le
// filtre (un objet vu du robot : angle et distance) et la pose du robot. Il verifie que le suivi
// restitue, dans le repere du TERRAIN, une position stable pour un objet fixe -- y compris quand
// c'est le robot qui bouge -- et une vitesse juste pour un objet mobile.
#include <cmath>
#include <cstdio>
#include "outils_banc.h"
#include "CObstacleTracker.h"

static const float PI_F = 3.14159265f;
static const unsigned long PERIODE_SCAN_MS = 125;      // lidar a 8 Hz

//! Fabrique ce que le filtre rendrait : l'objet (xo, yo) du terrain vu par un robot pose en
//! (xr, yr, cap). Largeur arbitraire mais coherente d'un tour a l'autre.
static void observer(CLidarBlobs &objets, float xr, float yr, float cap,
                     float xo, float yo, bool douteux = false)
{
    const float dx = xo - xr;
    const float dy = yo - yr;
    const float distance_cm = sqrtf(dx*dx + dy*dy);
    float angle_rad = atan2f(dy, dx) - cap;
    while (angle_rad > PI_F)  angle_rad -= 2.f*PI_F;
    while (angle_rad < -PI_F) angle_rad += 2.f*PI_F;
    objets.append(angle_rad * 180.f / PI_F, distance_cm * 10.f, 12.f, douteux);
}

static float vitesse(const tObstacleTrack *p)
{
    return sqrtf(p->Vx_cms*p->Vx_cms + p->Vy_cms*p->Vy_cms);
}

void tests_suivi()
{
    CObstacleTracker suivi;
    CLidarBlobs objets;
    unsigned long date = 1000;

    // ----------------------------------------------------------------
    titre("Un objet immobile, robot immobile : une piste stable, declaree statique");
    suivi.init();
    date = 1000;
    for (int tour = 0; tour < 8; tour++) {
        objets.clear();
        observer(objets, 100.f, 100.f, 0.f, 150.f, 100.f);
        suivi.nouveauScan(objets, 100.f, 100.f, 0.f, date);
        date += PERIODE_SCAN_MS;
    }
    verifier(suivi.count() == 1, "une seule piste");
    const tObstacleTrack *p = suivi.plusProche(100.f, 100.f);
    if (p) {
        printf("     piste : X=%.1f Y=%.1f V=%.1f cm/s statique=%d age=%d ms\n",
               p->X_cm, p->Y_cm, vitesse(p), p->statique, p->age_ms);
        verifier(fabsf(p->X_cm - 150.f) < 2.f && fabsf(p->Y_cm - 100.f) < 2.f,
                 "position rendue dans le repere terrain");
        verifier(vitesse(p) < 10.f, "vitesse sous le seuil statique");
        verifier(p->statique, "piste declaree statique apres 5 tours");
        verifier(p->age_ms >= 800 && p->age_ms <= 900, "age coherent (8 tours de 125 ms)");
    }

    // ----------------------------------------------------------------
    titre("Le meme objet, mais c'est NOTRE robot qui tourne sur lui-meme");
    suivi.init();
    date = 1000;
    for (int tour = 0; tour < 10; tour++) {
        const float cap = tour * 0.25f;                 // le robot pivote de 0,25 rad par tour
        objets.clear();
        observer(objets, 100.f, 100.f, cap, 150.f, 100.f);
        suivi.nouveauScan(objets, 100.f, 100.f, cap, date);
        date += PERIODE_SCAN_MS;
    }
    p = suivi.plusProche(100.f, 100.f);
    verifier(suivi.count() == 1, "toujours une seule piste malgre la rotation");
    if (p) {
        printf("     piste : X=%.1f Y=%.1f V=%.1f cm/s statique=%d\n",
               p->X_cm, p->Y_cm, vitesse(p), p->statique);
        verifier(fabsf(p->X_cm - 150.f) < 2.f && fabsf(p->Y_cm - 100.f) < 2.f,
                 "position inchangee : l'objet ne bouge pas, c'est nous qui tournons");
        verifier(vitesse(p) < 10.f, "vitesse toujours sous le seuil statique");
        verifier(p->statique, "piste toujours declaree statique");
    }

    // ----------------------------------------------------------------
    titre("Le meme objet, notre robot avance a 40 cm/s");
    suivi.init();
    date = 1000;
    for (int tour = 0; tour < 10; tour++) {
        const float xr = 100.f + 40.f * tour * (PERIODE_SCAN_MS / 1000.f);
        objets.clear();
        observer(objets, xr, 100.f, 0.f, 250.f, 100.f);
        suivi.nouveauScan(objets, xr, 100.f, 0.f, date);
        date += PERIODE_SCAN_MS;
    }
    p = suivi.plusProche(0.f, 0.f);
    if (p) {
        printf("     piste : X=%.1f (250 attendus) V=%.1f cm/s statique=%d\n",
               p->X_cm, vitesse(p), p->statique);
        verifier(fabsf(p->X_cm - 250.f) < 2.f, "objet fixe dans le repere terrain");
        verifier(vitesse(p) < 10.f, "vitesse sous le seuil statique malgre notre deplacement");
    }

    // ----------------------------------------------------------------
    titre("Un adversaire lance a 40 cm/s");
    suivi.init();
    date = 1000;
    for (int tour = 0; tour < 10; tour++) {
        const float xo = 200.f + 40.f * tour * (PERIODE_SCAN_MS / 1000.f);
        objets.clear();
        observer(objets, 100.f, 100.f, 0.f, xo, 150.f);
        suivi.nouveauScan(objets, 100.f, 100.f, 0.f, date);
        date += PERIODE_SCAN_MS;
    }
    p = suivi.plusProche(0.f, 0.f);
    if (p) {
        printf("     piste : V=%.1f cm/s (Vx=%.1f Vy=%.1f) statique=%d\n",
               vitesse(p), p->Vx_cms, p->Vy_cms, p->statique);
        verifier(fabsf(vitesse(p) - 40.f) < 5.f, "vitesse rendue a 40 cm/s pres de 5 cm/s");
        verifier(fabsf(p->Vy_cms) < 5.f, "vitesse laterale nulle");
        verifier(!p->statique, "piste declaree mobile");
    }

    // ----------------------------------------------------------------
    titre("Deux objets : l'un fixe, l'autre mobile, sans echange de piste");
    suivi.init();
    date = 1000;
    for (int tour = 0; tour < 10; tour++) {
        const float xo = 200.f + 40.f * tour * (PERIODE_SCAN_MS / 1000.f);
        objets.clear();
        observer(objets, 100.f, 100.f, 0.f, 120.f, 60.f);      // fixe
        observer(objets, 100.f, 100.f, 0.f, xo, 150.f);        // mobile
        suivi.nouveauScan(objets, 100.f, 100.f, 0.f, date);
        date += PERIODE_SCAN_MS;
    }
    verifier(suivi.count() == 2, "deux pistes");
    const tObstacleTrack *fixe = suivi.plusProche(120.f, 60.f);
    if (fixe) {
        verifier(fabsf(fixe->X_cm - 120.f) < 2.f && fixe->statique,
                 "la piste fixe est restee fixe et statique");
    }
    const tObstacleTrack *mobile = suivi.plusProche(245.f, 150.f);
    if (mobile) {
        verifier(!mobile->statique && fabsf(vitesse(mobile) - 40.f) < 6.f,
                 "la piste mobile garde sa vitesse");
    }

    // ----------------------------------------------------------------
    titre("Une piste survit a quelques tours sans mesure, puis est perdue");
    suivi.init();
    date = 1000;
    for (int tour = 0; tour < 6; tour++) {
        objets.clear();
        observer(objets, 100.f, 100.f, 0.f, 150.f, 100.f);
        suivi.nouveauScan(objets, 100.f, 100.f, 0.f, date);
        date += PERIODE_SCAN_MS;
    }
    objets.clear();
    for (int tour = 0; tour < 3; tour++) { suivi.nouveauScan(objets, 100.f, 100.f, 0.f, date); date += PERIODE_SCAN_MS; }
    verifier(suivi.count() == 1, "piste conservee apres 3 tours sans mesure");
    for (int tour = 0; tour < 4; tour++) { suivi.nouveauScan(objets, 100.f, 100.f, 0.f, date); date += PERIODE_SCAN_MS; }
    verifier(suivi.count() == 0, "piste perdue apres 6 tours sans mesure");

    // ----------------------------------------------------------------
    titre("Un saut superieur a la porte cree une piste, il n'y a pas de teleportation");
    suivi.init();
    date = 1000;
    for (int tour = 0; tour < 6; tour++) {
        objets.clear();
        observer(objets, 100.f, 100.f, 0.f, 150.f, 100.f);
        suivi.nouveauScan(objets, 100.f, 100.f, 0.f, date);
        date += PERIODE_SCAN_MS;
    }
    objets.clear();
    observer(objets, 100.f, 100.f, 0.f, 150.f, 130.f);        // 30 cm d'un coup : au-dela de la porte
    suivi.nouveauScan(objets, 100.f, 100.f, 0.f, date);
    date += PERIODE_SCAN_MS;
    verifier(suivi.count() == 2, "deux pistes : l'ancienne survit, une nouvelle est creee");
    const tObstacleTrack *ancienne = suivi.plusProche(150.f, 100.f);
    if (ancienne) verifier(ancienne->sans_mesure == 1, "l'ancienne piste est sans mesure a ce tour");

    // ----------------------------------------------------------------
    titre("Extrapolation entre deux tours de balayage");
    suivi.init();
    date = 1000;
    for (int tour = 0; tour < 10; tour++) {
        const float xo = 200.f + 40.f * tour * (PERIODE_SCAN_MS / 1000.f);
        objets.clear();
        observer(objets, 100.f, 100.f, 0.f, xo, 150.f);
        suivi.nouveauScan(objets, 100.f, 100.f, 0.f, date);
        date += PERIODE_SCAN_MS;
    }
    p = suivi.plusProche(0.f, 0.f);
    const float x_avant = p ? p->X_cm : 0.f;
    for (int pas = 0; pas < 5; pas++) suivi.extrapoler(date - PERIODE_SCAN_MS + (pas+1)*20);  // 5 pas de 20 ms
    p = suivi.plusProche(0.f, 0.f);
    if (p) {
        printf("     avance en 100 ms : %.1f cm (4 cm attendus a 40 cm/s)\n", p->X_cm - x_avant);
        verifier(fabsf((p->X_cm - x_avant) - 4.f) < 1.5f, "la piste avance de sa vitesse entre deux tours");
    }

    // ----------------------------------------------------------------
    titre("Un objet douteux est suivi comme les autres");
    suivi.init();
    date = 1000;
    for (int tour = 0; tour < 6; tour++) {
        objets.clear();
        observer(objets, 100.f, 100.f, 0.f, 150.f, 100.f, true);   // marque douteux par le filtre
        suivi.nouveauScan(objets, 100.f, 100.f, 0.f, date);
        date += PERIODE_SCAN_MS;
    }
    p = suivi.plusProche(100.f, 100.f);
    verifier(suivi.count() == 1, "la piste existe");
    if (p) verifier(p->forme_douteuse, "le doute est transmis a la piste, sans la supprimer");
}
