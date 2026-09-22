#include <cstdio>
#include <cstdlib>
#include "banc.h"
#include "scenarios.h"
#include "Lidar_utils.h"

// ___________________________________________________________________________
// Etape 1 de l'atelier evitement 2027 : le filtre « tracker » est rebranche, sa sortie « blobs »
// alimente la detection, et le lidar simule fabrique un vrai balayage (mats vus comme des
// cylindres) que ce filtre decoupe. La chaine complete balayage -> filtre -> objets -> detection
// est donc exercee en simulation.
//
// Les proprietes du filtre lui-meme (courbes enveloppes, largeur en degres, bruit) sont verifiees
// a part, sans Simulia ni plugin : voir tools/banc_filtre_lidar.
// ___________________________________________________________________________

static void depart(Banc &banc)
{
    banc.reinitialiser();
    banc.demarrerMatch(0, SM_DatasInterface::EQUIPE_COULEUR_1);
}

void scenarios_etape1(Banc &banc)
{
    banc.titre("Chaine complete : un mat balise traverse balayage, filtre et detection");
    depart(banc);
    banc.obstacle(1, 300, 0);
    banc.passagesModele(6);
    SM_DatasInterface *d = banc.donnees();
    banc.verifier(banc.entrees()->obstacleDetecte, "mat balise a 30 cm : evitement declenche");
    printf("     distance rendue par la chaine : %d mm (300 injectes)\n",
           (int)d->distance_premier_obstacle_detecte);
    banc.verifier(abs((int)d->distance_premier_obstacle_detecte - 300) <= 30,
                  "distance restituee a 30 mm pres : la compensation d'offset du filtre joue");

    banc.titre("Un objet trop large pour sa distance reste un obstacle");
    depart(banc);
    // Deux mats a 12 degres l'un de l'autre a 30 cm : leurs creneaux se touchent et le filtre n'en
    // fait qu'un seul objet, trop large pour sa distance -- donc marque douteux. Il doit malgre tout
    // arreter le robot : un objet large et proche est exactement ce devant quoi il faut s'arreter.
    banc.obstacle(1, 300, 12);
    banc.obstacle(2, 300, -12);
    banc.passagesModele(6);
    banc.verifier(banc.entrees()->obstacleDetecte,
                  "objet douteux mais proche : evitement declenche quand meme");

    banc.titre("Non-regression : l'horizon du filtre ne cree pas d'obstacle fantome");
    depart(banc);
    banc.obstacle(1, 3800, 0);                 // au-dela de m_d_MAX_dist (3,6 m)
    banc.passagesModele(6);
    banc.verifier(!banc.entrees()->obstacleDetecte_non_filtre, "objet a 3,8 m : aucune detection");
}
