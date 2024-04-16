#include "CApplication.h"
#include "CDataManager.h"
#include "CLidar_simu.h"

CLidarSimu::CLidarSimu(QObject *parent)
    : QObject(parent),
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
    updateDataManager();
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
