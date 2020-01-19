/*! \file CSimulia.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CSimulia.h"
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

  m_ia.setDebugger(new SM_DebugQDebug());

  // passe à toutes les classes de simulation l'application Simulia
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


  // Mise en cohérence de l'IHM avec l'état interne
  m_ihm.ui.actionActive_Start->setChecked(m_ia.m_sm_debug->m_active_start);
  m_ihm.ui.actionActive_Stop->setChecked(m_ia.m_sm_debug->m_active_stop);
  m_ihm.ui.actionActive_onEntry->setChecked(m_ia.m_sm_debug->m_active_on_entry);
  m_ihm.ui.actionActive_onExit->setChecked(m_ia.m_sm_debug->m_active_on_exit);
  m_ihm.ui.actionActive_Interrupt_Evitement->setChecked(m_ia.m_sm_debug->m_active_interrupt_evitement);

  connect(m_ihm.ui.pb_start_main, SIGNAL(pressed()), this, SLOT(on_pb_active_main()));
  connect(m_ihm.ui.pb_stop_all, SIGNAL(pressed()), this, SLOT(on_pb_stop_all()));
  connect(m_ihm.ui.pb_init_all, SIGNAL(pressed()), this, SLOT(on_pb_init_all()));
  connect(m_ihm.ui.dde_autotest, SIGNAL(pressed()), this, SLOT(on_dde_autotest_pressed()));

  connect(&m_timer, SIGNAL(timeout()), this, SLOT(on_timeout()));
  connect(m_ihm.ui.PlaySimu, SIGNAL(clicked(bool)), &m_timer, SLOT(start()));
  connect(m_ihm.ui.PauseSimu, SIGNAL(clicked(bool)), &m_timer, SLOT(stop()));
  connect(m_ihm.ui.StepSimu, SIGNAL(clicked(bool)), this, SLOT(step()));
  connect(m_ihm.ui.speed_simu, SIGNAL(valueChanged(int)), this, SLOT(on_speed_simu_valueChanged(int)));

  connect(m_ihm.ui.actionActive_Start, SIGNAL(changed()), this, SLOT(on_config_debugger_changed()));
  connect(m_ihm.ui.actionActive_Stop, SIGNAL(changed()), this, SLOT(on_config_debugger_changed()));
  connect(m_ihm.ui.actionActive_onEntry, SIGNAL(changed()), this, SLOT(on_config_debugger_changed()));
  connect(m_ihm.ui.actionActive_onExit, SIGNAL(changed()), this, SLOT(on_config_debugger_changed()));
  connect(m_ihm.ui.actionActive_Interrupt_Evitement, SIGNAL(changed()), this, SLOT(on_config_debugger_changed()));

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
    m_ia.m_sm_main.start();
}

void CSimulia::on_pb_stop_all()
{
  m_ia.stopAllStateMachines();
  m_timer.stop();
}

void CSimulia::on_pb_init_all()
{
    m_timer.stop();

    // initialise l'asservissement et le modèle simulink des moteurs
    Application.m_asservissement.CommandeManuelle(0, 0);
    Application.m_asservissement.Init();
    Application.m_roues.init_model();
    Application.m_servos_sd20.Init();
    Application.m_servos_ax.Init();
    Application.m_leds.setState(ALL_LED, 0);

    // initialise les machines d'états Modelia du robot
    m_ia.initAllStateMachines();

    m_ia.m_sm_recup_bouees_distributeur.setPrioriteExecution(0);
    m_ia.m_sm_activer_phare.setPrioriteExecution(1);
    m_ia.m_inputs_interface.TE_Modele = 0.02f;

    m_timer.start();
}

void CSimulia::on_timeout()
{
    step();
}

void CSimulia::step()
{
    // TODO :
    //  peut être à revoir l'interface avec le monde extérieur
    //  passer par les variables du DataManager
    // Pour l'asservissement, donner la possibilité d'alimenter le modèle
    //      - Soit à partir de Application.m_asservissement.xxxxx
    //      - Soit à partir de données du DataManger (de l'IHM)

    // IHM -> Inputs
    m_ia.m_inputs_interface.Tirette             = m_ihm.ui.Tirette->isChecked();
    m_ia.m_inputs_interface.obstacle_AVG        = m_ihm.ui.Obstacle_AVG->value();
    m_ia.m_inputs_interface.obstacle_AVD        = m_ihm.ui.Obstacle_AVD->value();
    m_ia.m_inputs_interface.obstacle_ARG        = m_ihm.ui.Obstacle_ARG->value();
    m_ia.m_inputs_interface.obstacle_ARD        = m_ihm.ui.Obstacle_ARD->value();
    m_ia.m_inputs_interface.obstacleDetecte     = m_ia.m_inputs_interface.obstacle_ARG || m_ia.m_inputs_interface.obstacle_ARD || m_ia.m_inputs_interface.obstacle_AVG || m_ia.m_inputs_interface.obstacle_AVD;
    m_ia.m_inputs_interface.Convergence         = Application.m_asservissement.convergence_conf;
    m_ia.m_inputs_interface.Convergence_rapide  = Application.m_asservissement.convergence_rapide;
    m_ia.m_inputs_interface.X_robot             = Application.m_asservissement.X_robot;
    m_ia.m_inputs_interface.Y_robot             = Application.m_asservissement.Y_robot;
    m_ia.m_inputs_interface.angle_robot         = Application.m_asservissement.angle_robot;
    Application.m_power_electrobot.simuSetGlobalCurrent(m_ihm.ui.power_electrobot_global_current->value());

    // Step
    simu_task_sequencer();

    // Outputs -> IHM
    m_application->m_data_center->write("TempsMatch", m_ia.m_datas_interface.TempsMatch);

    m_application->m_data_center->write("x_pos", Application.m_asservissement.X_robot);
    m_application->m_data_center->write("y_pos", Application.m_asservissement.Y_robot);
    m_application->m_data_center->write("teta_pos", Application.m_asservissement.angle_robot);
    m_application->m_data_center->write("convergence_conf", Application.m_asservissement.convergence_conf);
    m_application->m_data_center->write("convergence_rapide", Application.m_asservissement.convergence_rapide);

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

      // Execute un pas de calcul du modele
      m_ia.step();
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
    m_ia.m_inputs_interface.dde_test_actionneurs = 1;
}

void CSimulia::on_config_debugger_changed()
{
    m_ia.m_sm_debug->m_active_start = m_ihm.ui.actionActive_Start->isChecked();
    m_ia.m_sm_debug->m_active_stop = m_ihm.ui.actionActive_Stop->isChecked();
    m_ia.m_sm_debug->m_active_on_entry = m_ihm.ui.actionActive_onEntry->isChecked();
    m_ia.m_sm_debug->m_active_on_exit = m_ihm.ui.actionActive_onExit->isChecked();
    m_ia.m_sm_debug->m_active_interrupt_evitement = m_ihm.ui.actionActive_Interrupt_Evitement->isChecked();
}

