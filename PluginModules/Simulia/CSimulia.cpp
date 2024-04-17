/*! \file CSimulia.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QTableWidgetItem>
#include "CSimulia.h"
#include "CSimuBot.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"

#include "sm_debugqdebug.h"
#include "CGlobale.h"  // c'est le CGlobal.h spécifique pour la simulation

/*! \addtogroup Module_Test2
   *
   *  @{
   */

// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CSimulia::CSimulia(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_Simulia, AUTEUR_Simulia, INFO_Simulia)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CSimulia::~CSimulia()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CSimulia::init(CApplication *application)
{
    CModule::init(application);
    setGUI(&m_ihm); // indique à la classe de base l'IHM

    // ordre identique à l'enum CTelemetre_simu
    m_ihm.ui.OrigineTelemetres->addItem("FROM SIMU");
    m_ihm.ui.OrigineTelemetres->addItem("FROM GUI");
    m_ihm.ui.OrigineTelemetres->addItem("FROM SIMUBOT");
    // ordre identique à l'enum CDetectionObstacles_simu
    m_ihm.ui.OrigineDetectionObstacles->addItem("FROM SIMU");
    m_ihm.ui.OrigineDetectionObstacles->addItem("FROM GUI");
    m_ihm.ui.OrigineDetectionObstacles->addItem("FROM TELEMETRES");
    // ordre identique à l'enum CLidar_simu
    m_ihm.ui.OrigineLidar->addItems(Application.m_lidar.getOrigines());
    //  ordre identique a eLidarStatus
    m_ihm.ui.lidar_status->addItem("LIDAR_OK");
    m_ihm.ui.lidar_status->addItem("LIDAR_DISCONNECTED");
    m_ihm.ui.lidar_status->addItem("LIDAR_ERROR");

    // Gère les actions sur clic droit sur le panel graphique du module
    m_ihm.setContextMenuPolicy(Qt::CustomContextMenu);
    connect(&m_ihm, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRightClicGUI(QPoint)));

    // Restore la taille de la fenêtre
    QVariant val;
    val = m_application->m_eeprom->read(getName(), "geometry", QRect(50, 50, 150, 150));
    m_ihm.setGeometry(val.toRect());
    // Restore le fait que la fenêtre est visible ou non
    val = m_application->m_eeprom->read(getName(), "visible", QVariant(true));
    if (val.toBool()) { m_ihm.show(); }
    else              { m_ihm.hide(); }
    // Restore le niveau d'affichage
    val = m_application->m_eeprom->read(getName(), "niveau_trace", QVariant(MSG_TOUS));
    setNiveauTrace(val.toUInt());
    // Restore la couleur de fond
    val = m_application->m_eeprom->read(getName(), "background_color", QVariant(DEFAULT_MODULE_COLOR));
    setBackgroundColor(val.value<QColor>());
    // Restore la provenance des données télémètres
    val = m_application->m_eeprom->read(getName(), "origine_donnees_telemetres", QVariant(CTelemetresSimu::TELEMETRES_FROM_SIMU));
    m_ihm.ui.OrigineTelemetres->setCurrentIndex(val.toInt());
    on_origine_telemetre_changed();
    // Restore la provenance des données détection d'obstacle
    val = m_application->m_eeprom->read(getName(), "origine_donnees_detection_obstacle", QVariant(CDetectionObstaclesSimu::OBSTACLES_FROM_TELEMETRES));
    m_ihm.ui.OrigineDetectionObstacles->setCurrentIndex(val.toInt());
    on_origine_detect_obstacle_changed();
    // Restore la couleur de l'équipe
    val = m_application->m_eeprom->read(getName(), "couleur_equipe", QVariant(0));
    m_ihm.ui.CouleurEquipe->setCurrentIndex(val.toInt());
    Application.m_modelia.m_inputs_interface.dde_couleur_equipe = val.toInt();
    // Restore le numéro de stratégie de match
    val = m_application->m_eeprom->read(getName(), "choix_strategie", QVariant(0));
    m_ihm.ui.StrategieMatch->setCurrentIndex(val.toInt());
    Application.m_modelia.setStrategie(val.toInt());

    Application.m_modelia.setDebugger(new SM_DebugQDebug());

    // passe à toutes les classes de simulation l'application Simulia
    Application.m_capteurs.init(m_application);
    Application.m_roues.init(m_application);
    Application.m_servos_ax.init(m_application);
    Application.m_asservissement.init(m_application);
    Application.m_servos_sd20.init(m_application);
    Application.m_led1.init(m_application);
    Application.m_led2.init(m_application);
    Application.m_led3.init(m_application);
    Application.m_led4.init(m_application);
    Application.m_power_electrobot.init(m_application);
    Application.m_messenger_xbee_ntw.init(m_application);
    Application.m_asservissement_chariot.init(m_application);
    Application.m_telemetres.init(m_application);
    Application.m_lidar.init(m_application);
    Application.m_detection_obstacles.init(m_application);

    init_lidar();
    Application.m_lidar.setOrigineLidar(CLidarSimu::LIDAR_FROM_GUI);
    m_ihm.ui.OrigineLidar->setCurrentIndex(Application.m_lidar.m_origine_lidar);
    on_origine_lidar_changed();

    // Mise en cohérence de l'IHM avec l'état interne
    m_ihm.ui.actionActive_Start->setChecked(Application.m_modelia.m_sm_debug->m_active_start);
    m_ihm.ui.actionActive_Stop->setChecked(Application.m_modelia.m_sm_debug->m_active_stop);
    m_ihm.ui.actionActive_onEntry->setChecked(Application.m_modelia.m_sm_debug->m_active_on_entry);
    m_ihm.ui.actionActive_onExit->setChecked(Application.m_modelia.m_sm_debug->m_active_on_exit);
    m_ihm.ui.actionActive_Interrupt_Evitement->setChecked(Application.m_modelia.m_sm_debug->m_active_interrupt_evitement);

    connect(m_ihm.ui.pb_start_main, SIGNAL(pressed()), this, SLOT(on_pb_active_main()));
    connect(m_ihm.ui.pb_stop_all, SIGNAL(pressed()), this, SLOT(on_pb_stop_all()));
    connect(m_ihm.ui.pb_init_all, SIGNAL(pressed()), this, SLOT(on_pb_init_all()));
    connect(m_ihm.ui.dde_autotest, SIGNAL(pressed()), this, SLOT(on_dde_autotest_pressed()));

    connect(m_ihm.ui.dde_kmar_init_position, SIGNAL(pressed()), this, SLOT(on_pb_kmar_mouvement_init()));
    connect(m_ihm.ui.dde_kmar_ramasse, SIGNAL(pressed()), this, SLOT(on_pb_kmar_mouvement_ramasse()));

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(on_timeout()));
    connect(m_ihm.ui.PlaySimu, SIGNAL(clicked(bool)), &m_timer, SLOT(start()));
    connect(m_ihm.ui.PauseSimu, SIGNAL(clicked(bool)), &m_timer, SLOT(stop()));
    connect(m_ihm.ui.StepSimu, SIGNAL(clicked(bool)), this, SLOT(step_sequencer()));
    connect(m_ihm.ui.speed_simu, SIGNAL(valueChanged(int)), this, SLOT(on_speed_simu_valueChanged(int)));

    connect(m_ihm.ui.actionActive_Start, SIGNAL(changed()), this, SLOT(on_config_debugger_changed()));
    connect(m_ihm.ui.actionActive_Stop, SIGNAL(changed()), this, SLOT(on_config_debugger_changed()));
    connect(m_ihm.ui.actionActive_onEntry, SIGNAL(changed()), this, SLOT(on_config_debugger_changed()));
    connect(m_ihm.ui.actionActive_onExit, SIGNAL(changed()), this, SLOT(on_config_debugger_changed()));
    connect(m_ihm.ui.actionActive_Interrupt_Evitement, SIGNAL(changed()), this, SLOT(on_config_debugger_changed()));

    connect(m_ihm.ui.OrigineTelemetres, SIGNAL(currentIndexChanged(int)), this, SLOT(on_origine_telemetre_changed()));
    connect(m_ihm.ui.Telemetre_AVG, SIGNAL(editingFinished()), this, SLOT(on_telemetres_gui_changed()));
    connect(m_ihm.ui.Telemetre_AVD, SIGNAL(editingFinished()), this, SLOT(on_telemetres_gui_changed()));
    connect(m_ihm.ui.Telemetre_ARG, SIGNAL(editingFinished()), this, SLOT(on_telemetres_gui_changed()));
    connect(m_ihm.ui.Telemetre_ARD, SIGNAL(editingFinished()), this, SLOT(on_telemetres_gui_changed()));

    connect(m_ihm.ui.OrigineDetectionObstacles, SIGNAL(currentIndexChanged(int)), this, SLOT(on_origine_detect_obstacle_changed()));
    connect(m_ihm.ui.detectionObstacle_AVG, SIGNAL(clicked(bool)), this, SLOT(on_detect_obstacle_gui_changed()));
    connect(m_ihm.ui.detectionObstacle_AVD, SIGNAL(clicked(bool)), this, SLOT(on_detect_obstacle_gui_changed()));
    connect(m_ihm.ui.detectionObstacle_ARG, SIGNAL(clicked(bool)), this, SLOT(on_detect_obstacle_gui_changed()));
    connect(m_ihm.ui.detectionObstacle_ARD, SIGNAL(clicked(bool)), this, SLOT(on_detect_obstacle_gui_changed()));

    connect(m_ihm.ui.OrigineLidar, SIGNAL(currentIndexChanged(int)), this, SLOT(on_origine_lidar_changed()));

    connect(m_ihm.ui.CouleurEquipe, SIGNAL(currentIndexChanged(int)), this, SLOT(on_select_couleur_equipe(int)));
    connect(m_ihm.ui.StrategieMatch, SIGNAL(currentIndexChanged(int)), this, SLOT(on_select_strategie_match(int)));

    // Connexions DataManger -> IHM
    connect(m_application->m_data_center->getData("TempsMatch", true), SIGNAL(valueChanged(double)), m_ihm.ui.TempsMatch, SLOT(setValue(double)));
    connect(m_application->m_data_center->getData("PowerElectrobot.OUTPUT_STOR1", true), SIGNAL(valueChanged(bool)), m_ihm.ui.power_electrobot_sw_1, SLOT(setValue(bool)));
    connect(m_application->m_data_center->getData("PowerElectrobot.OUTPUT_STOR2", true), SIGNAL(valueChanged(bool)), m_ihm.ui.power_electrobot_sw_3, SLOT(setValue(bool)));
    connect(m_application->m_data_center->getData("PowerElectrobot.OUTPUT_STOR3", true), SIGNAL(valueChanged(bool)), m_ihm.ui.power_electrobot_sw_3, SLOT(setValue(bool)));
    connect(m_application->m_data_center->getData("PowerElectrobot.OUTPUT_STOR4", true), SIGNAL(valueChanged(bool)), m_ihm.ui.power_electrobot_sw_4, SLOT(setValue(bool)));
    connect(m_application->m_data_center->getData("PowerElectrobot.OUTPUT_STOR5", true), SIGNAL(valueChanged(bool)), m_ihm.ui.power_electrobot_sw_5, SLOT(setValue(bool)));
    connect(m_application->m_data_center->getData("PowerElectrobot.OUTPUT_STOR6", true), SIGNAL(valueChanged(bool)), m_ihm.ui.power_electrobot_sw_6, SLOT(setValue(bool)));
    connect(m_application->m_data_center->getData("PowerElectrobot.OUTPUT_STOR7", true), SIGNAL(valueChanged(bool)), m_ihm.ui.power_electrobot_sw_7, SLOT(setValue(bool)));
    connect(m_application->m_data_center->getData("PowerElectrobot.OUTPUT_STOR8", true), SIGNAL(valueChanged(bool)), m_ihm.ui.power_electrobot_sw_8, SLOT(setValue(bool)));

    connect(m_application->m_data_center->getData("Led1", true), SIGNAL(valueChanged(bool)), m_ihm.ui.led_mbed_1, SLOT(setValue(bool)));
    connect(m_application->m_data_center->getData("Led2", true), SIGNAL(valueChanged(bool)), m_ihm.ui.led_mbed_2, SLOT(setValue(bool)));
    connect(m_application->m_data_center->getData("Led3", true), SIGNAL(valueChanged(bool)), m_ihm.ui.led_mbed_3, SLOT(setValue(bool)));
    connect(m_application->m_data_center->getData("Led4", true), SIGNAL(valueChanged(bool)), m_ihm.ui.led_mbed_4, SLOT(setValue(bool)));

    connect(m_ihm.ui.Tirette, SIGNAL(clicked(bool)), m_application->m_data_center->getData("Capteurs.Tirette", true), SLOT(write(bool)));

    connect(m_application->m_data_center->getData("Simubot.step", true), SIGNAL(valueChanged(bool)), this, SLOT(updateStepFromSimuBot()));

    connect(m_application->m_data_center->getData("PosX_robot", true), SIGNAL(valueChanged(bool)), this, SLOT(updatePositionFromSimubot()));
    connect(m_application->m_data_center->getData("PosY_robot", true), SIGNAL(valueChanged(bool)), this, SLOT(updatePositionFromSimubot()));
    connect(m_application->m_data_center->getData("PosTeta_robot", true), SIGNAL(valueChanged(bool)), this, SLOT(updatePositionFromSimubot()));

    m_step=0;

    m_timer.start(10);
    m_ihm.ui.speed_simu->setValue(m_timer.interval());
}

// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CSimulia::close(void)
{
    // Mémorise en EEPROM l'état de la fenêtre
    m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
    m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
    m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
    m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
    m_application->m_eeprom->write(getName(), "origine_donnees_telemetres", QVariant(m_ihm.ui.OrigineTelemetres->currentIndex()));
    m_application->m_eeprom->write(getName(), "origine_donnees_detection_obstacle", QVariant(m_ihm.ui.OrigineDetectionObstacles->currentIndex()));
    m_application->m_eeprom->write(getName(), "couleur_equipe", QVariant(m_ihm.ui.CouleurEquipe->currentIndex()));
    m_application->m_eeprom->write(getName(), "choix_strategie", QVariant(m_ihm.ui.StrategieMatch->currentIndex()));
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CSimulia::onRightClicGUI(QPoint pos)
{
    QMenu *menu = new QMenu();

    menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
    menu->exec(m_ihm.mapToGlobal(pos));
}


void CSimulia::on_pb_active_main()
{
    Application.m_modelia.m_sm_main.start();
}

void CSimulia::on_pb_stop_all()
{
    Application.m_modelia.stopAllStateMachines();
    m_timer.stop();
}

void CSimulia::on_pb_init_all()
{
    m_timer.stop();

    // initialise l'asservissement et le modèle simulink des moteurs
    Application.m_capteurs.Init();
    Application.m_asservissement.CommandeManuelle(0, 0);
    Application.m_asservissement.Init();
    Application.m_roues.init_model();
    Application.m_servos_sd20.Init();
    Application.m_servos_ax.Init();
    Application.m_leds.setState(ALL_LED, 0);
    Application.m_asservissement_chariot.Init();
    Application.m_asservissement_chariot.Recal_Chariot();
    Application.m_telemetres.Init();
    Application.m_lidar.Init();
    Application.m_detection_obstacles.Init();

    // initialise les machines d'états Modelia du robot
    Application.m_modelia.init();
    Application.m_modelia.setStrategie(m_ihm.ui.StrategieMatch->currentIndex());
    Application.m_modelia.m_inputs_interface.dde_couleur_equipe = m_ihm.ui.CouleurEquipe->currentIndex();

    // Remet la tirette en position
    m_application->m_data_center->write("Capteurs.Tirette", false);
    m_ihm.ui.Tirette->setChecked(false);

    //Synchronise Simubot avec l'init
    m_application->m_data_center->write("Simubot.Init", true);

    m_step=0;

    m_timer.start();
}

void CSimulia::on_timeout()
{
    step_sequencer();
}

void CSimulia::step_sequencer()
{
    int old_step=m_step;
    // Mise en cohérence de l'IHM de simu avec le reste du modèle
    m_ihm.ui.led_isObstacle->setValue(Application.m_modelia.m_inputs_interface.obstacleDetecte); // La LED représente une détection d'obstacle confirmée
    if (m_ihm.ui.OrigineDetectionObstacles->currentIndex() != CDetectionObstaclesSimu::OBSTACLE_FROM_GUI) {
        m_ihm.ui.detectionObstacle_AVG->setChecked(Application.m_modelia.m_inputs_interface.obstacle_AVG);
        m_ihm.ui.detectionObstacle_AVD->setChecked(Application.m_modelia.m_inputs_interface.obstacle_AVD);
        m_ihm.ui.detectionObstacle_ARG->setChecked(Application.m_modelia.m_inputs_interface.obstacle_ARG);
        m_ihm.ui.detectionObstacle_ARD->setChecked(Application.m_modelia.m_inputs_interface.obstacle_ARD);
    }

    if (m_ihm.ui.OrigineTelemetres->currentIndex() != CTelemetresSimu::TELEMETRES_FROM_GUI) {
        // Mise à jour des valeurs télémètres en fonction de leur provenance
        m_ihm.ui.Telemetre_AVG->setValue(Application.m_modelia.m_inputs_interface.Telemetre_AVG);
        m_ihm.ui.Telemetre_AVD->setValue(Application.m_modelia.m_inputs_interface.Telemetre_AVD);
        m_ihm.ui.Telemetre_ARG->setValue(Application.m_modelia.m_inputs_interface.Telemetre_ARG);
        m_ihm.ui.Telemetre_ARD->setValue(Application.m_modelia.m_inputs_interface.Telemetre_ARD);
    }

    // Met a jour les donnees LIDAR Modelia
    Application.m_modelia.m_inputs_interface.m_lidar_status = Application.m_lidar.getStatus();
    Application.m_lidar.getObstacles(Application.m_modelia.m_inputs_interface.m_lidar_obstacles);

    Application.m_power_electrobot.simuSetGlobalCurrent(m_ihm.ui.power_electrobot_global_current->value());

    // Step
    simu_task_sequencer();

    // Outputs -> IHM
    m_application->m_data_center->write("TempsMatch", Application.m_modelia.m_datas_interface.TempsMatch);

    m_application->m_data_center->write("DetectionObstacle.proximite_bordure_Xdroite", Application.m_modelia.m_datas_interface.proximite_bordure_Xdroite);
    m_application->m_data_center->write("DetectionObstacle.proximite_bordure_Xgauche", Application.m_modelia.m_datas_interface.proximite_bordure_Xgauche);
    m_application->m_data_center->write("DetectionObstacle.proximite_bordure_Ybasse", Application.m_modelia.m_datas_interface.proximite_bordure_Ybasse);
    m_application->m_data_center->write("DetectionObstacle.proximite_bordure_Yhaute", Application.m_modelia.m_datas_interface.proximite_bordure_Yhaute);
    m_application->m_data_center->write("DetectionObstacle.inhibe_detection_AV", Application.m_modelia.m_datas_interface.inhibe_detection_AV);
    m_application->m_data_center->write("DetectionObstacle.inhibe_detection_AR", Application.m_modelia.m_datas_interface.inhibe_detection_AR);

    m_application->m_data_center->write("convergence_conf", Application.m_asservissement.convergence_conf);
    m_application->m_data_center->write("convergence_rapide", Application.m_asservissement.convergence_rapide);

    //pour le moteur physique de SimuBot
    if(m_step>old_step)
    {
        m_application->m_data_center->write("Simulia.vect_G", Application.m_roues.m_vect_deplacement_G);
        m_application->m_data_center->write("Simulia.vect_D", Application.m_roues.m_vect_deplacement_D);
        m_application->m_data_center->write("Simulia.x_pos", Application.m_asservissement.X_robot);
        m_application->m_data_center->write("Simulia.y_pos", Application.m_asservissement.Y_robot);
        m_application->m_data_center->write("Simulia.step", m_step);
    }

    m_application->m_data_center->write("consigne_vitesse_avance", Application.m_asservissement.consigne_vitesse_avance);
}

// L'objectif de cette fonction est de reprendre les mêmes rapports de temps
// que sur le robot pour que la simulation soit la plus représentative
void CSimulia::simu_task_sequencer()
{
    static unsigned int cpt10msec = 0;
    static unsigned int cpt20msec = 0;
    static unsigned int cpt50msec = 0;
    static unsigned int cpt100msec = 0;
    static unsigned int cpt200msec = 0;
    static unsigned int cpt500msec = 0;
    static unsigned int cpt1sec = 0;

    Application.m_servos_ax.simu();
    Application.m_servos_sd20.simu();
    Application.m_asservissement_chariot.simu();
    // ______________________________
    cpt10msec++;
    if (cpt10msec >= 1) {
        cpt10msec = 0;
    }

    // ______________________________
    cpt20msec++;
    if (cpt20msec >= 2) {
        cpt20msec = 0;

        Application.m_roues.step_model();   // exécute un pas de calcul du simulateur de moteurs
        Application.m_asservissement.CalculsMouvementsRobots();
        m_step++;
        Application.m_asservissement_chariot.Asser_chariot();

        // Execute un pas de calcul du modele
        Application.m_modelia.step();
    }
    // ______________________________
    cpt50msec++;
    if (cpt50msec >= 5) {
        cpt50msec = 0;

        Application.m_messenger_xbee_ntw.execute();
        Application.m_leds.compute();

    }
    // ______________________________
    cpt100msec++;
    if (cpt100msec >= 10) {
        cpt100msec = 0;
    }
    // ______________________________
    cpt200msec++;
    if (cpt200msec >= 20) {
        cpt200msec = 0;

        Application.m_capteurs.Traitement();
        Application.m_kmar.compute();
    }
    // ______________________________
    cpt500msec++;
    if (cpt500msec >= 50) {
        cpt500msec = 0;
    }
    // ______________________________
    cpt1sec++;
    if (cpt1sec >= 100) {
        cpt1sec = 0;
    }
}


void CSimulia::on_speed_simu_valueChanged(int val)
{
    m_timer.setInterval(val);
}

void CSimulia::on_dde_autotest_pressed()
{
    Application.m_modelia.m_inputs_interface.dde_test_actionneurs = 1;
}

void CSimulia::on_config_debugger_changed()
{
    Application.m_modelia.m_sm_debug->m_active_start = m_ihm.ui.actionActive_Start->isChecked();
    Application.m_modelia.m_sm_debug->m_active_stop = m_ihm.ui.actionActive_Stop->isChecked();
    Application.m_modelia.m_sm_debug->m_active_on_entry = m_ihm.ui.actionActive_onEntry->isChecked();
    Application.m_modelia.m_sm_debug->m_active_on_exit = m_ihm.ui.actionActive_onExit->isChecked();
    Application.m_modelia.m_sm_debug->m_active_interrupt_evitement = m_ihm.ui.actionActive_Interrupt_Evitement->isChecked();
}


// ___________________________________________________
// Les données télémètres peuvent venir de plusieurs sources
// en fonction des besoins de simulation
void CSimulia::on_telemetres_gui_changed()
{
    Application.m_telemetres.setDistancesFromGui(
                m_ihm.ui.Telemetre_AVG->value(),
                m_ihm.ui.Telemetre_AVD->value(),
                m_ihm.ui.Telemetre_ARG->value(),
                m_ihm.ui.Telemetre_ARD->value());
}

//
void CSimulia::on_origine_telemetre_changed()
{
    bool read_only = true;
    if (m_ihm.ui.OrigineTelemetres->currentIndex() == CTelemetresSimu::TELEMETRES_FROM_GUI) {
        read_only = false;
        // Met des valeurs neutres au changement de source
        float val_neutre= 1000;
        m_ihm.ui.Telemetre_AVG->setValue(val_neutre);
        m_ihm.ui.Telemetre_AVD->setValue(val_neutre);
        m_ihm.ui.Telemetre_ARG->setValue(val_neutre);
        m_ihm.ui.Telemetre_ARD->setValue(val_neutre);
        on_telemetres_gui_changed();
    }
    m_ihm.ui.Telemetre_AVG->setReadOnly(read_only);
    m_ihm.ui.Telemetre_AVD->setReadOnly(read_only);
    m_ihm.ui.Telemetre_ARG->setReadOnly(read_only);
    m_ihm.ui.Telemetre_ARD->setReadOnly(read_only);
    Application.m_telemetres.setOrigineTelemetre(m_ihm.ui.OrigineTelemetres->currentIndex());
}

// ___________________________________________________
void CSimulia::on_detect_obstacle_gui_changed()
{
    Application.m_detection_obstacles.setObstacleDetecteFromGui(
                m_ihm.ui.detectionObstacle_AVG->isChecked(),
                m_ihm.ui.detectionObstacle_AVD->isChecked(),
                m_ihm.ui.detectionObstacle_ARG->isChecked(),
                m_ihm.ui.detectionObstacle_ARD->isChecked());
}

void CSimulia::on_origine_detect_obstacle_changed()
{
    bool enabled = false;
    if (m_ihm.ui.OrigineDetectionObstacles->currentIndex() == CDetectionObstaclesSimu::OBSTACLE_FROM_GUI) {
        enabled = true;
        // Met des valeurs neutres au changement de source
        m_ihm.ui.detectionObstacle_AVG->setChecked(false);
        m_ihm.ui.detectionObstacle_AVD->setChecked(false);
        m_ihm.ui.detectionObstacle_ARG->setChecked(false);
        m_ihm.ui.detectionObstacle_ARD->setChecked(false);
        on_detect_obstacle_gui_changed();
    }
    m_ihm.ui.detectionObstacle_AVG->setEnabled(enabled);
    m_ihm.ui.detectionObstacle_AVD->setEnabled(enabled);
    m_ihm.ui.detectionObstacle_ARG->setEnabled(enabled);
    m_ihm.ui.detectionObstacle_ARD->setEnabled(enabled);
    Application.m_detection_obstacles.setOrigineDetection(m_ihm.ui.OrigineDetectionObstacles->currentIndex());
}

// ___________________________________________________
// Quand Simubot est en mode "Test", les coordonnées du robot
//  sur le terrain sont envoyées dans les 3 datas PosX_robot, ...
// Met en cohérence l'asservissement avec la position sur le terrain dans ce mode
void CSimulia::updatePositionFromSimubot()
{
    float x = m_application->m_data_center->read("PosX_robot").toFloat();
    float y = m_application->m_data_center->read("PosY_robot").toFloat();
    float teta = m_application->m_data_center->read("PosTeta_robot").toFloat();
    Application.m_asservissement.setPosition_XYTeta(x, y, teta);
}


void CSimulia::updateStepFromSimuBot()
{
    if(m_application->m_data_center->read("Simubot.step").toInt()>=m_step)
    {
        int steps_G = m_application->m_data_center->read("Simubot.codeur_G").toInt();
        int steps_D = m_application->m_data_center->read("Simubot.codeur_D").toInt();
        Application.m_roues.addSteps_Codeur_G(steps_G);
        Application.m_roues.addSteps_Codeur_D(steps_D);
    }
}

// ___________________________________________________
void CSimulia::on_pb_kmar_mouvement_init()
{
    Application.m_kmar.start(1);
}

// ___________________________________________________
void CSimulia::on_pb_kmar_mouvement_ramasse()
{
    Application.m_kmar.start(2);
}

// ___________________________________________________
void CSimulia::on_select_couleur_equipe(int val)
{
    Application.m_modelia.m_inputs_interface.dde_couleur_equipe = val;
}
// ___________________________________________________
void CSimulia::on_select_strategie_match(int val)
{
    Application.m_modelia.setStrategie(val);
}



// ==================================================================
//                              LIDAR
// ==================================================================

// _____________________________________________________________________
// Cree la table et met a jour l'IHM en coherence avec la structure simulee
void CSimulia::init_lidar()
{
    Application.m_lidar.initGUI(m_ihm.ui.lidar_table_obstacles, m_ihm.ui.lidar_status);

    // capte les changements de valeurs sur le tableau et autres elements du LIDAR
    connect(m_ihm.ui.lidar_table_obstacles, SIGNAL(cellChanged(int,int)), this, SLOT(on_lidar_gui_changed()));
    connect(m_ihm.ui.lidar_RAZ_obstacles, SIGNAL(clicked(bool)), this, SLOT(on_raz_lidar_values()));
    connect(m_ihm.ui.lidar_status, SIGNAL(currentIndexChanged(int)), &Application.m_lidar, SLOT(setStatus(int)));
}

// ___________________________________________________
// Changement de valeursur le tableau des obstacles
void CSimulia::on_lidar_gui_changed()
{
    Application.m_lidar.setObstacles(m_ihm.ui.lidar_table_obstacles);
}

// ___________________________________________________
void CSimulia::on_origine_lidar_changed()
{
    Application.m_lidar.setOrigineLidar(m_ihm.ui.OrigineLidar->currentIndex());
    bool enabled = false;
    if (Application.m_lidar.m_origine_lidar == CLidarSimu::LIDAR_FROM_GUI) {
        enabled = true;
    }

    m_ihm.ui.lidar_table_obstacles->setEnabled(enabled);
    m_ihm.ui.lidar_RAZ_obstacles->setEnabled(enabled);
    m_ihm.ui.label_lidar_status->setEnabled(enabled);
    m_ihm.ui.lidar_status->setEnabled(enabled);
}

// ___________________________________________________
// Remet tous les obstacles simules aux valeurs par defaut sur l'IHM
void CSimulia::on_raz_lidar_values()
{
    disconnect(m_ihm.ui.lidar_table_obstacles, SIGNAL(cellChanged(int,int)), this, SLOT(on_lidar_gui_changed()));
    Application.m_lidar.Init();
    Application.m_lidar.refreshGUI(m_ihm.ui.lidar_table_obstacles, m_ihm.ui.lidar_status);
    connect(m_ihm.ui.lidar_table_obstacles, SIGNAL(cellChanged(int,int)), this, SLOT(on_lidar_gui_changed()));
}
