#include <dlfcn.h>
#include <cstdio>
#include <cstring>
#include "CApplication.h"
#include "Lidar_utils.h"
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
      m_echecs(0), m_verifications(0)
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
