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

  // Restore les butées
  for (unsigned int i=0; i<8; i++) {
    m_butees_min.append(m_application->m_eeprom->read(getName(), "butee_min_servo_"+ QString::number(i+1), QVariant(0)).toInt());
  }
  for (unsigned int i=0; i<8; i++) {
    m_butees_max.append(m_application->m_eeprom->read(getName(), "butee_max_servo_"+ QString::number(i+1), QVariant(0)).toInt());
  }

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

  // _____________________________________________ Servo moteurs
  connect(m_ihm.ui.Servo_num_1, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_1, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_2, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_2, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_3, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_3, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_4, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_4, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_5, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_5, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_6, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_6, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_7, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_7, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_num_8, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_slide_8, SLOT(setValue(int)));

  connect(m_ihm.ui.Servo_slide_1, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_num_1, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_2, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_num_2, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_3, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_num_3, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_4, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_num_4, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_5, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_num_5, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_6, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_num_6, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_7, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_num_7, SLOT(setValue(int)));
  connect(m_ihm.ui.Servo_slide_8, SIGNAL(valueChanged(int)), m_ihm.ui.Servo_num_8, SLOT(setValue(int)));

  connect(m_ihm.ui.Servo_slide_1, SIGNAL(valueChanged(int)), this, SLOT(CdeServoMoteur1_changed(int)));
  connect(m_ihm.ui.Servo_slide_2, SIGNAL(valueChanged(int)), this, SLOT(CdeServoMoteur2_changed(int)));
  connect(m_ihm.ui.Servo_slide_3, SIGNAL(valueChanged(int)), this, SLOT(CdeServoMoteur3_changed(int)));
  connect(m_ihm.ui.Servo_slide_4, SIGNAL(valueChanged(int)), this, SLOT(CdeServoMoteur4_changed(int)));
  connect(m_ihm.ui.Servo_slide_5, SIGNAL(valueChanged(int)), this, SLOT(CdeServoMoteur5_changed(int)));
  connect(m_ihm.ui.Servo_slide_6, SIGNAL(valueChanged(int)), this, SLOT(CdeServoMoteur6_changed(int)));
  connect(m_ihm.ui.Servo_slide_7, SIGNAL(valueChanged(int)), this, SLOT(CdeServoMoteur7_changed(int)));
  connect(m_ihm.ui.Servo_slide_8, SIGNAL(valueChanged(int)), this, SLOT(CdeServoMoteur8_changed(int)));

  connect(m_ihm.ui.ButeeMin_Servo_1, SIGNAL(editingFinished()), this, SLOT(ButeeServo_changed()));
  connect(m_ihm.ui.ButeeMin_Servo_2, SIGNAL(editingFinished()), this, SLOT(ButeeServo_changed()));
  connect(m_ihm.ui.ButeeMin_Servo_3, SIGNAL(editingFinished()), this, SLOT(ButeeServo_changed()));
  connect(m_ihm.ui.ButeeMin_Servo_4, SIGNAL(editingFinished()), this, SLOT(ButeeServo_changed()));
  connect(m_ihm.ui.ButeeMin_Servo_5, SIGNAL(editingFinished()), this, SLOT(ButeeServo_changed()));
  connect(m_ihm.ui.ButeeMin_Servo_6, SIGNAL(editingFinished()), this, SLOT(ButeeServo_changed()));
  connect(m_ihm.ui.ButeeMin_Servo_7, SIGNAL(editingFinished()), this, SLOT(ButeeServo_changed()));
  connect(m_ihm.ui.ButeeMin_Servo_8, SIGNAL(editingFinished()), this, SLOT(ButeeServo_changed()));

  connect(m_ihm.ui.ButeeMax_Servo_1, SIGNAL(editingFinished()), this, SLOT(ButeeServo_changed()));
  connect(m_ihm.ui.ButeeMax_Servo_2, SIGNAL(editingFinished()), this, SLOT(ButeeServo_changed()));
  connect(m_ihm.ui.ButeeMax_Servo_3, SIGNAL(editingFinished()), this, SLOT(ButeeServo_changed()));
  connect(m_ihm.ui.ButeeMax_Servo_4, SIGNAL(editingFinished()), this, SLOT(ButeeServo_changed()));
  connect(m_ihm.ui.ButeeMax_Servo_5, SIGNAL(editingFinished()), this, SLOT(ButeeServo_changed()));
  connect(m_ihm.ui.ButeeMax_Servo_6, SIGNAL(editingFinished()), this, SLOT(ButeeServo_changed()));
  connect(m_ihm.ui.ButeeMax_Servo_7, SIGNAL(editingFinished()), this, SLOT(ButeeServo_changed()));
  connect(m_ihm.ui.ButeeMax_Servo_8, SIGNAL(editingFinished()), this, SLOT(ButeeServo_changed()));


  // Lit les données mémorisées dans le data center sur les butées hautes et basses de chaque servo
  // et met à jour l'IHM
  updateButeesServos();
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

  for (unsigned int i=0; i<8; i++) {
    m_application->m_eeprom->write(getName(), "butee_min_servo_"+ QString::number(i+1), QVariant(m_butees_min.at(i)));
  }
  for (unsigned int i=0; i<8; i++) {
      m_application->m_eeprom->write(getName(), "butee_max_servo_"+ QString::number(i+1), QVariant(m_butees_max.at(i)));
  }

}


// _____________________________________________________________________
void CActuatorElectrobot::updateAliasLabels(void)
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
void CActuatorElectrobot::updateButeesServos(void)
{
 m_ihm.ui.ButeeMax_Servo_1->setValue(m_butees_max.at(0));
 m_ihm.ui.ButeeMax_Servo_2->setValue(m_butees_max.at(1));
 m_ihm.ui.ButeeMax_Servo_3->setValue(m_butees_max.at(2));
 m_ihm.ui.ButeeMax_Servo_4->setValue(m_butees_max.at(3));
 m_ihm.ui.ButeeMax_Servo_5->setValue(m_butees_max.at(4));
 m_ihm.ui.ButeeMax_Servo_6->setValue(m_butees_max.at(5));
 m_ihm.ui.ButeeMax_Servo_7->setValue(m_butees_max.at(6));
 m_ihm.ui.ButeeMax_Servo_8->setValue(m_butees_max.at(7));

 m_ihm.ui.ButeeMin_Servo_1->setValue(m_butees_min.at(0));
 m_ihm.ui.ButeeMin_Servo_2->setValue(m_butees_min.at(1));
 m_ihm.ui.ButeeMin_Servo_3->setValue(m_butees_min.at(2));
 m_ihm.ui.ButeeMin_Servo_4->setValue(m_butees_min.at(3));
 m_ihm.ui.ButeeMin_Servo_5->setValue(m_butees_min.at(4));
 m_ihm.ui.ButeeMin_Servo_6->setValue(m_butees_min.at(5));
 m_ihm.ui.ButeeMin_Servo_7->setValue(m_butees_min.at(6));
 m_ihm.ui.ButeeMin_Servo_8->setValue(m_butees_min.at(7));

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




// _____________________________________________________________________
void CActuatorElectrobot::CdeServoMoteur1_changed(int val)
{
 if      (val < m_butees_min[0]) {
  val = m_butees_min[0];
  m_ihm.ui.Servo_num_1->setValue(val);
  m_ihm.ui.Servo_slide_1->setValue(val);
 }
 else if (val > m_butees_max[0]) {
  val = m_butees_max[0];
  m_ihm.ui.Servo_num_1->setValue(val);
  m_ihm.ui.Servo_slide_1->setValue(val);
 }
 m_application->m_data_center->write("NumeroServoMoteur1", 20);
 m_application->m_data_center->write("PositionServoMoteur1", val);
 m_application->m_data_center->write("VitesseServoMoteur1", 0);
}
void CActuatorElectrobot::CdeServoMoteur2_changed(int val)
{
    if      (val < m_butees_min[1]) {
     val = m_butees_min[1];
     m_ihm.ui.Servo_num_2->setValue(val);
     m_ihm.ui.Servo_slide_2->setValue(val);
    }
    else if (val > m_butees_max[1]) {
     val = m_butees_max[1];
     m_ihm.ui.Servo_num_2->setValue(val);
     m_ihm.ui.Servo_slide_2->setValue(val);
    }
 m_application->m_data_center->write("NumeroServoMoteur1", 19);
 m_application->m_data_center->write("PositionServoMoteur1", val);
 m_application->m_data_center->write("VitesseServoMoteur1", 0);
}
void CActuatorElectrobot::CdeServoMoteur3_changed(int val)
{
 if      (val < m_butees_min[2]) {
  val = m_butees_min[2];
  m_ihm.ui.Servo_num_3->setValue(val);
  m_ihm.ui.Servo_slide_3->setValue(val);
 }
else if (val > m_butees_max[2]) {
 val = m_butees_max[2];
 m_ihm.ui.Servo_num_3->setValue(val);
 m_ihm.ui.Servo_slide_3->setValue(val);
}
 m_application->m_data_center->write("NumeroServoMoteur1", 18);
 m_application->m_data_center->write("PositionServoMoteur1", val);
 m_application->m_data_center->write("VitesseServoMoteur1", 0);
}
void CActuatorElectrobot::CdeServoMoteur4_changed(int val)
{
    if      (val < m_butees_min[3]) {
     val = m_butees_min[3];
     m_ihm.ui.Servo_num_4->setValue(val);
     m_ihm.ui.Servo_slide_4->setValue(val);
    }
   else if (val > m_butees_max[3]) {
    val = m_butees_max[3];
    m_ihm.ui.Servo_num_4->setValue(val);
    m_ihm.ui.Servo_slide_4->setValue(val);
   }
 m_application->m_data_center->write("NumeroServoMoteur1", 17);
 m_application->m_data_center->write("PositionServoMoteur1", val);
 m_application->m_data_center->write("VitesseServoMoteur1", 0);
}
void CActuatorElectrobot::CdeServoMoteur5_changed(int val)
{
    if      (val < m_butees_min[4]) {
     val = m_butees_min[4];
     m_ihm.ui.Servo_num_5->setValue(val);
     m_ihm.ui.Servo_slide_5->setValue(val);
    }
   else if (val > m_butees_max[4]) {
    val = m_butees_max[4];
    m_ihm.ui.Servo_num_5->setValue(val);
    m_ihm.ui.Servo_slide_5->setValue(val);
   }

 m_application->m_data_center->write("NumeroServoMoteur1", 16);
 m_application->m_data_center->write("PositionServoMoteur1", val);
 m_application->m_data_center->write("VitesseServoMoteur1", 0);
}
void CActuatorElectrobot::CdeServoMoteur6_changed(int val)
{
    if      (val < m_butees_min[5]) {
     val = m_butees_min[5];
     m_ihm.ui.Servo_num_6->setValue(val);
     m_ihm.ui.Servo_slide_6->setValue(val);
    }
   else if (val > m_butees_max[5]) {
    val = m_butees_max[5];
    m_ihm.ui.Servo_num_6->setValue(val);
    m_ihm.ui.Servo_slide_6->setValue(val);
   }

 m_application->m_data_center->write("NumeroServoMoteur1", 15);
 m_application->m_data_center->write("PositionServoMoteur1", val);
 m_application->m_data_center->write("VitesseServoMoteur1", 0);
}
void CActuatorElectrobot::CdeServoMoteur7_changed(int val)
{
    if      (val < m_butees_min[6]) {
     val = m_butees_min[6];
     m_ihm.ui.Servo_num_7->setValue(val);
     m_ihm.ui.Servo_slide_7->setValue(val);
    }
   else if (val > m_butees_max[6]) {
    val = m_butees_max[6];
    m_ihm.ui.Servo_num_7->setValue(val);
    m_ihm.ui.Servo_slide_7->setValue(val);
   }

 m_application->m_data_center->write("NumeroServoMoteur1", 14);
 m_application->m_data_center->write("PositionServoMoteur1", val);
 m_application->m_data_center->write("VitesseServoMoteur1", 0);
}
void CActuatorElectrobot::CdeServoMoteur8_changed(int val)
{
    if      (val < m_butees_min[7]) {
     val = m_butees_min[7];
     m_ihm.ui.Servo_num_8->setValue(val);
     m_ihm.ui.Servo_slide_8->setValue(val);
    }
   else if (val > m_butees_max[7]) {
    val = m_butees_max[7];
    m_ihm.ui.Servo_num_8->setValue(val);
    m_ihm.ui.Servo_slide_8->setValue(val);
   }

 m_application->m_data_center->write("NumeroServoMoteur1", 13);
 m_application->m_data_center->write("PositionServoMoteur1", val);
 m_application->m_data_center->write("VitesseServoMoteur1", 0);
}


// ____________________________________________________
void CActuatorElectrobot::ButeeServo_changed(void)
{
  m_butees_min[0] =m_ihm.ui.ButeeMin_Servo_1->value();
  m_butees_min[1] =m_ihm.ui.ButeeMin_Servo_2->value();
  m_butees_min[2] =m_ihm.ui.ButeeMin_Servo_3->value();
  m_butees_min[3] =m_ihm.ui.ButeeMin_Servo_4->value();
  m_butees_min[4] =m_ihm.ui.ButeeMin_Servo_5->value();
  m_butees_min[5] =m_ihm.ui.ButeeMin_Servo_6->value();
  m_butees_min[6] =m_ihm.ui.ButeeMin_Servo_7->value();
  m_butees_min[7] =m_ihm.ui.ButeeMin_Servo_8->value();

  m_butees_max[0] =m_ihm.ui.ButeeMax_Servo_1->value();
  m_butees_max[1] =m_ihm.ui.ButeeMax_Servo_2->value();
  m_butees_max[2] =m_ihm.ui.ButeeMax_Servo_3->value();
  m_butees_max[3] =m_ihm.ui.ButeeMax_Servo_4->value();
  m_butees_max[4] =m_ihm.ui.ButeeMax_Servo_5->value();
  m_butees_max[5] =m_ihm.ui.ButeeMax_Servo_6->value();
  m_butees_max[6] =m_ihm.ui.ButeeMax_Servo_7->value();
  m_butees_max[7] =m_ihm.ui.ButeeMax_Servo_8->value();
}
