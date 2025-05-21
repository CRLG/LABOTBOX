/*! \file CActuatorElectrobot.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CActuatorElectrobot.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"



/*! \addtogroup Module_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CActuatorElectrobot::CActuatorElectrobot(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_ActuatorElectrobot, AUTEUR_ActuatorElectrobot, INFO_ActuatorElectrobot)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CActuatorElectrobot::~CActuatorElectrobot()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CActuatorElectrobot::init(CApplication *application)
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

  // _____________________________________________ Moteurs
  connect(m_ihm.ui.Moteurs_StopAll, SIGNAL(clicked()), this, SLOT(Moteurs_StopAll_clicked()));

  m_ihm.ui.Send_CdeMoteurs->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ihm.ui.Send_CdeMoteurs, SIGNAL(clicked()), this, SLOT(CdeMoteursSynchroSend_left_clic()));
  connect(m_ihm.ui.Send_CdeMoteurs, SIGNAL(customContextMenuRequested(QPoint)) , this, SLOT(CdeMoteursSynchroSend_right_clic(QPoint)));

  connect(m_ihm.ui.Moteur_1, SIGNAL(editingFinished()), this, SLOT(CdeMoteur1_changed()));
  connect(m_ihm.ui.Moteur_2, SIGNAL(editingFinished()), this, SLOT(CdeMoteur2_changed()));
  connect(m_ihm.ui.Moteur_3, SIGNAL(editingFinished()), this, SLOT(CdeMoteur3_changed()));
  connect(m_ihm.ui.Moteur_4, SIGNAL(editingFinished()), this, SLOT(CdeMoteur4_changed()));
  connect(m_ihm.ui.Moteur_5, SIGNAL(editingFinished()), this, SLOT(CdeMoteur5_changed()));
  connect(m_ihm.ui.Moteur_6, SIGNAL(editingFinished()), this, SLOT(CdeMoteur6_changed()));

  // Met à jour les tooltips des moteurs pour donner des informations pratiques
  // sur chaque moteur
  updateMotorsTooltips();

  // Met à jour les labels en fonction de la propriété "Alias" de chaque data
  // Si la data n'a pas d'Alias dans  le data_center,  le label reste celui par défaut
  updateMotorsAliasLabels();

  connect(&m_ihm, SIGNAL(keyPressed(int)), this, SLOT(keyPressed(int)));

  // _____________________________________________ Servo moteurs (classiques et SD20)
  m_choix_type_servos = cSERVOS_SD20;
  choix_type_servo_classique_sd20();
  connect(m_ihm.ui.choix_servos_classiques, SIGNAL(clicked()), this, SLOT(choix_type_servo_classique_sd20()));
  connect(m_ihm.ui.choix_servos_sd20, SIGNAL(clicked()), this, SLOT(choix_type_servo_classique_sd20()));

  connect(m_ihm.ui.Servo_num_1, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_1, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_2, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_2, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_3, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_3, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_4, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_4, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_5, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_5, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_6, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_6, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_7, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_7, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_8, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_8, SLOT(setValue(int)));

  connect(m_ihm.ui.Servo_slide_1, SIGNAL(sliderMoved(int)), m_ihm.ui.Servo_num_1, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_2, SIGNAL(sliderMoved(int)), m_ihm.ui.Servo_num_2, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_3, SIGNAL(sliderMoved(int)), m_ihm.ui.Servo_num_3, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_4, SIGNAL(sliderMoved(int)), m_ihm.ui.Servo_num_4, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_5, SIGNAL(sliderMoved(int)), m_ihm.ui.Servo_num_5, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_6, SIGNAL(sliderMoved(int)), m_ihm.ui.Servo_num_6, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_7, SIGNAL(sliderMoved(int)), m_ihm.ui.Servo_num_7, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_8, SIGNAL(sliderMoved(int)), m_ihm.ui.Servo_num_8, SLOT(setValue(int)));

  connect(m_ihm.ui.Servo_num_1, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteur1_changed()));
  connect(m_ihm.ui.Servo_num_2, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteur2_changed()));
  connect(m_ihm.ui.Servo_num_3, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteur3_changed()));
  connect(m_ihm.ui.Servo_num_4, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteur4_changed()));
  connect(m_ihm.ui.Servo_num_5, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteur5_changed()));
  connect(m_ihm.ui.Servo_num_6, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteur6_changed()));
  connect(m_ihm.ui.Servo_num_7, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteur7_changed()));
  connect(m_ihm.ui.Servo_num_8, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteur8_changed()));

  connect(m_ihm.ui.Servo_slide_1, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteur1_changed()));
  connect(m_ihm.ui.Servo_slide_2, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteur2_changed()));
  connect(m_ihm.ui.Servo_slide_3, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteur3_changed()));
  connect(m_ihm.ui.Servo_slide_4, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteur4_changed()));
  connect(m_ihm.ui.Servo_slide_5, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteur5_changed()));
  connect(m_ihm.ui.Servo_slide_6, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteur6_changed()));
  connect(m_ihm.ui.Servo_slide_7, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteur7_changed()));
  connect(m_ihm.ui.Servo_slide_8, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteur8_changed()));

  connect(m_ihm.ui.ServosConfig_Send, SIGNAL(clicked()), this, SLOT(ServosConfig_Send_clicked()));

  // met à jour la liste des commandes possibles sur les servos SD20
  initList_ActionsServos();

  // _____________________________________________ Servo moteurs AX
  connect(m_ihm.ui.ServoAX_num_0, SIGNAL(valueChanged(int)), m_ihm.ui.ServoAX_slide_0, SLOT(setValue(int)));
  connect(m_ihm.ui.ServoAX_num_1, SIGNAL(valueChanged(int)), m_ihm.ui.ServoAX_slide_1, SLOT(setValue(int)));
  connect(m_ihm.ui.ServoAX_num_2, SIGNAL(valueChanged(int)), m_ihm.ui.ServoAX_slide_2, SLOT(setValue(int)));
  connect(m_ihm.ui.ServoAX_num_3, SIGNAL(valueChanged(int)), m_ihm.ui.ServoAX_slide_3, SLOT(setValue(int)));
  connect(m_ihm.ui.ServoAX_num_4, SIGNAL(valueChanged(int)), m_ihm.ui.ServoAX_slide_4, SLOT(setValue(int)));
  connect(m_ihm.ui.ServoAX_num_5, SIGNAL(valueChanged(int)), m_ihm.ui.ServoAX_slide_5, SLOT(setValue(int)));
  connect(m_ihm.ui.ServoAX_num_6, SIGNAL(valueChanged(int)), m_ihm.ui.ServoAX_slide_6, SLOT(setValue(int)));
  connect(m_ihm.ui.ServoAX_num_7, SIGNAL(valueChanged(int)), m_ihm.ui.ServoAX_slide_7, SLOT(setValue(int)));

  connect(m_ihm.ui.ServoAX_slide_0, SIGNAL(sliderMoved(int)), m_ihm.ui.ServoAX_num_0, SLOT(setValue(int)));
  connect(m_ihm.ui.ServoAX_slide_1, SIGNAL(sliderMoved(int)), m_ihm.ui.ServoAX_num_1, SLOT(setValue(int)));
  connect(m_ihm.ui.ServoAX_slide_2, SIGNAL(sliderMoved(int)), m_ihm.ui.ServoAX_num_2, SLOT(setValue(int)));
  connect(m_ihm.ui.ServoAX_slide_3, SIGNAL(sliderMoved(int)), m_ihm.ui.ServoAX_num_3, SLOT(setValue(int)));
  connect(m_ihm.ui.ServoAX_slide_4, SIGNAL(sliderMoved(int)), m_ihm.ui.ServoAX_num_4, SLOT(setValue(int)));
  connect(m_ihm.ui.ServoAX_slide_5, SIGNAL(sliderMoved(int)), m_ihm.ui.ServoAX_num_5, SLOT(setValue(int)));
  connect(m_ihm.ui.ServoAX_slide_6, SIGNAL(sliderMoved(int)), m_ihm.ui.ServoAX_num_6, SLOT(setValue(int)));
  connect(m_ihm.ui.ServoAX_slide_7, SIGNAL(sliderMoved(int)), m_ihm.ui.ServoAX_num_7, SLOT(setValue(int)));

  connect(m_ihm.ui.ServoAX_num_0, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteurAX0_changed()));
  connect(m_ihm.ui.ServoAX_num_1, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteurAX1_changed()));
  connect(m_ihm.ui.ServoAX_num_2, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteurAX2_changed()));
  connect(m_ihm.ui.ServoAX_num_3, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteurAX3_changed()));
  connect(m_ihm.ui.ServoAX_num_4, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteurAX4_changed()));
  connect(m_ihm.ui.ServoAX_num_5, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteurAX5_changed()));
  connect(m_ihm.ui.ServoAX_num_6, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteurAX6_changed()));
  connect(m_ihm.ui.ServoAX_num_7, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteurAX7_changed()));

  connect(m_ihm.ui.ServoAX_slide_0, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteurAX0_changed()));
  connect(m_ihm.ui.ServoAX_slide_1, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteurAX1_changed()));
  connect(m_ihm.ui.ServoAX_slide_2, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteurAX2_changed()));
  connect(m_ihm.ui.ServoAX_slide_3, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteurAX3_changed()));
  connect(m_ihm.ui.ServoAX_slide_4, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteurAX4_changed()));
  connect(m_ihm.ui.ServoAX_slide_5, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteurAX5_changed()));
  connect(m_ihm.ui.ServoAX_slide_6, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteurAX6_changed()));
  connect(m_ihm.ui.ServoAX_slide_7, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteurAX7_changed()));

  connect(m_ihm.ui.ServosAXConfig_Send, SIGNAL(clicked()), this, SLOT(ServosAXConfig_Send_clicked()));

  // met à jour la liste des commandes possibles sur les servos AX
  initList_ActionsServosAX();

  // _____________________________________________ Power Switch
  updatePowerSwitchAliasLabels();
  updatePowerSwitchTooltips();
  connect(m_ihm.ui.PowerSwitch_StopAll, SIGNAL(clicked()), this, SLOT(PowerSwitch_StopAll_clicked()));

  m_ihm.ui.Send_CdePowerSwitch->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ihm.ui.Send_CdePowerSwitch, SIGNAL(clicked()), this, SLOT(CdePowerSwitchSynchroSend_left_clic()));
  connect(m_ihm.ui.Send_CdePowerSwitch, SIGNAL(customContextMenuRequested(QPoint)) , this, SLOT(CdePowerSwitchSynchroSend_right_clic(QPoint)));

  connect(m_ihm.ui.PowerSwitch_xt1, SIGNAL(clicked(bool)), this, SLOT(CdePowerSwitch_xt1_changed(bool)));
  connect(m_ihm.ui.PowerSwitch_xt2, SIGNAL(clicked(bool)), this, SLOT(CdePowerSwitch_xt2_changed(bool)));
  connect(m_ihm.ui.PowerSwitch_xt3, SIGNAL(clicked(bool)), this, SLOT(CdePowerSwitch_xt3_changed(bool)));
  connect(m_ihm.ui.PowerSwitch_xt4, SIGNAL(clicked(bool)), this, SLOT(CdePowerSwitch_xt4_changed(bool)));
  connect(m_ihm.ui.PowerSwitch_xt5, SIGNAL(clicked(bool)), this, SLOT(CdePowerSwitch_xt5_changed(bool)));
  connect(m_ihm.ui.PowerSwitch_xt6, SIGNAL(clicked(bool)), this, SLOT(CdePowerSwitch_xt6_changed(bool)));
  connect(m_ihm.ui.PowerSwitch_xt7, SIGNAL(clicked(bool)), this, SLOT(CdePowerSwitch_xt7_changed(bool)));
  connect(m_ihm.ui.PowerSwitch_xt8, SIGNAL(clicked(bool)), this, SLOT(CdePowerSwitch_xt8_changed(bool)));
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CActuatorElectrobot::close(void)
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
void CActuatorElectrobot::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

// _____________________________________________________________________
void CActuatorElectrobot::keyPressed(int key)
{
  switch(key) {
    case Qt::Key_Escape:
        Moteurs_StopAll_clicked();
        PowerSwitch_StopAll_clicked();
      break;
    // default : ne rien faire
  }
}

// =============================================================
//                          MOTEURS
// =============================================================
// _____________________________________________________________________
void CActuatorElectrobot::updateMotorsAliasLabels(void)
{
 // Gestion des alias sur les noms de capteurs
 QString str_val;
 str_val = m_application->m_data_center->getDataProperty("cde_moteur_1", "Alias").toString();
 if (str_val != "") { m_ihm.ui.label_Moteur_1->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_moteur_2", "Alias").toString();
 if (str_val != "") { m_ihm.ui.label_Moteur_2->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_moteur_3", "Alias").toString();
 if (str_val != "") { m_ihm.ui.label_Moteur_3->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_moteur_4", "Alias").toString();
 if (str_val != "") { m_ihm.ui.label_Moteur_4->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_moteur_5", "Alias").toString();
 if (str_val != "") { m_ihm.ui.label_Moteur_5->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_moteur_6", "Alias").toString();
 if (str_val != "") { m_ihm.ui.label_Moteur_6->setText(str_val); }
}



// _____________________________________________________________________
void CActuatorElectrobot::updateMotorsTooltips(void)
{
 QString str_val;

 str_val = m_application->m_data_center->getDataProperty("cde_moteur_1", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.Moteur_1->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_moteur_2", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.Moteur_2->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_moteur_3", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.Moteur_3->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_moteur_4", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.Moteur_4->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_moteur_5", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.Moteur_5->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_moteur_6", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.Moteur_6->setToolTip(str_val); }
}


// _____________________________________________________________________
void CActuatorElectrobot::CdeMoteur1_changed(void)                  { m_application->m_data_center->write("cde_moteur_1", m_ihm.ui.Moteur_1->value()); }
void CActuatorElectrobot::CdeMoteur2_changed(void)                  { m_application->m_data_center->write("cde_moteur_2", m_ihm.ui.Moteur_2->value()); }
void CActuatorElectrobot::CdeMoteur3_changed(void)                  { m_application->m_data_center->write("cde_moteur_3", m_ihm.ui.Moteur_3->value()); }
void CActuatorElectrobot::CdeMoteur4_changed(void)                  { m_application->m_data_center->write("cde_moteur_4", m_ihm.ui.Moteur_4->value()); }
void CActuatorElectrobot::CdeMoteur5_changed(void)                  { m_application->m_data_center->write("cde_moteur_5", m_ihm.ui.Moteur_5->value()); }
void CActuatorElectrobot::CdeMoteur6_changed(void)                  { m_application->m_data_center->write("cde_moteur_6", m_ihm.ui.Moteur_6->value()); }


// _____________________________________________________________________
// Clic gauche : envoie la trame
// Clic droit : vérouille la trame
void CActuatorElectrobot::CdeMoteursSynchroSend_left_clic(void)
{
  // enchaine 1->0 sur le floag de synchro pour forcer l'émission de la trame
  m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", true);
  m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", false);
  m_ihm.ui.Send_CdeMoteurs->setChecked(false);
}

void CActuatorElectrobot::CdeMoteursSynchroSend_right_clic(QPoint pt)
{
  Q_UNUSED(pt)
  m_ihm.ui.Send_CdeMoteurs->setCheckable(true);
  m_ihm.ui.Send_CdeMoteurs->setChecked(!m_ihm.ui.Send_CdeMoteurs->isChecked());
  m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", m_ihm.ui.Send_CdeMoteurs->isChecked());
}

// _____________________________________________________________________
void CActuatorElectrobot::Moteurs_StopAll_clicked(void)
{
 // Synchro pour qu'une seule trame ne parte
 m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 1);

 m_application->m_data_center->write("cde_moteur_1", 0);
 m_application->m_data_center->write("cde_moteur_2", 0);
 m_application->m_data_center->write("cde_moteur_3", 0);
 m_application->m_data_center->write("cde_moteur_4", 0);
 m_application->m_data_center->write("cde_moteur_5", 0);
 m_application->m_data_center->write("cde_moteur_6", 0);

 // Synchro = 0 : la trame est envoyée
 m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 0);

 // Met à jour l'IHM
 m_ihm.ui.Moteur_1->setValue(0);
 m_ihm.ui.Moteur_2->setValue(0);
 m_ihm.ui.Moteur_3->setValue(0);
 m_ihm.ui.Moteur_4->setValue(0);
 m_ihm.ui.Moteur_5->setValue(0);
 m_ihm.ui.Moteur_6->setValue(0);
 m_ihm.ui.Send_CdeMoteurs->setChecked(false);
}


// =============================================================
//               SERVOS CLASSIQUES & SERVOS SD20
// =============================================================
// _____________________________________________________________
void CActuatorElectrobot::PosServoMoteur_changed(int id, int position, int speed)
{
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", true);
    m_application->m_data_center->write("NumeroServoMoteur1", id);
    m_application->m_data_center->write("PositionServoMoteur1", position);
    m_application->m_data_center->write("VitesseServoMoteur1", speed);
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", false);
}

// _____________________________________________________________________
void CActuatorElectrobot::CdeServoMoteur1_changed()
{ PosServoMoteur_changed(servo_id_to_servo_num(1), m_ihm.ui.Servo_slide_1->value(), m_ihm.ui.servo_speed->value()); }

void CActuatorElectrobot::CdeServoMoteur2_changed()
{ PosServoMoteur_changed(servo_id_to_servo_num(2), m_ihm.ui.Servo_slide_2->value(), m_ihm.ui.servo_speed->value()); }

void CActuatorElectrobot::CdeServoMoteur3_changed()
{ PosServoMoteur_changed(servo_id_to_servo_num(3), m_ihm.ui.Servo_slide_3->value(), m_ihm.ui.servo_speed->value()); }

void CActuatorElectrobot::CdeServoMoteur4_changed()
{ PosServoMoteur_changed(servo_id_to_servo_num(4), m_ihm.ui.Servo_slide_4->value(), m_ihm.ui.servo_speed->value()); }

void CActuatorElectrobot::CdeServoMoteur5_changed()
{ PosServoMoteur_changed(servo_id_to_servo_num(5), m_ihm.ui.Servo_slide_5->value(), m_ihm.ui.servo_speed->value()); }

void CActuatorElectrobot::CdeServoMoteur6_changed()
{ PosServoMoteur_changed(servo_id_to_servo_num(6), m_ihm.ui.Servo_slide_6->value(), m_ihm.ui.servo_speed->value()); }

void CActuatorElectrobot::CdeServoMoteur7_changed()
{ PosServoMoteur_changed(servo_id_to_servo_num(7), m_ihm.ui.Servo_slide_7->value(), m_ihm.ui.servo_speed->value()); }

void CActuatorElectrobot::CdeServoMoteur8_changed()
{ PosServoMoteur_changed(servo_id_to_servo_num(8), m_ihm.ui.Servo_slide_8->value(), m_ihm.ui.servo_speed->value()); }


void CActuatorElectrobot::choix_type_servo_classique_sd20()
{

    if (m_ihm.ui.choix_servos_classiques->isChecked())  m_choix_type_servos = cSERVOS_CLASSIQUES;
    else if (m_ihm.ui.choix_servos_sd20->isChecked())   m_choix_type_servos = cSERVOS_SD20;
    else                                                m_choix_type_servos = cSERVOS_CLASSIQUES;


    int min_value, max_value, default_val;
    switch(m_choix_type_servos)
    {
    case cSERVOS_SD20 :
        min_value = 0;
        max_value = 255;
        default_val = 128;
        m_ihm.ui.servo_label_8->setEnabled(true); // il n'y a que 7 servos sur la carte Electrobot H755

        break;

    case cSERVOS_CLASSIQUES :
    default :
        m_ihm.ui.servo_label_1->setText("Servo1");
        min_value = 300;
        max_value = 3000;
        default_val = 1500;
        m_ihm.ui.servo_label_8->setEnabled(false);
        break;
    }
    // ajuste le nom du label
    m_ihm.ui.servo_label_1->setText(QString("Servo%1").arg(servo_id_to_servo_num(1)));
    m_ihm.ui.servo_label_2->setText(QString("Servo%1").arg(servo_id_to_servo_num(2)));
    m_ihm.ui.servo_label_3->setText(QString("Servo%1").arg(servo_id_to_servo_num(3)));
    m_ihm.ui.servo_label_4->setText(QString("Servo%1").arg(servo_id_to_servo_num(4)));
    m_ihm.ui.servo_label_5->setText(QString("Servo%1").arg(servo_id_to_servo_num(5)));
    m_ihm.ui.servo_label_6->setText(QString("Servo%1").arg(servo_id_to_servo_num(6)));
    m_ihm.ui.servo_label_7->setText(QString("Servo%1").arg(servo_id_to_servo_num(7)));
    m_ihm.ui.servo_label_8->setText(QString("Servo%1").arg(servo_id_to_servo_num(8)));

    // Fixe les valeurs min et max des sliders et champs numériques
    m_ihm.ui.Servo_slide_1->setMinimum(min_value);
    m_ihm.ui.Servo_slide_1->setMaximum(max_value);
    m_ihm.ui.Servo_slide_2->setMinimum(min_value);
    m_ihm.ui.Servo_slide_2->setMaximum(max_value);
    m_ihm.ui.Servo_slide_3->setMinimum(min_value);
    m_ihm.ui.Servo_slide_3->setMaximum(max_value);
    m_ihm.ui.Servo_slide_4->setMinimum(min_value);
    m_ihm.ui.Servo_slide_4->setMaximum(max_value);
    m_ihm.ui.Servo_slide_5->setMinimum(min_value);
    m_ihm.ui.Servo_slide_5->setMaximum(max_value);
    m_ihm.ui.Servo_slide_6->setMinimum(min_value);
    m_ihm.ui.Servo_slide_6->setMaximum(max_value);
    m_ihm.ui.Servo_slide_7->setMinimum(min_value);
    m_ihm.ui.Servo_slide_7->setMaximum(max_value);
    m_ihm.ui.Servo_slide_8->setMinimum(min_value);
    m_ihm.ui.Servo_slide_8->setMaximum(max_value);

    m_ihm.ui.Servo_num_1->setMinimum(min_value);
    m_ihm.ui.Servo_num_1->setMaximum(max_value);
    m_ihm.ui.Servo_num_2->setMinimum(min_value);
    m_ihm.ui.Servo_num_2->setMaximum(max_value);
    m_ihm.ui.Servo_num_3->setMinimum(min_value);
    m_ihm.ui.Servo_num_3->setMaximum(max_value);
    m_ihm.ui.Servo_num_4->setMinimum(min_value);
    m_ihm.ui.Servo_num_4->setMaximum(max_value);
    m_ihm.ui.Servo_num_5->setMinimum(min_value);
    m_ihm.ui.Servo_num_5->setMaximum(max_value);
    m_ihm.ui.Servo_num_6->setMinimum(min_value);
    m_ihm.ui.Servo_num_6->setMaximum(max_value);
    m_ihm.ui.Servo_num_7->setMinimum(min_value);
    m_ihm.ui.Servo_num_7->setMaximum(max_value);
    m_ihm.ui.Servo_num_8->setMinimum(min_value);
    m_ihm.ui.Servo_num_8->setMaximum(max_value);

    // remet les valeurs par défaut au centre
    m_ihm.ui.Servo_slide_1->setValue(default_val);
    m_ihm.ui.Servo_slide_2->setValue(default_val);
    m_ihm.ui.Servo_slide_3->setValue(default_val);
    m_ihm.ui.Servo_slide_4->setValue(default_val);
    m_ihm.ui.Servo_slide_5->setValue(default_val);
    m_ihm.ui.Servo_slide_6->setValue(default_val);
    m_ihm.ui.Servo_slide_7->setValue(default_val);
    m_ihm.ui.Servo_slide_8->setValue(default_val);

    m_ihm.ui.Servo_num_1->setValue(default_val);
    m_ihm.ui.Servo_num_2->setValue(default_val);
    m_ihm.ui.Servo_num_3->setValue(default_val);
    m_ihm.ui.Servo_num_4->setValue(default_val);
    m_ihm.ui.Servo_num_5->setValue(default_val);
    m_ihm.ui.Servo_num_6->setValue(default_val);
    m_ihm.ui.Servo_num_7->setValue(default_val);
    m_ihm.ui.Servo_num_8->setValue(default_val);
}

unsigned int CActuatorElectrobot::servo_id_to_servo_num(unsigned int servo_id)
{
    if (m_choix_type_servos == cSERVOS_CLASSIQUES) {
        return servo_id;
    }
    else if (m_choix_type_servos == cSERVOS_SD20) {
        return 20-servo_id+1;  // 20, 19, ... 13
    }

    return servo_id;
}


// =============================================================
//                          SERVOS AX
// =============================================================

// _____________________________________________________________
void CActuatorElectrobot::PosServoMoteurAX_changed(int id, int position)
{
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", true);
    m_application->m_data_center->write("num_servo_ax", id);
    m_application->m_data_center->write("commande_ax", cSERVO_AX_POSITION);
    m_application->m_data_center->write("valeur_commande_ax", position);
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", false);
}

// ____________________________________________________
void CActuatorElectrobot::CdeServoMoteurAX0_changed()
{ PosServoMoteurAX_changed(0, m_ihm.ui.ServoAX_slide_0->value()); }

void CActuatorElectrobot::CdeServoMoteurAX1_changed()
{ PosServoMoteurAX_changed(1, m_ihm.ui.ServoAX_slide_1->value()); }

void CActuatorElectrobot::CdeServoMoteurAX2_changed()
{ PosServoMoteurAX_changed(2, m_ihm.ui.ServoAX_slide_2->value()); }

void CActuatorElectrobot::CdeServoMoteurAX3_changed()
{ PosServoMoteurAX_changed(3, m_ihm.ui.ServoAX_slide_3->value()); }

void CActuatorElectrobot::CdeServoMoteurAX4_changed()
{ PosServoMoteurAX_changed(4, m_ihm.ui.ServoAX_slide_4->value()); }

void CActuatorElectrobot::CdeServoMoteurAX5_changed()
{ PosServoMoteurAX_changed(5, m_ihm.ui.ServoAX_slide_5->value()); }

void CActuatorElectrobot::CdeServoMoteurAX6_changed()
{ PosServoMoteurAX_changed(6, m_ihm.ui.ServoAX_slide_6->value()); }

void CActuatorElectrobot::CdeServoMoteurAX7_changed()
{ PosServoMoteurAX_changed(7, m_ihm.ui.ServoAX_slide_7->value()); }



// ____________________________________________________
void CActuatorElectrobot::initList_ActionsServosAX(void)
{
  QStringList lst;
  // à mettre dans le même ordre que l'énuméré "eCOMMANDES_SERVOS_AX" (commun avec le CPU)
  lst << "POSITION"
      << "VITESSE"
      << "COUPLE"
      << "CHANGE_ID"
      << "LED_STATE"
      << "BUTEE_MIN"
      << "BUTEE_MAX"
      << "POSITION_INIT";
  m_ihm.ui.ServosAXConfig_Action->addItems(lst);
}

// ____________________________________________________
void CActuatorElectrobot::ServosAXConfig_Send_clicked(void)
{
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", true);
    m_application->m_data_center->write("num_servo_ax", m_ihm.ui.ServosAXConfig_ID->value());
    m_application->m_data_center->write("commande_ax", m_ihm.ui.ServosAXConfig_Action->currentIndex());
    m_application->m_data_center->write("valeur_commande_ax", m_ihm.ui.ServosAXConfig_Value->value());
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", false);
}



// ____________________________________________________
void CActuatorElectrobot::initList_ActionsServos(void)
{
  QStringList lst;
  // à mettre dans le même ordre que l'énuméré "eCONFIG_SERVOS" (commun avec le CPU)
  lst << "POSITION"
      << "BUTEE_MIN"
      << "BUTEE_MAX"
      << "POSITION_INIT";
  m_ihm.ui.ServosConfig_Action->addItems(lst);
}

// ____________________________________________________
void CActuatorElectrobot::ServosConfig_Send_clicked(void)
{
    m_application->m_data_center->write("ELECTROBOT_CONFIG_SERVOS_TxSync", true);
    m_application->m_data_center->write("num_servo_sd20", m_ihm.ui.ServosConfig_ID->value());
    m_application->m_data_center->write("commande_sd20", m_ihm.ui.ServosConfig_Action->currentIndex());
    m_application->m_data_center->write("valeur_commande_sd20", m_ihm.ui.ServosConfig_Value->value());
    m_application->m_data_center->write("ELECTROBOT_CONFIG_SERVOS_TxSync", false);
}


// =============================================================
//                          POWER SWITCH
// =============================================================
void CActuatorElectrobot::updatePowerSwitchAliasLabels(void)
{
 // Gestion des alias sur les noms de capteurs
 QString str_val;
 str_val = m_application->m_data_center->getDataProperty("PowerSwitch_xt1", "Alias").toString();
 if (str_val != "") { m_ihm.ui.PowerSwitch_xt1->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerSwitch_xt2", "Alias").toString();
 if (str_val != "") { m_ihm.ui.PowerSwitch_xt2->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerSwitch_xt3", "Alias").toString();
 if (str_val != "") { m_ihm.ui.PowerSwitch_xt3->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerSwitch_xt4", "Alias").toString();
 if (str_val != "") { m_ihm.ui.PowerSwitch_xt4->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerSwitch_xt5", "Alias").toString();
 if (str_val != "") { m_ihm.ui.PowerSwitch_xt5->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerSwitch_xt6", "Alias").toString();
 if (str_val != "") { m_ihm.ui.PowerSwitch_xt6->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerSwitch_xt7", "Alias").toString();
 if (str_val != "") { m_ihm.ui.PowerSwitch_xt7->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerSwitch_xt8", "Alias").toString();
 if (str_val != "") { m_ihm.ui.PowerSwitch_xt8->setText(str_val); }
}



// _____________________________________________________________________
void CActuatorElectrobot::updatePowerSwitchTooltips(void)
{
 QString str_val;

 str_val = m_application->m_data_center->getDataProperty("PowerSwitch_xt1", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.PowerSwitch_xt1->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerSwitch_xt2", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.PowerSwitch_xt2->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerSwitch_xt3", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.PowerSwitch_xt3->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerSwitch_xt4", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.PowerSwitch_xt4->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerSwitch_xt5", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.PowerSwitch_xt5->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerSwitch_xt6", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.PowerSwitch_xt6->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerSwitch_xt7", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.PowerSwitch_xt7->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerSwitch_xt8", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.PowerSwitch_xt8->setToolTip(str_val); }
}


// _____________________________________________________________________
void CActuatorElectrobot::CdePowerSwitch_xt1_changed(bool val)      { m_application->m_data_center->write("PowerSwitch_xt1", val); }
void CActuatorElectrobot::CdePowerSwitch_xt2_changed(bool val)      { m_application->m_data_center->write("PowerSwitch_xt2", val); }
void CActuatorElectrobot::CdePowerSwitch_xt3_changed(bool val)      { m_application->m_data_center->write("PowerSwitch_xt3", val); }
void CActuatorElectrobot::CdePowerSwitch_xt4_changed(bool val)      { m_application->m_data_center->write("PowerSwitch_xt4", val); }
void CActuatorElectrobot::CdePowerSwitch_xt5_changed(bool val)      { m_application->m_data_center->write("PowerSwitch_xt5", val); }
void CActuatorElectrobot::CdePowerSwitch_xt6_changed(bool val)      { m_application->m_data_center->write("PowerSwitch_xt6", val); }
void CActuatorElectrobot::CdePowerSwitch_xt7_changed(bool val)      { m_application->m_data_center->write("PowerSwitch_xt7", val); }
void CActuatorElectrobot::CdePowerSwitch_xt8_changed(bool val)      { m_application->m_data_center->write("PowerSwitch_xt8", val); }


// _____________________________________________________________________
// Clic gauche : envoie la trame
// Clic droit : vérouille la trame
void CActuatorElectrobot::CdePowerSwitchSynchroSend_left_clic(void)
{
  // enchaine 1->0 sur le floag de synchro pour forcer l'émission de la trame
  m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", true);
  m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", false);
  m_ihm.ui.Send_CdePowerSwitch->setChecked(false);
}

void CActuatorElectrobot::CdePowerSwitchSynchroSend_right_clic(QPoint pt)
{
  Q_UNUSED(pt)
  m_ihm.ui.Send_CdePowerSwitch->setCheckable(true);
  m_ihm.ui.Send_CdePowerSwitch->setChecked(!m_ihm.ui.Send_CdePowerSwitch->isChecked());
  m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", m_ihm.ui.Send_CdePowerSwitch->isChecked());
}

// _____________________________________________________________________
void CActuatorElectrobot::PowerSwitch_StopAll_clicked(void)
{
 // Synchro pour qu'une seule trame ne parte
 m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 1);

 m_application->m_data_center->write("PowerSwitch_xt1", false);
 m_application->m_data_center->write("PowerSwitch_xt2", false);
 m_application->m_data_center->write("PowerSwitch_xt3", false);
 m_application->m_data_center->write("PowerSwitch_xt4", false);
 m_application->m_data_center->write("PowerSwitch_xt5", false);
 m_application->m_data_center->write("PowerSwitch_xt6", false);
 m_application->m_data_center->write("PowerSwitch_xt7", false);
 m_application->m_data_center->write("PowerSwitch_xt8", false);

 // Synchro = 0 : la trame est envoyée
 m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 0);

 // Met à jour l'IHM
 m_ihm.ui.PowerSwitch_xt1->setChecked(0);
 m_ihm.ui.PowerSwitch_xt2->setChecked(0);
 m_ihm.ui.PowerSwitch_xt3->setChecked(0);
 m_ihm.ui.PowerSwitch_xt4->setChecked(0);
 m_ihm.ui.PowerSwitch_xt5->setChecked(0);
 m_ihm.ui.PowerSwitch_xt6->setChecked(0);
 m_ihm.ui.PowerSwitch_xt7->setChecked(0);
 m_ihm.ui.PowerSwitch_xt8->setChecked(0);
 m_ihm.ui.Send_CdePowerSwitch->setChecked(false);
}
