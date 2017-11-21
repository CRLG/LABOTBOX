/*! \file CActuatorElectrobot.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CActuatorElectrobot.h"
#include "CLaBotBox.h"
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
void CActuatorElectrobot::init(CLaBotBox *application)
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
  updateAliasLabels();

  connect(&m_ihm, SIGNAL(keyPressed(int)), this, SLOT(keyPressed(int)));


  // _____________________________________________ Servo moteurs
  connect(m_ihm.ui.Servo_num_20, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_20, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_19, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_19, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_18, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_18, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_17, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_17, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_16, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_16, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_15, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_15, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_14, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_14, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_13, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_13, SLOT(setValue(int)));

  connect(m_ihm.ui.Servo_slide_20, SIGNAL(sliderMoved(int)), m_ihm.ui.Servo_num_20, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_19, SIGNAL(sliderMoved(int)), m_ihm.ui.Servo_num_19, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_18, SIGNAL(sliderMoved(int)), m_ihm.ui.Servo_num_18, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_17, SIGNAL(sliderMoved(int)), m_ihm.ui.Servo_num_17, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_16, SIGNAL(sliderMoved(int)), m_ihm.ui.Servo_num_16, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_15, SIGNAL(sliderMoved(int)), m_ihm.ui.Servo_num_15, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_14, SIGNAL(sliderMoved(int)), m_ihm.ui.Servo_num_14, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_13, SIGNAL(sliderMoved(int)), m_ihm.ui.Servo_num_13, SLOT(setValue(int)));

  connect(m_ihm.ui.Servo_num_20, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteur20_changed()));
  connect(m_ihm.ui.Servo_num_19, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteur19_changed()));
  connect(m_ihm.ui.Servo_num_18, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteur18_changed()));
  connect(m_ihm.ui.Servo_num_17, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteur17_changed()));
  connect(m_ihm.ui.Servo_num_16, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteur16_changed()));
  connect(m_ihm.ui.Servo_num_15, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteur15_changed()));
  connect(m_ihm.ui.Servo_num_14, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteur14_changed()));
  connect(m_ihm.ui.Servo_num_13, SIGNAL(editingFinished()), this, SLOT(CdeServoMoteur13_changed()));

  connect(m_ihm.ui.Servo_slide_20, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteur20_changed()));
  connect(m_ihm.ui.Servo_slide_19, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteur19_changed()));
  connect(m_ihm.ui.Servo_slide_18, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteur18_changed()));
  connect(m_ihm.ui.Servo_slide_17, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteur17_changed()));
  connect(m_ihm.ui.Servo_slide_16, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteur16_changed()));
  connect(m_ihm.ui.Servo_slide_15, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteur15_changed()));
  connect(m_ihm.ui.Servo_slide_14, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteur14_changed()));
  connect(m_ihm.ui.Servo_slide_13, SIGNAL(sliderReleased()), this, SLOT(CdeServoMoteur13_changed()));

  connect(m_ihm.ui.ServosSD20Config_Send, SIGNAL(clicked()), this, SLOT(ServosSD20Config_Send_clicked()));

  // met à jour la liste des commandes possibles sur les servos SD20
  initList_ActionsServosSD20();

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
    case Qt::Key_Escape:    Moteurs_StopAll_clicked();      break;
    // default : ne rien faire
  }
}

// _____________________________________________________________________
void CActuatorElectrobot::updateAliasLabels(void)
{
 // Gestion des alias sur les noms des moteurs
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

 // Gestion des alias sur les noms des servos
 str_val = m_application->m_data_center->getDataProperty("cde_servo_13", "Alias").toString();
 if (str_val != "") { m_ihm.ui.servo_label_13->setText(str_val);}

 str_val = m_application->m_data_center->getDataProperty("cde_servo_14", "Alias").toString();
 if (str_val != "") { m_ihm.ui.servo_label_14->setText(str_val);}

 str_val = m_application->m_data_center->getDataProperty("cde_servo_15", "Alias").toString();
 if (str_val != "") { m_ihm.ui.servo_label_15->setText(str_val);}

 str_val = m_application->m_data_center->getDataProperty("cde_servo_16", "Alias").toString();
 if (str_val != "") { m_ihm.ui.servo_label_16->setText(str_val);}

 str_val = m_application->m_data_center->getDataProperty("cde_servo_17", "Alias").toString();
 if (str_val != "") { m_ihm.ui.servo_label_17->setText(str_val);}

 str_val = m_application->m_data_center->getDataProperty("cde_servo_18", "Alias").toString();
 if (str_val != "") { m_ihm.ui.servo_label_18->setText(str_val);}

 str_val = m_application->m_data_center->getDataProperty("cde_servo_19", "Alias").toString();
 if (str_val != "") { m_ihm.ui.servo_label_19->setText(str_val);}

 str_val = m_application->m_data_center->getDataProperty("cde_servo_20", "Alias").toString();
 if (str_val != "") { m_ihm.ui.servo_label_20->setText(str_val);}


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

 //pour les servos
 str_val = m_application->m_data_center->getDataProperty("cde_servo_13", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.Servo_num_13->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_servo_14", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.Servo_num_14->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_servo_15", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.Servo_num_15->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_servo_16", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.Servo_num_16->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_servo_17", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.Servo_num_17->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_servo_18", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.Servo_num_18->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_servo_19", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.Servo_num_19->setToolTip(str_val); }

 str_val = m_application->m_data_center->getDataProperty("cde_servo_20", "Tooltip").toString();
 if (str_val != "") { m_ihm.ui.Servo_num_20->setToolTip(str_val); }


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




// _____________________________________________________________
void CActuatorElectrobot::PosServoMoteur_changed(int id, int position)
{
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", true);
    m_application->m_data_center->write("NumeroServoMoteur1", id);
    m_application->m_data_center->write("PositionServoMoteur1", position);
    m_application->m_data_center->write("VitesseServoMoteur1", 0);
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", false);
}

// _____________________________________________________________________
void CActuatorElectrobot::CdeServoMoteur20_changed()
{ PosServoMoteur_changed(20, m_ihm.ui.Servo_slide_20->value()); }

void CActuatorElectrobot::CdeServoMoteur19_changed()
{ PosServoMoteur_changed(19, m_ihm.ui.Servo_slide_19->value()); }

void CActuatorElectrobot::CdeServoMoteur18_changed()
{ PosServoMoteur_changed(18, m_ihm.ui.Servo_slide_18->value()); }

void CActuatorElectrobot::CdeServoMoteur17_changed()
{ PosServoMoteur_changed(17, m_ihm.ui.Servo_slide_17->value()); }

void CActuatorElectrobot::CdeServoMoteur16_changed()
{ PosServoMoteur_changed(16, m_ihm.ui.Servo_slide_16->value()); }

void CActuatorElectrobot::CdeServoMoteur15_changed()
{ PosServoMoteur_changed(15, m_ihm.ui.Servo_slide_15->value()); }

void CActuatorElectrobot::CdeServoMoteur14_changed()
{ PosServoMoteur_changed(14, m_ihm.ui.Servo_slide_14->value()); }

void CActuatorElectrobot::CdeServoMoteur13_changed()
{ PosServoMoteur_changed(13, m_ihm.ui.Servo_slide_13->value()); }



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
  // à mettre dans le même ordre que l'énuméré "eCOMMANDES_SERVOS_AX" (commun avec le MBED)
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
void CActuatorElectrobot::initList_ActionsServosSD20(void)
{
  QStringList lst;
  // à mettre dans le même ordre que l'énuméré "eCOMMANDES_SERVOS_SD20" (commun avec le MBED)
  lst << "POSITION"
      << "BUTEE_MIN"
      << "BUTEE_MAX"
      << "POSITION_INIT";
  m_ihm.ui.ServosSD20Config_Action->addItems(lst);
}

// ____________________________________________________
void CActuatorElectrobot::ServosSD20Config_Send_clicked(void)
{
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_SD20_TxSync", true);
    m_application->m_data_center->write("num_servo_sd20", m_ihm.ui.ServosSD20Config_ID->value());
    m_application->m_data_center->write("commande_sd20", m_ihm.ui.ServosSD20Config_Action->currentIndex());
    m_application->m_data_center->write("valeur_commande_sd20", m_ihm.ui.ServosSD20Config_Value->value());
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_SD20_TxSync", false);
}
