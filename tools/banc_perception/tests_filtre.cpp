// Banc de test du filtre lidar « tracker » sur balayages synthetiques.
//
// Ce que le banc fabrique : un tour de balayage de 360 points (1 point par degre, comme le T-mini a
// sa frequence nominale), ou l'on pose des objets de largeur et de distance choisies. Un objet vu a
// la distance D et de demi-diagonale R occupe une largeur angulaire theta = 2*atan(R/D) : le banc
// s'en sert pour poser des mats balises realistes, des murs (objets larges et lointains) et du
// bruit isole.
//
// Ce qu'il verifie : le decoupage en objets, la largeur angulaire rendue, et surtout le facteur de
// forme borne des deux cotes -- le seuil unique d'avant l'etape 1 acceptait tout objet plus proche
// que la courbe nominale, donc les murs.
#include <cmath>
#include <cstdio>
#include <cstring>
#include "lidar_data.h"
#include "lidar_blob.h"
#include "Lidar_utils.h"
#include "lidar_data_filter_tracker.h"

static const double PI = 3.14159265358979;
static int g_verifications = 0;
static int g_echecs = 0;

static void titre(const char *texte) { printf("\n== %s\n", texte); }

static void verifier(bool condition, const char *libelle)
{
    g_verifications++;
    if (!condition) g_echecs++;
    printf("  [%s] %s\n", condition ? " OK " : "ECHEC", libelle);
}

//! Balayage vide de 360 points, 1 degre par point, origine a 0 degre
static void balayage_vide(CLidarData &scan, int nombre_points = 360)
{
    scan.m_start_angle = 0.;
    scan.m_angle_step_resolution = 360. / nombre_points;
    scan.m_measures_count = nombre_points;
    for (int i = 0; i < nombre_points; i++) scan.m_dist_measures[i] = LidarUtils::NO_OBSTACLE;
}

//! Pose un objet cylindrique de rayon R_mm dont le CENTRE est a distance_mm, vu sous angle_deg.
//! Le lidar mesure la surface du cylindre : la moyenne des points tombe en deca du centre, ce que
//! l'offset du filtre compense. Modeliser un arc a distance constante fausserait cet offset.
static void poser_objet(CLidarData &scan, double angle_deg, double distance_mm, double R_mm)
{
    const double demi_angle_rad = asin(R_mm / distance_mm);
    const int demi = (int)(demi_angle_rad * 180. / PI / scan.m_angle_step_resolution);
    const int centre = (int)(angle_deg / scan.m_angle_step_resolution);
    for (int i = centre - demi; i <= centre + demi; i++) {
        if (i < 0 || i >= scan.m_measures_count) continue;
        const double alpha = (i - centre) * scan.m_angle_step_resolution * PI / 180.;
        const double ecart = distance_mm * sin(alpha);
        if (fabs(ecart) >= R_mm) continue;
        scan.m_dist_measures[i] = distance_mm * cos(alpha) - sqrt(R_mm*R_mm - ecart*ecart);
    }
}

//! Objet de largeur angulaire imposee (pour fabriquer un mur : large ET lointain)
static void poser_objet_large(CLidarData &scan, double angle_deg, double distance_mm, double largeur_deg)
{
    const int demi = (int)(0.5 * largeur_deg / scan.m_angle_step_resolution);
    const int centre = (int)(angle_deg / scan.m_angle_step_resolution);
    for (int i = centre - demi; i <= centre + demi; i++) {
        if (i < 0 || i >= scan.m_measures_count) continue;
        scan.m_dist_measures[i] = distance_mm;
    }
}

//! Nombre de points retenus dans le balayage filtre (objets ecrits dans data_out)
static int points_retenus(const CLidarData &sortie)
{
    int n = 0;
    for (int i = 0; i < sortie.m_measures_count; i++) {
        if (sortie.m_dist_measures[i] != LidarUtils::NO_OBSTACLE) n++;
    }
    return n;
}

//! Cherche un objet retenu (non douteux) proche d'un angle donne ; rend son index ou -1
static int trouver_blob(const CLidarBlobs &blobs, double angle_deg, double tolerance_deg = 4.)
{
    for (int i = 0; i < blobs.m_count; i++) {
        if (fabs(blobs.m_blobs[i].angle_deg - angle_deg) <= tolerance_deg) return i;
    }
    return -1;
}

int main()
{
    CLidarDataFilterTracker filtre;
    CLidarData entree, sortie;

    // Demi-diagonale d'un mat balise au maximum du reglement (carre de 100 mm)
    const double R_MAT = 70.71;

    // ----------------------------------------------------------------
    titre("Un mat balise est retenu, a plusieurs distances");
    for (int d = 300; d <= 1500; d += 400) {
        balayage_vide(entree);
        poser_objet(entree, 90., d, R_MAT);
        filtre.filter(&entree, &sortie);
        const CLidarBlobs &blobs = filtre.blobs();
        char libelle[160];
        const int idx = trouver_blob(blobs, 90.);
        snprintf(libelle, sizeof(libelle), "mat balise a %d mm : un objet retenu a 90 deg", d);
        verifier(blobs.m_count == 1 && idx == 0 && !blobs.m_blobs[0].forme_douteuse, libelle);
        if (idx >= 0) {
            // la moyenne des points de surface plus l'offset doit retomber pres du centre du mat
            const double ecart = fabs(blobs.m_blobs[idx].distance_mm - d);
            snprintf(libelle, sizeof(libelle),
                     "distance rendue a moins de 25 mm du centre (%.0f mm rendus, %d attendus)",
                     blobs.m_blobs[idx].distance_mm, d);
            verifier(ecart < 25., libelle);
        }
    }

    // ----------------------------------------------------------------
    titre("Un mur (objet large et lointain) est rejete par les courbes enveloppes");
    balayage_vide(entree);
    // 2 m et 7 deg de large : un objet de ~25 cm (element de jeu, flanc de robot, pan de mur decoupe
    // par le filtre de gradient). Impossible pour un mat balise a cette distance.
    poser_objet_large(entree, 90., 2000., 7.);
    filtre.filter(&entree, &sortie);
    verifier(filtre.blobs().m_count == 1, "un objet decoupe");
    verifier(filtre.blobs().m_count == 1 && filtre.blobs().m_blobs[0].forme_douteuse,
             "objet marque douteux (large et lointain)");
    verifier(points_retenus(sortie) == 0, "aucun point dans le balayage filtre (non retenu)");

    titre("Le meme mur passait le seuil unique d'avant l'etape 1");
    {
        // ancien test : accepte si (d + offset) < 70.7106781/tan(0.00872665*i_COUNT) + 1400
        const double i_COUNT = 7.;                   // 7 points pour 7 deg a 1 deg/point
        const double facteur_forme = 70.7106781 / tan(0.00872665 * i_COUNT);
        const bool accepte_avant = (2000. + filtre.m_d_dist_offset) < (facteur_forme + 1400.);
        verifier(accepte_avant, "l'ancien critere l'acceptait : c'est le defaut corrige");
    }

    // ----------------------------------------------------------------
    titre("Un objet trop proche pour sa largeur est rejete");
    balayage_vide(entree);
    poser_objet_large(entree, 180., 200., 3.);       // 20 cm mais seulement 3 deg de large
    filtre.filter(&entree, &sortie);
    verifier(filtre.blobs().m_count == 1 && filtre.blobs().m_blobs[0].forme_douteuse,
             "objet marque douteux (trop fin pour sa distance)");

    // ----------------------------------------------------------------
    titre("Largeur angulaire rendue : degres, et non nombre de points");
    balayage_vide(entree, 360);                      // 1 deg par point
    poser_objet(entree, 90., 500., R_MAT);
    filtre.filter(&entree, &sortie);
    const double largeur_360 = filtre.blobs().m_count ? filtre.blobs().m_blobs[0].largeur_deg : 0.;
    balayage_vide(entree, 720);                      // 0,5 deg par point : deux fois plus de points
    poser_objet(entree, 90., 500., R_MAT);
    filtre.filter(&entree, &sortie);
    const double largeur_720 = filtre.blobs().m_count ? filtre.blobs().m_blobs[0].largeur_deg : 0.;
    const double theorie = 2. * atan2(R_MAT, 500.) * 180. / PI;
    printf("     largeur a 360 points : %.1f deg | a 720 points : %.1f deg | theorie : %.1f deg\n",
           largeur_360, largeur_720, theorie);
    verifier(fabs(largeur_360 - theorie) < 2.5, "largeur correcte a 360 points par tour");
    verifier(fabs(largeur_720 - theorie) < 2.5, "largeur INCHANGEE a 720 points par tour");

    // ----------------------------------------------------------------
    titre("Deux mats balises distincts sont separes");
    balayage_vide(entree);
    poser_objet(entree, 60., 600., R_MAT);
    poser_objet(entree, 200., 900., R_MAT);
    filtre.filter(&entree, &sortie);
    verifier(filtre.blobs().m_count == 2, "deux objets decoupes");
    verifier(trouver_blob(filtre.blobs(), 60.) >= 0 && trouver_blob(filtre.blobs(), 200.) >= 0,
             "objets rendus aux bons angles");
    verifier(points_retenus(sortie) == 2, "deux points dans le balayage filtre");

    // ----------------------------------------------------------------
    titre("Le bruit isole et les objets hors zone sont ignores");
    balayage_vide(entree);
    entree.m_dist_measures[45] = 800.;               // un point isole : sous m_i_MIN_COUNT
    poser_objet(entree, 270., 100., R_MAT);          // 10 cm : sous m_d_MIN_dist (150 mm)
    poser_objet(entree, 300., 4000., R_MAT);         // 4 m : au-dela de m_d_MAX_dist (3,6 m)
    filtre.filter(&entree, &sortie);
    verifier(filtre.blobs().m_count == 0, "aucun objet decoupe");
    verifier(points_retenus(sortie) == 0, "balayage filtre vide");

    // ----------------------------------------------------------------
    titre("Un balayage vide ne rend rien (et ne plante pas)");
    balayage_vide(entree);
    filtre.filter(&entree, &sortie);
    verifier(filtre.blobs().m_count == 0 && points_retenus(sortie) == 0, "aucun objet");

    printf("\n%d verification(s), %d echec(s)\n", g_verifications, g_echecs);
    return g_echecs == 0 ? 0 : 1;
}
