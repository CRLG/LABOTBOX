// Banc de perception : verifie les couches 1 (filtre « tracker ») et 2 (suivi temporel) de
// l'evitement, sans Qt, sans Simulia et sans plugin -- donc sans robot.
//
// Deliberement SEPARE de banc_logique_robot, qui charge le plugin : celui-ci contient deja une copie
// de ces classes, et les compiler dans le meme executable ferait primer la copie de l'executable
// (interposition de symboles), masquant silencieusement un ecart entre les deux.
#include <cstdio>
#include "outils_banc.h"

static int g_verifications = 0;
static int g_echecs = 0;

void titre(const char *texte) { printf("\n== %s\n", texte); }

void verifier(bool condition, const char *libelle)
{
    g_verifications++;
    if (!condition) g_echecs++;
    printf("  [%s] %s\n", condition ? " OK " : "ECHEC", libelle);
}

int verifications() { return g_verifications; }
int echecs() { return g_echecs; }

int main()
{
    printf("COUCHE 1 -- filtre « tracker »");
    tests_filtre();
    printf("\n\nCOUCHE 2 -- suivi temporel");
    tests_suivi();
    printf("\n\nCOUCHE 3 -- evaluation tactique");
    tests_tactique();
    printf("\n%d verification(s), %d echec(s)\n", g_verifications, g_echecs);
    return g_echecs == 0 ? 0 : 1;
}
