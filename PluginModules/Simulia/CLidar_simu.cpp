#include <math.h>
#include "CApplication.h"
#include "CDataManager.h"
#include "CLidar_simu.h"

// Reglement : support de balise au maximum un carre de 100 mm de cote (demi-diagonale 70,71 mm)
const double CLidarSimu::RAYON_MAT_BALISE_MM = 70.71;

CLidarSimu::CLidarSimu(QObject *parent)
    : QObject(parent),
      m_date_dernier_scan_ms(0),
      m_premier_scan_fait(false),
      m_nouveau_scan(false),
      m_application(nullptr),
      m_lidar_table_obstacles(nullptr),
      m_lidar_status_combobox(nullptr)
{
    Init();
    setOrigineLidar(LIDAR_FROM_GUI);
}

CLidarSimu::~CLidarSimu()
{

}

// ___________________________________________________
void CLidarSimu::Init()
{
    for (int i=0; i<LidarUtils::NBRE_MAX_OBSTACLES; i++) {
        m_obstacles[i].angle = LidarUtils::NO_OBSTACLE;
        m_obstacles[i].distance = LidarUtils::NO_OBSTACLE;
    }
    m_status = LidarUtils::LIDAR_OK;
    m_date_dernier_scan_ms = 0;
    m_premier_scan_fait = false;
    m_nouveau_scan = false;
    synthetiserBalayage();
    updateDataManager();
}

// ___________________________________________________
/*!
 * \brief Refabrique un tour de balayage a la cadence d'un vrai lidar
 *
 * Appelee a chaque pas de la logique robot, elle ne produit un tour que toutes les PERIODE_SCAN_MS.
 * Le balayage n'est donc PAS refait a chaque changement d'obstacle : c'est la cadence du lidar qui
 * commande, comme sur le robot.
 */
void CLidarSimu::periodicTask(unsigned long date_ms)
{
    // Le premier tour est produit sans attendre ; ensuite, un tour toutes les PERIODE_SCAN_MS.
    // (tester "date nulle" ne marcherait pas : la date VAUT zero au demarrage de la simulation)
    if (!m_premier_scan_fait || (date_ms >= m_date_dernier_scan_ms + PERIODE_SCAN_MS)) {
        m_premier_scan_fait = true;
        m_date_dernier_scan_ms = date_ms;
        synthetiserBalayage();
        m_nouveau_scan = true;
    }
}

// ___________________________________________________
/*!
 * \brief Fabrique un balayage a partir des obstacles connus, puis le filtre comme le fait le driver
 *
 * Chaque obstacle devient un mat balise vu sous sa largeur angulaire reelle
 * (theta = 2.atan(R/D)), pose sur un tour de 360 points d'un degre, origine a -180 degres --
 * la meme convention d'angle signe que la liste d'obstacles. Le filtre "tracker" tourne ensuite
 * dessus, exactement comme sur le robot : la logique robot voit donc en simulation un balayage
 * filtre, et non plus rien du tout.
 *
 * Limite assumee : seuls les obstacles connus sont representes. Ni bordures de terrain, ni
 * elements de jeu, ni bruit de mesure -- un lancer de rayons sur la geometrie de SimuBot serait
 * plus representatif, et reste a faire.
 */
void CLidarSimu::synthetiserBalayage()
{
    m_raw_data.m_start_angle = -180.;
    m_raw_data.m_angle_step_resolution = 360. / NBRE_POINTS_BALAYAGE;
    m_raw_data.m_measures_count = NBRE_POINTS_BALAYAGE;
    for (int i=0; i<NBRE_POINTS_BALAYAGE; i++) {
        m_raw_data.m_dist_measures[i] = LidarUtils::NO_OBSTACLE;
    }

    for (int n=0; n<LidarUtils::NBRE_MAX_OBSTACLES; n++) {
        const double distance = m_obstacles[n].distance;
        if (distance == LidarUtils::NO_OBSTACLE) continue;
        if (distance <= 0.) continue;

        if (distance <= RAYON_MAT_BALISE_MM) continue;

        // Le mat est vu comme un cylindre, et non comme un arc a distance constante : le lidar
        // mesure sa SURFACE. C'est ce que compense l'offset du filtre (la moyenne des points d'un
        // cylindre tombe en deca de son centre) ; un arc plat rendrait cet offset faux et
        // deplacerait tous les obstacles de 4 cm en simulation.
        const double demi_angle_rad = asin(RAYON_MAT_BALISE_MM / distance);
        const int demi_largeur = (int)(demi_angle_rad * 180. / M_PI
                                       / m_raw_data.m_angle_step_resolution);
        const int centre = (int)((m_obstacles[n].angle - m_raw_data.m_start_angle)
                                 / m_raw_data.m_angle_step_resolution);
        for (int i=centre-demi_largeur; i<=centre+demi_largeur; i++) {
            const double alpha_rad = (i - centre) * m_raw_data.m_angle_step_resolution * M_PI / 180.;
            const double ecart = distance * sin(alpha_rad);
            if (fabs(ecart) >= RAYON_MAT_BALISE_MM) continue;      // le rayon passe a cote
            const double portee = distance * cos(alpha_rad)
                                  - sqrt(RAYON_MAT_BALISE_MM*RAYON_MAT_BALISE_MM - ecart*ecart);
            // le tour est cyclique : un objet a +-180 degres deborde d'un bout sur l'autre
            int index = i % NBRE_POINTS_BALAYAGE;
            if (index < 0) index += NBRE_POINTS_BALAYAGE;
            m_raw_data.m_dist_measures[index] = portee;
        }
    }

    m_filtre.filter(&m_raw_data, &m_filtered_data);
}

// ============================================================
//                          SIMULATION
// ============================================================
void CLidarSimu::init(CApplication *application)
{
    m_application = application;
    Init();
}

// ___________________________________________________
void CLidarSimu::setOrigineLidar(int origine)
{
    m_origine_lidar = origine;
    if (m_origine_lidar == LIDAR_FROM_DATA_MANAGER) {
        connect_disconnect_datamanager(true);
    }
    else {
        connect_disconnect_datamanager(false);
    }
    // s'asssure qu'au changement d'origine des donnees, le DataManager et l'IHM sont bien en coherence avec les donnees internes
    updateDataManager();
    refreshGUI(m_lidar_table_obstacles, m_lidar_status_combobox);
}

// ___________________________________________________
// Lorsque l'origine des donnees LIDAR est DataManager :
//      - Un changement de valeur d'une donnee LIDAR du DataManager met a jour les donnees internes
// Lorsque l'origine des donnees LIDAR est l'IHM
//      - Un changement de valeur d'une donnee sur l'IHM met a jour les donnees dans le DataManager
void CLidarSimu::connect_disconnect_datamanager(bool _connect)
{
    if (!m_application) return;
    QString dataname;
    QVector<CData *> data_lst;
    // liste toutes les data a connecter/deconnecter
    for (int i=0; i<LidarUtils::NBRE_MAX_OBSTACLES; i++) {
        dataname= QString("Lidar.Obstacle%1.Angle").arg(i+1);
        data_lst.append(m_application->m_data_center->getData(dataname));

        dataname= QString("Lidar.Obstacle%1.Distance").arg(i+1);
        data_lst.append(m_application->m_data_center->getData(dataname));
    }
    dataname= QString("Lidar.Status");
    data_lst.append(m_application->m_data_center->getData(dataname));

    // effectue la connexion ou deconnection
    foreach (CData *data, data_lst) {
        if (data) {
            if (_connect) {
                connect(data, SIGNAL(valueChanged(QVariant)), this, SLOT(updateFromDataManager()));
            }
            else {
                disconnect(data, SIGNAL(valueChanged(QVariant)), this, SLOT(updateFromDataManager()));
            }
        }
    }
}

// ___________________________________________________
void CLidarSimu::initGUI(QTableWidget *lidar_table_obstacles, QComboBox *lidar_status_combobox)
{
    m_lidar_table_obstacles = lidar_table_obstacles;
    m_lidar_status_combobox = lidar_status_combobox;
    if (!m_lidar_table_obstacles) return;
    if (!m_lidar_status_combobox) return;

    m_lidar_table_obstacles->setRowCount(LidarUtils::NBRE_MAX_OBSTACLES);
    for (int row=0; row<LidarUtils::NBRE_MAX_OBSTACLES; row++) {
        QTableWidgetItem *newItem = new QTableWidgetItem();
        m_lidar_table_obstacles->setItem(row, 0, newItem);

        newItem = new QTableWidgetItem();
        m_lidar_table_obstacles->setItem(row, 1, newItem);
    }
    refreshGUI(m_lidar_table_obstacles, m_lidar_status_combobox);
}

// ___________________________________________________
bool CLidarSimu::is_present()
{
    // Present = statut simule OK. Avec UTILISATION_LIDAR == LIDAR_INTERNE, la logique robot deduit le
    // statut du lidar de is_present() : renvoyer toujours true neutralisait la simulation de panne
    // (statut choisi dans l'IHM Simulia ou case "deconnecter le lidar" de SimuBot).
    return (m_status == LidarUtils::LIDAR_OK);
}

// ___________________________________________________
QStringList CLidarSimu::getOrigines()
{
    QStringList lst;
    lst << "FROM_GUI"
        << "FROM_DATA_MANAGER";
    return lst;
}

// ___________________________________________________
void CLidarSimu::setObstacles(LidarUtils::tLidarObstacle *src)
{
    LidarUtils::copy_tab_obstacles(src, m_obstacles);
    updateDataManager();
}

// ___________________________________________________
// Met a jour la structure interne a partir des donnes de la table sur l'IHM
void CLidarSimu::setObstacles(QTableWidget *lidar_table_obstacles)
{
    if (!lidar_table_obstacles) return;
    for (int i=0; i<lidar_table_obstacles->rowCount(); i++) {
        m_obstacles[i].angle = (signed int)lidar_table_obstacles->item(i, 0)->text().toShort();
        m_obstacles[i].distance = lidar_table_obstacles->item(i, 1)->text().toInt();
    }
    updateDataManager();
}

// ___________________________________________________
// Met a jour la table IHM a partir des donnees internes
void CLidarSimu::refreshGUI(QTableWidget *lidar_table_obstacles, QComboBox *lidar_status_combobox)
{
    if (!lidar_table_obstacles) return;
    if (!lidar_status_combobox) return;

    for (int i=0; i<lidar_table_obstacles->rowCount(); i++) {
        QTableWidgetItem *item = lidar_table_obstacles->item(i, 0);
        if (item) item->setText(QString("%1").arg(m_obstacles[i].angle));

        item = lidar_table_obstacles->item(i, 1);
        if (item) item->setText(QString("%1").arg(m_obstacles[i].distance));
    }

    m_lidar_status_combobox->setCurrentIndex(m_status);
}

// ___________________________________________________
void CLidarSimu::setStatus(int status)
{
    m_status = status;
    updateDataManager("Lidar.Status", m_status);
}

// ___________________________________________________
int CLidarSimu::getStatus()
{
    return m_status;
}
// ___________________________________________________
void CLidarSimu::getObstacles(LidarUtils::tLidarObstacle *dest)
{
    LidarUtils::copy_tab_obstacles(m_obstacles, dest);
}

// ___________________________________________________
void CLidarSimu::updateDataManager(QString dataname, QVariant val)
{
    if (!m_application) return;
    m_application->m_data_center->write(dataname, val);
}

// ___________________________________________________
// Ecrit les valeurs dans le DataManager a partir des donnes internes
// Lidar.Obstacle1.Angle => -10
// Lidar.Obstacle1.Distance => 13
// Lidar.Obstacle2.Angle => -4
// Lidar.Obstacle2.Distance => 6
// ....
// Lidar.Obstacle<n>.Angle => -4
// Lidar.Obstacle<n>.Distance => 6

// 1 obstacle -> 2 datas dans le DataManager
void CLidarSimu::updateDataManager()
{
    if (!m_application) return;
    QString dataname;
    for (int i=0; i<LidarUtils::NBRE_MAX_OBSTACLES; i++) {
        dataname= QString("Lidar.Obstacle%1.Angle").arg(i+1);
        updateDataManager(dataname, m_obstacles[i].angle);

        dataname= QString("Lidar.Obstacle%1.Distance").arg(i+1);
        updateDataManager(dataname, m_obstacles[i].distance);
    }
    dataname= QString("Lidar.Status");
    updateDataManager(dataname, m_status);
}

// ___________________________________________________
// Met a jour les donnees internes a partir du DataManager
void CLidarSimu::updateFromDataManager()
{
    if (!m_application) return;
    QString dataname;
    CData *data;
    for (int i=0; i<LidarUtils::NBRE_MAX_OBSTACLES; i++) {
        dataname= QString("Lidar.Obstacle%1.Angle").arg(i+1);
        data = m_application->m_data_center->getData(dataname);
        if (data) m_obstacles[i].angle = (signed short)data->read().toInt();

        dataname= QString("Lidar.Obstacle%1.Distance").arg(i+1);
        data = m_application->m_data_center->getData(dataname);
        if (data) m_obstacles[i].distance = (unsigned short)data->read().toInt();
    }
    dataname= QString("Lidar.Status");
    data = m_application->m_data_center->getData(dataname);
    if (data) m_status = data->read().toInt();

    // met en coherence la table sur l'IHM (meme lorsque l'origine des donnees est DataManager et que l'IHM est grisee)
    refreshGUI(m_lidar_table_obstacles, m_lidar_status_combobox);
}
