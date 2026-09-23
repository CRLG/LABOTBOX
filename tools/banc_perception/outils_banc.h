// Outils partages par les suites du banc de perception (couches 1 et 2 de l'evitement).
#ifndef _OUTILS_BANC_H_
#define _OUTILS_BANC_H_

void titre(const char *texte);
void verifier(bool condition, const char *libelle);
int  verifications();
int  echecs();

void tests_filtre();     //!< couche 1 : filtre « tracker » sur balayages synthetiques
void tests_suivi();      //!< couche 2 : suivi temporel des objets
void tests_tactique();   //!< couche 3 : evaluation tactique

#endif // _OUTILS_BANC_H_
