/*! \file CLidarSimu.h
    \brief Classe de simulation du LIDAR
*/

#ifndef _LIDAR_SIMU_H_
#define _LIDAR_SIMU_H_

#include <QObject>
#include <QString>
#include <QVariant>
#include <QTableWidget>
#include <QComboBox>

#include "lidar_data.h"
#include "lidar_data_filter_tracker.h"
#include "lidar_data_filter_example.h"
#include "Lidar_utils.h"

class CApplication;

class CLidarSimu : public QObject
{
    Q_OBJECT
public :
    explicit CLidarSimu(QObject *parent = nullptr);
    ~CLidarSimu();

    void init(CApplication *application);
    void Init();

     typedef enum {
        LIDAR_FROM_GUI = 0,
        LIDAR_FROM_DATA_MANAGER,
    }tOrigineLidar;

    int m_origine_lidar;

    void setOrigineLidar(int origine);
    QStringList getOrigines();

    void setObstacles(LidarUtils::tLidarObstacle *src);
    void setObstacles(QTableWidget *lidar_table_obstacles);

    void getObstacles(LidarUtils::tLidarObstacle *dest);

    void refreshGUI(QTableWidget *lidar_table_obstacles, QComboBox *lidar_status_combobox);

    int getStatus();

    void connect_disconnect_datamanager(bool _connect);

    void initGUI(QTableWidget *lidar_table_obstacles, QComboBox *lidar_status_combobox);

    bool is_present();

    CLidarData m_filtered_data;
    LidarUtils::tLidarObstacles m_obstacles;

    //! Objets decoupes par le filtre sur le balayage synthetise au dernier tour
    const CLidarBlobs& blobs() const { return m_filtre.blobs(); }

    //! Refabrique un tour de balayage a la cadence d'un vrai lidar (~8 Hz), quelle que soit la
    //! cadence d'appel. La logique robot voit ainsi en simulation le meme rythme que sur le robot.
    void periodicTask(unsigned long date_ms);

    //! Meme interface que le driver YdLidar : un tour a-t-il ete produit depuis la derniere
    //! consommation ? Sans cela le suivi temporel prendrait le meme balayage pour une nouvelle
    //! mesure a chaque pas de modele, et estimerait des vitesses nulles.
    bool is_new_scan() const { return m_nouveau_scan; }
    void consume_scan() { m_nouveau_scan = false; }

private :
    // Balayage synthetise a partir des obstacles connus, puis filtre comme sur le robot : la chaine
    // complete (balayage -> filtre -> objets -> detection) est ainsi exercee en simulation. Sans lui,
    // m_filtered_data restait vide et la logique robot ne voyait jamais le balayage.
    void synthetiserBalayage();

    static const int NBRE_POINTS_BALAYAGE = 360;      // 1 point par degre, comme le T-mini nominal
    static const unsigned long PERIODE_SCAN_MS = 125; // 8 Hz, cadence nominale du T-mini plus
    static const double RAYON_MAT_BALISE_MM;          // demi-diagonale du support de balise

    CLidarData m_raw_data;
    CLidarDataFilterTracker m_filtre;
    unsigned long m_date_dernier_scan_ms;
    bool m_premier_scan_fait;
    bool m_nouveau_scan;

    CApplication *m_application;

    int m_status;
    QTableWidget *m_lidar_table_obstacles;
    QComboBox *m_lidar_status_combobox;

    void updateDataManager(QString dataname, QVariant val);
    void updateDataManager();

public Q_SLOTS :
    void updateFromDataManager();
    void setStatus(int status);
};

#endif



