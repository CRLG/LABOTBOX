/*! \file CSimulia.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_Simulia_H_
#define _CPLUGIN_MODULE_Simulia_H_

#include <QMainWindow>
#include <QTimer>

#include "CPluginModule.h"
#include "ui_ihm_Simulia.h"

 class Cihm_Simulia : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_Simulia(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_Simulia() { }

    Ui::ihm_Simulia ui;

    CApplication *m_application;
 };



 /*! \addtogroup Simulia
   * 
   *  @{
   */

	   
/*! @brief class CSimulia
 */	   
class CSimulia : public CPluginModule
{
    Q_OBJECT
#define     VERSION_Simulia   "1.0"
#define     AUTEUR_Simulia    "Nico"
#define     INFO_Simulia      "Simulation du modèle IA robot"

public:
    CSimulia(const char *plugin_name);
    ~CSimulia();

    Cihm_Simulia *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

//    IA m_ia;
    QTimer m_timer;
    int m_step;

private:
    Cihm_Simulia m_ihm;
    void simu_task_sequencer();

    void init_lidar();

private slots :
    void onRightClicGUI(QPoint pos);

    void on_pb_init_all();
    void on_pb_active_main();
    void on_pb_stop_all();
    void on_timeout();
    void step_sequencer();
    void on_config_debugger_changed();
    // autoconnect
    void on_speed_simu_valueChanged(int va);
    void on_dde_autotest_pressed();
    void on_origine_telemetre_changed();
    void on_telemetres_gui_changed();
    void on_origine_detect_obstacle_changed();
    void on_detect_obstacle_gui_changed();

    void on_lidar_gui_changed();
    void on_origine_lidar_changed();
    void on_raz_lidar_values();

    void on_select_couleur_equipe(int val);
    void on_select_strategie_match(int val);

    void updatePositionFromSimubot();
    void updateStepFromSimuBot();

    void on_pb_kmar_mouvement_init();
    void on_pb_kmar_mouvement_ramasse();

};

#endif // _CPLUGIN_MODULE_Simulia_H_

/*! @} */

