/*! \file CSimulia.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 *
 * POC hot-reload : CSimulia est desormais un DISPATCHER. Il ne contient plus la logique
 * robot (partie dans librobotlogic_*.so) ; il charge le plugin via QLibrary (RTLD_LOCAL),
 * tient un IRobotLogic* et pilote la simulation a travers cette interface. Cf. CLAUDE.md
 * a la racine du clone GROSBOT_STM32_test_simulia.
 */
#include <QDebug>
#include <QDir>
#include <QMenu>
#include <QTableWidgetItem>
#include "CSimulia.h"
#include "CSimuBot.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"

// Headers _simu inclus UNIQUEMENT pour leurs enums (LIDAR_FROM_GUI, TELEMETRES_FROM_*, ...).
// Aucune methode de ces classes n'est appelee depuis le shell : tout passe par IRobotLogic.
#include "CLidar_simu.h"
#include "CTelemetres_simu.h"
#include "CDetectionObstacles_simu.h"

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
    , m_logic(nullptr)
    , m_createFn(nullptr)
    , m_destroyFn(nullptr)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CSimulia::~CSimulia()
{
    unloadRobotLogic();
}


// _____________________________________________________________________
//                     POC hot-reload : loader du plugin
// _____________________________________________________________________

// Recherche la librobotlogic_*.so la plus recente dans un repertoire.
// Le tri par nom == tri par date grace a l'horodatage AAAAMMJJ_HHMMSS du TARGET.
QString CSimulia::findLatestLib(const QString& dir) const
{
    QDir d(dir);
    QStringList filters;
    filters << "librobotlogic_*.so";
    QStringList libs = d.entryList(filters, QDir::Files, QDir::Name);
    if (libs.isEmpty()) return QString();
    return d.absoluteFilePath(libs.last());
}

// Charge la bibliotheque en RTLD_LOCAL (isolation, cf. spike), resout la factory ABI C
// et cree l'objet de logique robot. AUCUN ExportExternalSymbolsHint (interdirait le reload).
bool CSimulia::loadRobotLogic(const QString& lib_path)
{
    m_lib.setFileName(lib_path);
    // Volontairement PAS de m_lib.setLoadHints(QLibrary::ExportExternalSymbolsHint) :
    // on veut RTLD_LOCAL pour que unload/reload d'une nouvelle version soit effectif.
    if (!m_lib.load()) {
        qWarning() << "[POC hot-reload] echec load:" << lib_path << m_lib.errorString();
        return false;
    }
    m_createFn  = (createRobotLogicFn)  m_lib.resolve("createRobotLogic");
    m_destroyFn = (destroyRobotLogicFn) m_lib.resolve("destroyRobotLogic");
    if (!m_createFn || !m_destroyFn) {
        qWarning() << "[POC hot-reload] symboles createRobotLogic/destroyRobotLogic introuvables";
        m_lib.unload();
        return false;
    }
    m_logic = m_createFn();
    m_current_lib = lib_path;
    qDebug() << "[POC hot-reload] logique robot chargee:" << lib_path;
    return (m_logic != nullptr);
}

// Detruit l'objet AVANT de decharger la lib (ordre du POC), puis unload.
void CSimulia::unloadRobotLogic()
{
    if (m_logic && m_destroyFn) {
        m_destroyFn(m_logic);
    }
    m_logic = nullptr;
    if (m_lib.isLoaded()) {
        m_lib.unload();
    }
}

// Rejoue toute la sequence d'init DEPENDANTE de l'objet logique (appelee a l'init ET au reload).
// Ne rejoue AUCUN geste one-shot (combos addItem, connects shell<->shell) -> pas de doublon.
void CSimulia::wireRobotLogicToIhm()
{
    if (!m_logic) return;

    m_logic->installDefaultDebugger();
    m_logic->initSubsystems(m_application);

    // Re-pousse la configuration courante de l'IHM dans les nouveaux objets
    m_logic->setStrategie(m_ihm.ui.StrategieMatch->currentIndex());
    m_logic->setCouleurEquipe(m_ihm.ui.CouleurEquipe->currentIndex());
    m_logic->setOrigineTelemetre(m_ihm.ui.OrigineTelemetres->currentIndex());
    m_logic->setOrigineDetection(m_ihm.ui.OrigineDetectionObstacles->currentIndex());

    // Lidar : partie plugin (GUI + connect interne combo->setStatus), re-jouee a chaque reload.
    m_logic->lidarInitGui(m_ihm.ui.lidar_table_obstacles, m_ihm.ui.lidar_status);
    m_logic->setOrigineLidar(CLidarSimu::LIDAR_FROM_GUI);
    m_ihm.ui.OrigineLidar->setCurrentIndex(m_logic->lidarOrigine());
    on_origine_lidar_changed();

    // Mise en coherence de l'IHM avec l'etat interne du debugger
    m_ihm.ui.actionActive_Start->setChecked(m_logic->debug()->m_active_start);
    m_ihm.ui.actionActive_Stop->setChecked(m_logic->debug()->m_active_stop);
    m_ihm.ui.actionActive_onEntry->setChecked(m_logic->debug()->m_active_on_entry);
    m_ihm.ui.actionActive_onExit->setChecked(m_logic->debug()->m_active_on_exit);
    m_ihm.ui.actionActive_Interrupt_Evitement->setChecked(m_logic->debug()->m_active_interrupt_evitement);
}

// Recharge a chaud la logique robot (declenche depuis le menu contextuel).
// Reset complet de la simulation (pas de preservation d'etat, assume par le POC).
void CSimulia::on_reload_robot_logic()
{
    m_timer.stop();
    QString lib = findLatestLib(m_lib_dir);
    if (lib.isEmpty()) {
        qWarning() << "[POC hot-reload] aucune librobotlogic_*.so dans" << m_lib_dir;
        m_timer.start();
        return;
    }
    unloadRobotLogic();
    if (!loadRobotLogic(lib)) {
        qWarning() << "[POC hot-reload] reload echoue";
        return;
    }
    wireRobotLogicToIhm();
    m_logic->resetSimulation();
    m_step = 0;
    m_timer.start();
    qDebug() << "[POC hot-reload] logique robot rechargee a chaud:" << lib;
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

    // ---- POC hot-reload : charger la logique robot AVANT tout usage de m_logic ----
    m_lib_dir = m_application->m_eeprom->read(getName(), "robot_logic_lib_path", ".").toString();
    QString lib = findLatestLib(m_lib_dir);
    if (lib.isEmpty() || !loadRobotLogic(lib)) {
        qWarning() << "[POC hot-reload] AUCUNE logique robot chargee depuis" << m_lib_dir
                   << "-> Simulia demarre sans modele. Placer une librobotlogic_*.so dans ce repertoire"
                   << "(clef EEPROM [Simulia] robot_logic_lib_path).";
        return;
    }

    // ordre identique à l'enum CLidar_simu (origines fournies par le plugin)
    {
        QStringList origines;
        m_logic->lidarOrigines(origines);
        m_ihm.ui.OrigineLidar->addItems(origines);
    }
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
    // Restore le numéro de stratégie de match
    val = m_application->m_eeprom->read(getName(), "choix_strategie", QVariant(0));
    m_ihm.ui.StrategieMatch->setCurrentIndex(val.toInt());

    // Sequence d'init dependante de l'objet logique (debugger + 17 sous-systemes + lidar + sync IHM).
    // Factorisee dans wireRobotLogicToIhm() car rejouee a l'identique lors d'un hot-reload.
    wireRobotLogicToIhm();

    // Connects shell<->shell du lidar (one-shot, NE PAS rejouer au reload)
    init_lidar();

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
    connect(m_ihm.ui.Telemetre_ARG_Centre, SIGNAL(editingFinished()), this, SLOT(on_telemetres_gui_changed()));
    connect(m_ihm.ui.Telemetre_ARD_Centre, SIGNAL(editingFinished()), this, SLOT(on_telemetres_gui_changed()));

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

    // POC hot-reload : hook de TEST headless (gate par variable d'environnement, off par defaut).
    // Permet de valider le rechargement a chaud sans interaction GUI (offscreen/CI). Inoffensif en
    // usage normal : le reload reste declenche par le menu contextuel. A retirer si le POC est adopte.
    if (qEnvironmentVariableIsSet("SIMULIA_POC_AUTORELOAD")) {
        int delay_ms = qEnvironmentVariableIntValue("SIMULIA_POC_AUTORELOAD");
        if (delay_ms <= 0) delay_ms = 5000;
        qDebug() << "[POC hot-reload] auto-reload de TEST programme dans" << delay_ms << "ms";
        QTimer::singleShot(delay_ms, this, SLOT(on_reload_robot_logic()));
    }
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
    // POC hot-reload : rechargement a chaud de la logique robot
    menu->addAction("Recharger la logique robot (hot-reload)", this, SLOT(on_reload_robot_logic()));
    menu->exec(m_ihm.mapToGlobal(pos));
}


void CSimulia::on_pb_active_main()
{
    if (m_logic) m_logic->startMain();
}

void CSimulia::on_pb_stop_all()
{
    if (m_logic) m_logic->stopAll();
    m_timer.stop();
}

void CSimulia::on_pb_init_all()
{
    m_timer.stop();

    // Reset complet de la simulation (subsystems + modele), execute cote plugin.
    if (m_logic) {
        m_logic->resetSimulation();
        m_logic->setStrategie(m_ihm.ui.StrategieMatch->currentIndex());
        m_logic->setCouleurEquipe(m_ihm.ui.CouleurEquipe->currentIndex());
    }

    // Remet la tirette en position
    m_ihm.ui.Tirette->setChecked(false);

    m_step=0;

    m_timer.start();
}

void CSimulia::on_timeout()
{
    step_sequencer();
}

void CSimulia::step_sequencer()
{
    if (!m_logic) return;

    // Mise en cohérence de l'IHM de simu avec le reste du modèle (lecture des champs via inputs())
    SM_InputsInterface* in = m_logic->inputs();
    m_ihm.ui.led_isObstacle->setValue(in->obstacleDetecte); // La LED représente une détection d'obstacle confirmée
    if (m_ihm.ui.OrigineDetectionObstacles->currentIndex() != CDetectionObstaclesSimu::OBSTACLE_FROM_GUI) {
        m_ihm.ui.detectionObstacle_AVG->setChecked(in->obstacle_AVG);
        m_ihm.ui.detectionObstacle_AVD->setChecked(in->obstacle_AVD);
        m_ihm.ui.detectionObstacle_ARG->setChecked(in->obstacle_ARG);
        m_ihm.ui.detectionObstacle_ARD->setChecked(in->obstacle_ARD);
    }

    if (m_ihm.ui.OrigineTelemetres->currentIndex() != CTelemetresSimu::TELEMETRES_FROM_GUI) {
        // Mise à jour des valeurs télémètres en fonction de leur provenance
        m_ihm.ui.Telemetre_AVG->setValue(in->Telemetre_AVG);
        m_ihm.ui.Telemetre_AVD->setValue(in->Telemetre_AVD);
        m_ihm.ui.Telemetre_ARG->setValue(in->Telemetre_ARG);
        m_ihm.ui.Telemetre_ARD->setValue(in->Telemetre_ARD);
        m_ihm.ui.Telemetre_ARG_Centre->setValue(in->Telemetre_ARGCentre);
        m_ihm.ui.Telemetre_ARD_Centre->setValue(in->Telemetre_ARDCentre);
    }

    // Courant global : entree IHM -> modele
    m_logic->setGlobalCurrent(m_ihm.ui.power_electrobot_global_current->value());

    // Un pas de simulation (lidar->inputs + ordonnanceur multi-cadences + publications DataManager)
    m_logic->step();
    m_step = m_logic->currentStep();
}


void CSimulia::on_speed_simu_valueChanged(int val)
{
    m_timer.setInterval(val);
}

void CSimulia::on_dde_autotest_pressed()
{
    if (m_logic) m_logic->setTestActionneurs(1);
}

void CSimulia::on_config_debugger_changed()
{
    if (!m_logic) return;
    m_logic->setDebugFlags(
        m_ihm.ui.actionActive_Start->isChecked(),
        m_ihm.ui.actionActive_Stop->isChecked(),
        m_ihm.ui.actionActive_onEntry->isChecked(),
        m_ihm.ui.actionActive_onExit->isChecked(),
        m_ihm.ui.actionActive_Interrupt_Evitement->isChecked());
}


// ___________________________________________________
// Les données télémètres peuvent venir de plusieurs sources
// en fonction des besoins de simulation
void CSimulia::on_telemetres_gui_changed()
{
    if (!m_logic) return;
    m_logic->setTelemetresFromGui(
                m_ihm.ui.Telemetre_AVG->value(),
                m_ihm.ui.Telemetre_AVD->value(),
                m_ihm.ui.Telemetre_ARG->value(),
                m_ihm.ui.Telemetre_ARD->value(),
                m_ihm.ui.Telemetre_ARG_Centre->value(),
                m_ihm.ui.Telemetre_ARD_Centre->value());
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
        m_ihm.ui.Telemetre_ARG_Centre->setValue(val_neutre);
        m_ihm.ui.Telemetre_ARD_Centre->setValue(val_neutre);
        on_telemetres_gui_changed();
    }
    m_ihm.ui.Telemetre_AVG->setReadOnly(read_only);
    m_ihm.ui.Telemetre_AVD->setReadOnly(read_only);
    m_ihm.ui.Telemetre_ARG->setReadOnly(read_only);
    m_ihm.ui.Telemetre_ARD->setReadOnly(read_only);
    m_ihm.ui.Telemetre_ARG_Centre->setReadOnly(read_only);
    m_ihm.ui.Telemetre_ARD_Centre->setReadOnly(read_only);
    if (m_logic) m_logic->setOrigineTelemetre(m_ihm.ui.OrigineTelemetres->currentIndex());
}

// ___________________________________________________
void CSimulia::on_detect_obstacle_gui_changed()
{
    if (!m_logic) return;
    m_logic->setDetectionFromGui(
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
    if (m_logic) m_logic->setOrigineDetection(m_ihm.ui.OrigineDetectionObstacles->currentIndex());
}

// ___________________________________________________
// Quand Simubot est en mode "Test", les coordonnées du robot
//  sur le terrain sont envoyées dans les 3 datas PosX_robot, ...
// Met en cohérence l'asservissement avec la position sur le terrain dans ce mode
void CSimulia::updatePositionFromSimubot()
{
    if (!m_logic) return;
    float x = m_application->m_data_center->read("PosX_robot").toFloat();
    float y = m_application->m_data_center->read("PosY_robot").toFloat();
    float teta = m_application->m_data_center->read("PosTeta_robot").toFloat();
    m_logic->setPositionXYTeta(x, y, teta);
}


void CSimulia::updateStepFromSimuBot()
{
    if (!m_logic) return;
    if(m_application->m_data_center->read("Simubot.step").toInt()>=m_step)
    {
        int steps_G = m_application->m_data_center->read("Simubot.codeur_G").toInt();
        int steps_D = m_application->m_data_center->read("Simubot.codeur_D").toInt();
        m_logic->addCodeurSteps(steps_G, steps_D);
    }
}

// ___________________________________________________
void CSimulia::on_pb_kmar_mouvement_init()
{
    //Application.m_kmar.start(1);
}

// ___________________________________________________
void CSimulia::on_pb_kmar_mouvement_ramasse()
{
    //Application.m_kmar.start(2);
}

// ___________________________________________________
void CSimulia::on_select_couleur_equipe(int val)
{
    if (m_logic) m_logic->setCouleurEquipe(val);
}
// ___________________________________________________
void CSimulia::on_select_strategie_match(int val)
{
    if (m_logic) m_logic->setStrategie(val);
}



// ==================================================================
//                              LIDAR
// ==================================================================

// _____________________________________________________________________
// Connects shell<->shell du lidar (one-shot). La partie plugin (initGUI + connect
// interne combo->setStatus) est faite dans wireRobotLogicToIhm (rejouee au reload).
void CSimulia::init_lidar()
{
    // capte les changements de valeurs sur le tableau et autres elements du LIDAR
    connect(m_ihm.ui.lidar_table_obstacles, SIGNAL(cellChanged(int,int)), this, SLOT(on_lidar_gui_changed()));
    connect(m_ihm.ui.lidar_RAZ_obstacles, SIGNAL(clicked(bool)), this, SLOT(on_raz_lidar_values()));
}

// ___________________________________________________
// Changement de valeursur le tableau des obstacles
void CSimulia::on_lidar_gui_changed()
{
    if (m_logic) m_logic->lidarSetObstaclesFromGui(m_ihm.ui.lidar_table_obstacles);
}

// ___________________________________________________
void CSimulia::on_origine_lidar_changed()
{
    if (!m_logic) return;
    m_logic->setOrigineLidar(m_ihm.ui.OrigineLidar->currentIndex());
    bool enabled = false;
    if (m_logic->lidarOrigine() == CLidarSimu::LIDAR_FROM_GUI) {
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
    if (!m_logic) return;
    disconnect(m_ihm.ui.lidar_table_obstacles, SIGNAL(cellChanged(int,int)), this, SLOT(on_lidar_gui_changed()));
    m_logic->lidarReset();
    m_logic->lidarRefreshGui(m_ihm.ui.lidar_table_obstacles, m_ihm.ui.lidar_status);
    connect(m_ihm.ui.lidar_table_obstacles, SIGNAL(cellChanged(int,int)), this, SLOT(on_lidar_gui_changed()));
}
