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

  // Mise en cohérence de l'IHM avec l'état interne
  m_ihm.ui.actionActive_Start->setChecked(m_ia.m_sm_debug->m_active_start);
  m_ihm.ui.actionActive_Stop->setChecked(m_ia.m_sm_debug->m_active_stop);
  m_ihm.ui.actionActive_onEntry->setChecked(m_ia.m_sm_debug->m_active_on_entry);
  m_ihm.ui.actionActive_onExit->setChecked(m_ia.m_sm_debug->m_active_on_exit);
  m_ihm.ui.actionActive_Interrupt_Evitement->setChecked(m_ia.m_sm_debug->m_active_interrupt_evitement);

  connect(m_ihm.ui.pb_start_main, SIGNAL(pressed()), this, SLOT(on_pb_active_main()));
  connect(m_ihm.ui.pb_stop_all, SIGNAL(pressed()), this, SLOT(on_pb_stop_all()));
  connect(m_ihm.ui.pb_init_all, SIGNAL(pressed()), this, SLOT(on_pb_init_all()));
  connect(m_ihm.ui.speed_simu, SIGNAL(valueChanged(int)), this, SLOT(on_speed_simu_valueChanged(int)));
  connect(m_ihm.ui.dde_autotest, SIGNAL(pressed()), this, SLOT(on_dde_autotest_pressed()));

  connect(&m_timer, SIGNAL(timeout()), this, SLOT(on_timeout()));
  connect(m_ihm.ui.actionActive_Start, SIGNAL(changed()), this, SLOT(on_config_debugger_changed()));
  connect(m_ihm.ui.actionActive_Stop, SIGNAL(changed()), this, SLOT(on_config_debugger_changed()));
  connect(m_ihm.ui.actionActive_onEntry, SIGNAL(changed()), this, SLOT(on_config_debugger_changed()));
  connect(m_ihm.ui.actionActive_onExit, SIGNAL(changed()), this, SLOT(on_config_debugger_changed()));
  connect(m_ihm.ui.actionActive_Interrupt_Evitement, SIGNAL(changed()), this, SLOT(on_config_debugger_changed()));


  m_timer.start(100);
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
}

void CSimulia::on_pb_init_all()
{
    m_ia.initAllStateMachines();

    m_ia.m_sm_recup_bouees_distributeur.setPrioriteExecution(0);
    m_ia.m_sm_activer_phare.setPrioriteExecution(1);
    m_ia.m_inputs_interface.TE_Modele = 0.02f;
}

void CSimulia::on_timeout()
{
    // IHM -> Inputs
    m_ia.m_inputs_interface.Tirette             = m_ihm.ui.Tirette->isChecked();
    m_ia.m_inputs_interface.obstacle_AVG        = m_ihm.ui.Obstacle_AVG->value();
    m_ia.m_inputs_interface.obstacle_AVD        = m_ihm.ui.Obstacle_AVD->value();
    m_ia.m_inputs_interface.obstacle_ARG        = m_ihm.ui.Obstacle_ARG->value();
    m_ia.m_inputs_interface.obstacle_ARD        = m_ihm.ui.Obstacle_ARD->value();
    Application.m_power_electrobot.simuSetGlobalCurrent(m_ihm.ui.power_electrobot_global_current->value());

    // Step
    m_ia.step();

    // Outputs -> IHM
    m_ihm.ui.TempsMatch->setValue(m_ia.m_datas_interface.TempsMatch);
    m_ihm.ui.power_electrobot_sw_1->setValue(Application.m_power_electrobot.getOutputPort()&0x01);
    m_ihm.ui.power_electrobot_sw_2->setValue(Application.m_power_electrobot.getOutputPort()&0x02);
    m_ihm.ui.power_electrobot_sw_3->setValue(Application.m_power_electrobot.getOutputPort()&0x04);
    m_ihm.ui.power_electrobot_sw_4->setValue(Application.m_power_electrobot.getOutputPort()&0x08);
    m_ihm.ui.power_electrobot_sw_5->setValue(Application.m_power_electrobot.getOutputPort()&0x10);
    m_ihm.ui.power_electrobot_sw_6->setValue(Application.m_power_electrobot.getOutputPort()&0x20);
    m_ihm.ui.power_electrobot_sw_7->setValue(Application.m_power_electrobot.getOutputPort()&0x40);
    m_ihm.ui.power_electrobot_sw_8->setValue(Application.m_power_electrobot.getOutputPort()&0x80);
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

