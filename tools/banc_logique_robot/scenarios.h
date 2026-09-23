// Banc logique robot : suites de scenarios. Une suite par etape de l'atelier evitement 2027.
#ifndef _SCENARIOS_H_
#define _SCENARIOS_H_

class Banc;

void scenarios_etape0(Banc &banc);   // assainissement de la chaine de detection existante
void scenarios_etape1(Banc &banc);   // filtre tracker rebranche, sortie blobs, balayage simule
void scenarios_etape2(Banc &banc);   // suivi temporel dans la chaine complete
void scenarios_etape3(Banc &banc);   // evaluation tactique dans la chaine complete
void scenarios_etape4(Banc &banc);   // strategie AE : echelle de phases reentrante
void scenarios_mode_avant(bool avant);   // ne jouer que les verifications valables sur un plugin anterieur
void trace_match(Banc &banc, int duree_s);   // exploration : trace la position et le sens

#endif // _SCENARIOS_H_
