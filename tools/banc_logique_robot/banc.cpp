#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include <cmath>
#include "CApplication.h"
#include "Lidar_utils.h"
#include "RobotGeometrySimu.h"
#include "banc.h"

// Le plugin ne lit de CApplication que le pointeur m_data_center (verifie sur ses sources : aucune
// methode de CApplication n'est appelee, aucun autre membre n'est lu). On lui fournit donc un bloc
// memoire a la disposition de CApplication -- compile avec la MEME liste de MODULE_* que le plugin,
// dont depend la position des membres -- ou seul ce pointeur est renseigne. L'objet n'est jamais
// construit ni utilise comme QObject.
alignas(CApplication) static unsigned char g_stockage_app[sizeof(CApplication)];

// Origine des donnees du lidar simule : ordre de l'enum CLidarSimu::tOrigineLidar
static const int LIDAR_FROM_DATA_MANAGER = 1;

Banc::Banc(const QString &chemin_plugin)
    : m_handle(nullptr), m_logique(nullptr), m_detruire(nullptr), m_app(nullptr),
      m_echecs(0), m_verifications(0),
      m_adversaire_x_cm(0.f), m_adversaire_y_cm(0.f), m_adversaire_actif(false),
      m_reste_pas_G(0.f), m_reste_pas_D(0.f)
{
    memset(g_stockage_app, 0, sizeof(g_stockage_app));
    m_app = reinterpret_cast<CApplication *>(g_stockage_app);
    m_app->m_data_center = &m_dm;

    // RTLD_LOCAL comme Simulia ; le plugin resout CDataManager et CData dans l'executable du banc,
    // lie en -rdynamic.
    m_handle = dlopen(chemin_plugin.toLocal8Bit().constData(), RTLD_NOW | RTLD_LOCAL);
    if (!m_handle) {
        fprintf(stderr, "Chargement impossible : %s\n", dlerror());
        return;
    }
    typedef IRobotLogic *(*tCreer)();
    tCreer creer = reinterpret_cast<tCreer>(dlsym(m_handle, "createRobotLogic"));
    m_detruire = reinterpret_cast<void (*)(IRobotLogic *)>(dlsym(m_handle, "destroyRobotLogic"));
    if (!creer || !m_detruire) {
        fprintf(stderr, "Fabrique absente du plugin : %s\n", dlerror());
        return;
    }
    m_logique = creer();
    // Meme sequence d'initialisation que CSimulia::init(), hors IHM
    m_logique->initSubsystems(m_app);
    m_logique->resetSimulation();
}

Banc::~Banc()
{
    if (m_logique && m_detruire) m_detruire(m_logique);
    if (m_handle) dlclose(m_handle);
}

void Banc::reinitialiser()
{
    m_adversaire_actif = false;
    m_reste_pas_G = 0.f;
    m_reste_pas_D = 0.f;
    m_logique->resetSimulation();
    m_logique->resetStep();
    m_logique->setOrigineLidar(LIDAR_FROM_DATA_MANAGER);
    aucunObstacle();
    statutLidar(LidarUtils::LIDAR_OK);
}

void Banc::demarrerMatch(int strategie, int couleur)
{
    m_logique->setStrategie(strategie);
    m_logique->setCouleurEquipe(couleur);
    m_logique->startMain();
    passagesModele(5);                              // SM_Main en ATTENTE_TIRETTE
    m_dm.write("Capteurs.Tirette", true);           // gotoStateIfTrue(MATCH_EN_COURS, Tirette)
    passagesModele(2);
}

void Banc::pas(int n)
{
    for (int i = 0; i < n; i++) m_logique->step();
}

int Banc::attendreDetectionBrute(int passages_max)
{
    for (int n = 1; n <= passages_max; n++) {
        passagesModele(1);
        if (m_logique->inputs()->obstacleDetecte_non_filtre) return n;
    }
    return -1;
}

int Banc::attendreFinDetectionBrute(int passages_max)
{
    for (int n = 1; n <= passages_max; n++) {
        passagesModele(1);
        if (!m_logique->inputs()->obstacleDetecte_non_filtre) return n;
    }
    return -1;
}

void Banc::obstacle(int index, int distance_mm, int angle_deg)
{
    m_dm.write(QString("Lidar.Obstacle%1.Angle").arg(index), angle_deg);
    m_dm.write(QString("Lidar.Obstacle%1.Distance").arg(index), distance_mm);
}

void Banc::aucunObstacle()
{
    for (int i = 1; i <= LidarUtils::NBRE_MAX_OBSTACLES; i++) {
        obstacle(i, LidarUtils::NO_OBSTACLE, LidarUtils::NO_OBSTACLE);
    }
}

// Cap du robot dans le repere terrain : meme convention que IA::capTerrainRobot(). Les coordonnees
// terrain se deduisent de celles de l'asservissement par une translation (couleur 1) ; le cap vaut
// donc angle_robot, et NON angle_robot_terrain, qui est le cap relatif au depart.
void Banc::adversaireEnPositionTerrain(float x_cm, float y_cm)
{
    m_adversaire_x_cm = x_cm;
    m_adversaire_y_cm = y_cm;
    m_adversaire_actif = true;
    rafraichirAdversaire();
}

void Banc::retirerAdversaire()
{
    m_adversaire_actif = false;
    aucunObstacle();
}

void Banc::rafraichirAdversaire()
{
    if (!m_adversaire_actif) return;
    SM_InputsInterface *in = m_logique->inputs();
    const float dx = m_adversaire_x_cm - in->X_robot_terrain;
    const float dy = m_adversaire_y_cm - in->Y_robot_terrain;
    const float distance_cm = sqrtf(dx*dx + dy*dy);
    float angle_rad = atan2f(dy, dx) - in->angle_robot;
    while (angle_rad > (float)M_PI)  angle_rad -= 2.f*(float)M_PI;
    while (angle_rad < -(float)M_PI) angle_rad += 2.f*(float)M_PI;
    obstacle(1, (int)(distance_cm * 10.f), (int)(angle_rad * 180.f / (float)M_PI));
}

void Banc::simuler(int passages, float vitesse_avance_cms, float vitesse_rotation_rads)
{
    const float dt = 0.02f;                       // un passage de modele
    for (int n = 0; n < passages; n++) {
        // cinematique differentielle : distance de chaque roue sur le pas, convertie en pas codeurs
        const float avance_cm = vitesse_avance_cms * dt;
        const float ecart_cm = vitesse_rotation_rads * dt * RobotGeometrySimu::VOIE_ROBOT / 2.f;
        const float pas_G = (avance_cm - ecart_cm) / RobotGeometrySimu::DISTANCE_PAR_PAS_CODEUR_G + m_reste_pas_G;
        const float pas_D = (avance_cm + ecart_cm) / RobotGeometrySimu::DISTANCE_PAR_PAS_CODEUR_D + m_reste_pas_D;
        const int entiers_G = (int)pas_G;
        const int entiers_D = (int)pas_D;
        m_reste_pas_G = pas_G - (float)entiers_G;  // la fraction est reportee : pas de derive
        m_reste_pas_D = pas_D - (float)entiers_D;
        if (entiers_G || entiers_D) m_logique->addCodeurSteps(entiers_G, entiers_D);

        rafraichirAdversaire();
        passagesModele(1);
    }
}

void Banc::statutLidar(int statut)
{
    m_dm.write("Lidar.Status", statut);
}

void Banc::verifier(bool condition, const QString &libelle)
{
    m_verifications++;
    if (!condition) m_echecs++;
    printf("  [%s] %s\n", condition ? " OK " : "ECHEC", libelle.toLocal8Bit().constData());
}

void Banc::titre(const QString &texte)
{
    printf("\n== %s\n", texte.toLocal8Bit().constData());
}
