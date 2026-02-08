/*! \file CLidar.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_Lidar_H_
#define _CPLUGIN_MODULE_Lidar_H_

#include <QMainWindow>
#include <QTimer>

#include "CPluginModule.h"
#include "ui_ihm_Lidar.h"

#include "sick_tim561.h"
#include "lidar_data.h"
#include "lidar_data_player.h"
#include "lidar_data_filter_base.h"
#include "Lidar_utils.h"

 class Cihm_Lidar : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_Lidar(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_Lidar() { }

    Ui::ihm_Lidar ui;

    CApplication *m_application;
 };

class CLidarFilterParams;

 /*! \addtogroup Lidar
   * 
   *  @{
   */

	   
/*! @brief class CLidar
 */	   
class CLidar : public CPluginModule
{
    Q_OBJECT
#define     VERSION_Lidar   "1.0"
#define     AUTEUR_Lidar    "Nico"
#define     INFO_Lidar      "Gestion du LIDAR"

public:
    CLidar(const char *plugin_name);
    ~CLidar();

    Cihm_Lidar *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_Lidar m_ihm;

    LidarBase *m_lidar;

    QTimer m_timer_test;

    enum {
        GRAPH_TYPE_POLAR,
        GRAPH_TYPE_LINEAR
    };

    enum {
        GRAPH_RAW_DATA,
        GRAPH_FILTERED_DATA
    };

    const QString DEFAULT_LOGGER_FILENAME = "LogLidar.csv";

    void init_polar_qcustomplot();
    void init_linear_qcustomplot();
    void delete_current_graph();

    CLidarDataFilterBase *m_lidar_data_filter;
    CLidarFilterParams *m_lidar_filter_params;
    void init_data_filter();

    QCPPolarGraph *m_polar_graph;
    QCPPolarAxisAngular *m_angular_axis;

    bool m_logger_active;
    bool m_first_log;
    QFile *m_logger_file;
    const QString CSV_SEPARATOR = ";";
    void log_data(const CLidarData &data);

    CLidarDataPlayer m_data_player;
    void player_parse();

    void send_ETAT_LIDAR(LidarUtils::tLidarObstacles obstacles, unsigned char lidar_status);

    void obstacles_to_datamanager(LidarUtils::tLidarObstacles obstacles);
    void status_to_datamanager(unsigned char lidar_status);

private slots :
    void onRightClicGUI(QPoint pos);

    void create_lidar();
    void lidar_connected();
    void lidar_disconnected();
    void lidar_error(QString msg);
    void new_data(const CLidarData &data);
    void on_change_zoom_distance(int zoom_mm);
    void on_change_graph_type(int choice);
    void on_change_data_displayed(int choice);
    void on_change_data_filter(QString filter_name);
    void on_filter_params_show();
    void filter_params_load();
    void on_filter_params_close();
    void temps_match_changed(QVariant temps_match);
    void active_synchro_match_logger(bool on_off);

signals :
    void test();

public slots :
    void refresh_graph(const CLidarData &data);
    void refresh_polar_graph(const CLidarData &data);
    void refresh_linear_graph(const CLidarData &data);
    void logger_start(QString pathfilename);
    void logger_start();
    void logger_stop();
    void logger_select_file();

    // Gestion du rejeu de trace DataPlayer
    void on_PB_player_choix_trace_clicked(void);
    void on_PlayedStep(int step_num);
    void on_PlayerStepDurationChanged();
};

#endif // _CPLUGIN_MODULE_Lidar_H_

/*! @} */

