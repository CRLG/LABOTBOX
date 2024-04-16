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

private :
    CApplication *m_application;

    LidarUtils::tLidarObstacles m_obstacles;
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



