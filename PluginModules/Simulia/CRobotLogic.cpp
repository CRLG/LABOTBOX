/*! \file CRobotLogic.cpp
    \brief Implementation de CRobotLogic (cf. CRobotLogic.h).
*/
#include "CRobotLogic.h"
#include "CGlobale.h"          // CGlobaleSimule + global Application (interne au plugin)
#include "CApplication.h"
#include "CDataManager.h"
#include "sm_debugqdebug.h"
// ALL_LED et CLeds sont tires par CGlobale.h (common-rob/CLeds.h).

#include <QStringList>
#include <QTableWidget>
#include <QComboBox>
#include <QDebug>

// _____________________________________________________________________
CRobotLogic::CRobotLogic()
    : m_application(nullptr)
    , m_step(0)
    , m_cpt10msec(0), m_cpt20msec(0), m_cpt50msec(0), m_cpt100msec(0)
    , m_cpt200msec(0), m_cpt500msec(0), m_cpt1sec(0)
{
    // POC hot-reload : trace de construction du plugin. Sert de temoin visuel au reload
    // (a personnaliser entre deux builds pour verifier que le NOUVEAU code s'execute bien).
    qDebug("[CRobotLogic] instance de logique robot construite (plugin)");
}

CRobotLogic::~CRobotLogic()
{
}

// _____________________________________________________________________
// Cycle de vie
// _____________________________________________________________________
void CRobotLogic::initSubsystems(CApplication* application)
{
    m_application = application;
    // Ordre identique a l'ancien CSimulia::init (les ~17 Application.xxx.init(app)).
    Application.m_capteurs.init(application);
    Application.m_roues.init(application);
    Application.m_servos_ax.init(application);
    Application.m_asservissement.init(application);
    Application.m_servos_sd20.init(application);
    Application.m_servos.init(application);
    Application.m_led1.init(application);
    Application.m_led2.init(application);
    Application.m_led3.init(application);
    Application.m_led4.init(application);
    Application.m_power_electrobot.init(application);
    Application.m_messenger_xbee_ntw.init(application);
    Application.m_asservissement_chariot.init(application);
    Application.m_telemetres.init(application);
    Application.m_lidar.init(application);
    Application.m_detection_obstacles.init(application);
    Application.m_ascenseur.init(application);
}

void CRobotLogic::installDefaultDebugger()
{
    Application.m_modelia.setDebugger(new SM_DebugQDebug());
}

// Reset complet de la simulation (ancien CSimulia::on_pb_init_all, sans la partie timer/IHM).
void CRobotLogic::resetSimulation()
{
    Application.m_capteurs.Init();
    Application.m_asservissement.CommandeManuelle(0, 0);
    Application.m_asservissement.Init();
    Application.m_roues.init_model();
    Application.m_servos_sd20.Init();
    Application.m_servos.Init();
    Application.m_servos_ax.Init();
    Application.m_leds.setState(ALL_LED, 0);
    Application.m_asservissement_chariot.Init();
    Application.m_asservissement_chariot.Recal_Chariot();
    Application.m_telemetres.init();
    Application.m_lidar.Init();
    Application.m_detection_obstacles.Init();

    Application.m_modelia.init();

    // Synchronisation avec SimuBot (le plugin dispose du DataManager)
    if (m_application) {
        m_application->m_data_center->write("Capteurs.Tirette", false);
        m_application->m_data_center->write("Simubot.Init", true);
    }
    m_step = 0;
}

void CRobotLogic::setStrategie(int strategie)
{
    Application.m_modelia.setStrategie((unsigned char)strategie);
}

void CRobotLogic::setCouleurEquipe(int couleur)
{
    Application.m_modelia.m_inputs_interface.dde_couleur_equipe = couleur;
}

void CRobotLogic::startMain()
{
    Application.m_modelia.m_sm_main.start();
}

void CRobotLogic::stopAll()
{
    Application.m_modelia.stopAllStateMachines();
}

// _____________________________________________________________________
// Tick (boucle chaude)
// _____________________________________________________________________
void CRobotLogic::step()
{
    int old_step = m_step;

    // Met a jour les donnees LIDAR consommees par Modelia
    Application.m_modelia.m_inputs_interface.m_lidar_status = Application.m_lidar.getStatus();
    Application.m_lidar.getObstacles(Application.m_modelia.m_inputs_interface.m_lidar_obstacles);

    taskSequencer();
    publishOutputs(old_step);
}

// Ancien CSimulia::simu_task_sequencer (compteurs desormais membres -> reset au reload).
void CRobotLogic::taskSequencer()
{
    Application.m_servos_ax.simu();
    Application.m_servos_sd20.simu();
    Application.m_asservissement_chariot.simu();

    m_cpt10msec++;
    if (m_cpt10msec >= 1) {
        m_cpt10msec = 0;
    }

    m_cpt20msec++;
    if (m_cpt20msec >= 2) {
        m_cpt20msec = 0;
        Application.m_roues.step_model();
        Application.m_asservissement.CalculsMouvementsRobots();
        m_step++;
        Application.m_asservissement_chariot.Asser_chariot();
        Application.m_modelia.step();
        Application.m_servos.periodicCall();
    }

    m_cpt50msec++;
    if (m_cpt50msec >= 5) {
        m_cpt50msec = 0;
        Application.m_messenger_xbee_ntw.execute();
        Application.m_leds.compute();
    }

    m_cpt100msec++;
    if (m_cpt100msec >= 10) {
        m_cpt100msec = 0;
    }

    m_cpt200msec++;
    if (m_cpt200msec >= 20) {
        m_cpt200msec = 0;
        Application.m_capteurs.Traitement();
    }

    m_cpt500msec++;
    if (m_cpt500msec >= 50) {
        m_cpt500msec = 0;
    }

    m_cpt1sec++;
    if (m_cpt1sec >= 100) {
        m_cpt1sec = 0;
    }
}

// Ancien bas de CSimulia::step_sequencer : publication des sorties dans le DataManager.
void CRobotLogic::publishOutputs(int oldStep)
{
    if (!m_application) return;
    CDataManager* dm = m_application->m_data_center;

    dm->write("TempsMatch", Application.m_modelia.m_datas_interface.TempsMatch);

    dm->write("DetectionObstacle.proximite_bordure_Xdroite", Application.m_modelia.m_datas_interface.proximite_bordure_Xdroite);
    dm->write("DetectionObstacle.proximite_bordure_Xgauche", Application.m_modelia.m_datas_interface.proximite_bordure_Xgauche);
    dm->write("DetectionObstacle.proximite_bordure_Ybasse", Application.m_modelia.m_datas_interface.proximite_bordure_Ybasse);
    dm->write("DetectionObstacle.proximite_bordure_Yhaute", Application.m_modelia.m_datas_interface.proximite_bordure_Yhaute);
    dm->write("DetectionObstacle.inhibe_detection_AV", Application.m_modelia.m_datas_interface.inhibe_detection_AV);
    dm->write("DetectionObstacle.inhibe_detection_AR", Application.m_modelia.m_datas_interface.inhibe_detection_AR);

    dm->write("convergence_conf", Application.m_asservissement.convergence_conf);
    dm->write("convergence_rapide", Application.m_asservissement.convergence_rapide);

    // pour le moteur physique de SimuBot
    if (m_step > oldStep) {
        dm->write("Simulia.vect_G", Application.m_roues.m_vect_deplacement_G);
        dm->write("Simulia.vect_D", Application.m_roues.m_vect_deplacement_D);
        dm->write("Simulia.x_pos", Application.m_asservissement.X_robot);
        dm->write("Simulia.y_pos", Application.m_asservissement.Y_robot);
        dm->write("Simulia.teta_pos", Application.m_asservissement.angle_robot);
        dm->write("Simulia.step", m_step);
    }

    dm->write("consigne_vitesse_avance", Application.m_asservissement.consigne_vitesse_avance);
}

// _____________________________________________________________________
// Entrees IHM -> modele
// _____________________________________________________________________
void CRobotLogic::setTestActionneurs(int val)
{
    Application.m_modelia.m_inputs_interface.dde_test_actionneurs = val;
}

void CRobotLogic::setDebugFlags(bool start, bool stop, bool onEntry, bool onExit, bool interruptEvit)
{
    Application.m_modelia.m_sm_debug->m_active_start = start;
    Application.m_modelia.m_sm_debug->m_active_stop = stop;
    Application.m_modelia.m_sm_debug->m_active_on_entry = onEntry;
    Application.m_modelia.m_sm_debug->m_active_on_exit = onExit;
    Application.m_modelia.m_sm_debug->m_active_interrupt_evitement = interruptEvit;
}

void CRobotLogic::setTelemetresFromGui(float avg, float avd, float arg, float ard, float argCentre, float ardCentre)
{
    Application.m_telemetres.setDistancesFromGui(avg, avd, arg, ard, argCentre, ardCentre);
}

void CRobotLogic::setOrigineTelemetre(int origine)
{
    Application.m_telemetres.setOrigineTelemetre(origine);
}

void CRobotLogic::setDetectionFromGui(bool avg, bool avd, bool arg, bool ard)
{
    Application.m_detection_obstacles.setObstacleDetecteFromGui(avg, avd, arg, ard);
}

void CRobotLogic::setOrigineDetection(int origine)
{
    Application.m_detection_obstacles.setOrigineDetection(origine);
}

void CRobotLogic::setGlobalCurrent(float current)
{
    Application.m_power_electrobot.simuSetGlobalCurrent(current);
}

void CRobotLogic::setPositionXYTeta(float x, float y, float teta)
{
    Application.m_asservissement.setPosition_XYTeta(x, y, teta);
}

void CRobotLogic::addCodeurSteps(int steps_G, int steps_D)
{
    Application.m_roues.addSteps_Codeur_G(steps_G);
    Application.m_roues.addSteps_Codeur_D(steps_D);
}

// _____________________________________________________________________
// Lidar
// _____________________________________________________________________
void CRobotLogic::lidarInitGui(QTableWidget* table, QComboBox* statusCombo)
{
    Application.m_lidar.initGUI(table, statusCombo);
    // Le seul connect shell->plugin : place ici cote plugin pour etre re-etabli a chaque reload.
    QObject::connect(statusCombo, SIGNAL(currentIndexChanged(int)),
                     &Application.m_lidar, SLOT(setStatus(int)));
}

void CRobotLogic::lidarRefreshGui(QTableWidget* table, QComboBox* statusCombo)
{
    Application.m_lidar.refreshGUI(table, statusCombo);
}

void CRobotLogic::lidarSetObstaclesFromGui(QTableWidget* table)
{
    Application.m_lidar.setObstacles(table);
}

void CRobotLogic::lidarReset()
{
    Application.m_lidar.Init();
}

void CRobotLogic::setOrigineLidar(int origine)
{
    Application.m_lidar.setOrigineLidar(origine);
}

int CRobotLogic::lidarOrigine() const
{
    return Application.m_lidar.m_origine_lidar;
}

void CRobotLogic::lidarOrigines(QStringList& out) const
{
    out = Application.m_lidar.getOrigines();
}

// _____________________________________________________________________
// Acces donnees
// _____________________________________________________________________
SM_InputsInterface* CRobotLogic::inputs() { return &Application.m_modelia.m_inputs_interface; }
SM_DatasInterface*  CRobotLogic::datas()  { return &Application.m_modelia.m_datas_interface; }
SM_DebugInterface*  CRobotLogic::debug()  { return Application.m_modelia.m_sm_debug; }
