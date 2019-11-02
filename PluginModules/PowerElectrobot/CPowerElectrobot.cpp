/*! \file CPowerElectrobot.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CPowerElectrobot.h"
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
CPowerElectrobot::CPowerElectrobot(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_PowerElectrobot, AUTEUR_PowerElectrobot, INFO_PowerElectrobot),
     m_eeprom_unlocked(false)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CPowerElectrobot::~CPowerElectrobot()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CPowerElectrobot::init(CApplication *application)
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

  // Connexions pour l'onglet Général
  connect(m_ihm.ui.push_all_outputs_off, SIGNAL(clicked(bool)), this, SLOT(onClicked_allOutputsOff()));
  //   >Connexions IHM -> DataManager
  connect(m_ihm.ui.output_1, SIGNAL(clicked(bool)), m_application->m_data_center->getData("PowerElectrobot.output1", true), SLOT(write(bool)));
  connect(m_ihm.ui.output_2, SIGNAL(clicked(bool)), m_application->m_data_center->getData("PowerElectrobot.output2", true), SLOT(write(bool)));
  connect(m_ihm.ui.output_3, SIGNAL(clicked(bool)), m_application->m_data_center->getData("PowerElectrobot.output3", true), SLOT(write(bool)));
  connect(m_ihm.ui.output_4, SIGNAL(clicked(bool)), m_application->m_data_center->getData("PowerElectrobot.output4", true), SLOT(write(bool)));
  connect(m_ihm.ui.output_5, SIGNAL(clicked(bool)), m_application->m_data_center->getData("PowerElectrobot.output5", true), SLOT(write(bool)));
  connect(m_ihm.ui.output_6, SIGNAL(clicked(bool)), m_application->m_data_center->getData("PowerElectrobot.output6", true), SLOT(write(bool)));
  connect(m_ihm.ui.output_7, SIGNAL(clicked(bool)), m_application->m_data_center->getData("PowerElectrobot.output7", true), SLOT(write(bool)));
  connect(m_ihm.ui.output_8, SIGNAL(clicked(bool)), m_application->m_data_center->getData("PowerElectrobot.output8", true), SLOT(write(bool)));
  //   >Connexions DataManger -> IHM
  connect(m_application->m_data_center->getData("PowerElectrobot.output1"), SIGNAL(valueChanged(bool)), m_ihm.ui.output_1, SLOT(setChecked(bool)));
  connect(m_application->m_data_center->getData("PowerElectrobot.output2"), SIGNAL(valueChanged(bool)), m_ihm.ui.output_2, SLOT(setChecked(bool)));
  connect(m_application->m_data_center->getData("PowerElectrobot.output3"), SIGNAL(valueChanged(bool)), m_ihm.ui.output_3, SLOT(setChecked(bool)));
  connect(m_application->m_data_center->getData("PowerElectrobot.output4"), SIGNAL(valueChanged(bool)), m_ihm.ui.output_4, SLOT(setChecked(bool)));
  connect(m_application->m_data_center->getData("PowerElectrobot.output5"), SIGNAL(valueChanged(bool)), m_ihm.ui.output_5, SLOT(setChecked(bool)));
  connect(m_application->m_data_center->getData("PowerElectrobot.output6"), SIGNAL(valueChanged(bool)), m_ihm.ui.output_6, SLOT(setChecked(bool)));
  connect(m_application->m_data_center->getData("PowerElectrobot.output7"), SIGNAL(valueChanged(bool)), m_ihm.ui.output_7, SLOT(setChecked(bool)));
  connect(m_application->m_data_center->getData("PowerElectrobot.output8"), SIGNAL(valueChanged(bool)), m_ihm.ui.output_8, SLOT(setChecked(bool)));

  connect(m_application->m_data_center->getData("PowerElectrobot.battery_voltage", true), SIGNAL(valueChanged(double)), m_ihm.ui.battery_voltage, SLOT(setValue(double)));
  connect(m_application->m_data_center->getData("PowerElectrobot.global_current", true), SIGNAL(valueChanged(double)), m_ihm.ui.global_current, SLOT(setValue(double)));
  connect(m_application->m_data_center->getData("PowerElectrobot.current_out1", true), SIGNAL(valueChanged(double)), m_ihm.ui.current_out1, SLOT(setValue(double)));
  connect(m_application->m_data_center->getData("PowerElectrobot.current_out2", true), SIGNAL(valueChanged(double)), m_ihm.ui.current_out2, SLOT(setValue(double)));
  //   >Connexions DataManger -> envoie message
  connect(m_application->m_data_center->getData("PowerElectrobot.output1"), SIGNAL(valueChanged(bool)), this, SLOT(onChanged_output1(bool)));
  connect(m_application->m_data_center->getData("PowerElectrobot.output2"), SIGNAL(valueChanged(bool)), this, SLOT(onChanged_output2(bool)));
  connect(m_application->m_data_center->getData("PowerElectrobot.output3"), SIGNAL(valueChanged(bool)), this, SLOT(onChanged_output3(bool)));
  connect(m_application->m_data_center->getData("PowerElectrobot.output4"), SIGNAL(valueChanged(bool)), this, SLOT(onChanged_output4(bool)));
  connect(m_application->m_data_center->getData("PowerElectrobot.output5"), SIGNAL(valueChanged(bool)), this, SLOT(onChanged_output5(bool)));
  connect(m_application->m_data_center->getData("PowerElectrobot.output6"), SIGNAL(valueChanged(bool)), this, SLOT(onChanged_output6(bool)));
  connect(m_application->m_data_center->getData("PowerElectrobot.output7"), SIGNAL(valueChanged(bool)), this, SLOT(onChanged_output7(bool)));
  connect(m_application->m_data_center->getData("PowerElectrobot.output8"), SIGNAL(valueChanged(bool)), this, SLOT(onChanged_output8(bool)));

  // Connexions pour l'onglet Calibration
  connect(m_ihm.ui.valid_calib_batt_voltag_p1, SIGNAL(clicked(bool)), this, SLOT(onClicked_CalibBattVoltagePoint1()));
  connect(m_ihm.ui.valid_calib_batt_voltag_p2, SIGNAL(clicked(bool)), this, SLOT(onClicked_CalibBattVoltagePoint2()));
  connect(m_ihm.ui.valid_calib_global_current_p1, SIGNAL(clicked(bool)), this, SLOT(onClicked_CalibGlobalCurrentPoint1()));
  connect(m_ihm.ui.valid_calib_global_current_p2, SIGNAL(clicked(bool)), this, SLOT(onClicked_CalibGlobalCurrentPoint2()));
  connect(m_ihm.ui.valid_calib_current_out1_p1, SIGNAL(clicked(bool)), this, SLOT(onClicked_CalibCurrentOut1Point1()));
  connect(m_ihm.ui.valid_calib_current_out1_p2, SIGNAL(clicked(bool)), this, SLOT(onClicked_CalibCurrentOut1Point2()));
  connect(m_ihm.ui.valid_calib_current_out2_p1, SIGNAL(clicked(bool)), this, SLOT(onClicked_CalibCurrentOut2Point1()));
  connect(m_ihm.ui.valid_calib_current_out2_p2, SIGNAL(clicked(bool)), this, SLOT(onClicked_CalibCurrentOut2Point2()));

  // Connexions pour l'onglet Spécial
  connect(m_ihm.ui.valid_unlock_eeprom, SIGNAL(clicked(bool)), this, SLOT(onClicked_UnlockEEPROM()));
  connect(m_ihm.ui.valid_reset_eeprom_factory, SIGNAL(clicked(bool)), this, SLOT(onClicked_ResetFactoryEEPROM()));
  connect(m_ihm.ui.valid_i2c_address, SIGNAL(clicked(bool)), this, SLOT(onClicked_ChangeI2CAddress()));

  // Grise les boutons d'accès aux paramètres EEPROM
  enableDisable_SendEEPROM();

  // Met à jour les alias sur les données s'il y en a
  updateOutputsLabels();
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CPowerElectrobot::close(void)
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
void CPowerElectrobot::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

// _____________________________________________________________________
void CPowerElectrobot::sendCommand(unsigned short command, unsigned short val)
{
    m_application->m_data_center->write("COMMANDE_POWER_ELECTROBOT_TxSync", true);
    m_application->m_data_center->write("PowerElectrobot.commande", command);
    m_application->m_data_center->write("PowerElectrobot.value", val);
    m_application->m_data_center->write("COMMANDE_POWER_ELECTROBOT_TxSync", false);
}


// ========================================================================
//                           ONGLET GENERAL
// ========================================================================
// _____________________________________________________________________
void CPowerElectrobot::onChanged_output1(bool val)
{
    sendCommand(cCDE_PWR_ELECTROBOT_OUTPUT_1, val);
}
// _____________________________________________________________________
void CPowerElectrobot::onChanged_output2(bool val)
{
    sendCommand(cCDE_PWR_ELECTROBOT_OUTPUT_2, val);
}
// _____________________________________________________________________
void CPowerElectrobot::onChanged_output3(bool val)
{
    sendCommand(cCDE_PWR_ELECTROBOT_OUTPUT_3, val);
}
// _____________________________________________________________________
void CPowerElectrobot::onChanged_output4(bool val)
{
    sendCommand(cCDE_PWR_ELECTROBOT_OUTPUT_4, val);
}
// _____________________________________________________________________
void CPowerElectrobot::onChanged_output5(bool val)
{
    sendCommand(cCDE_PWR_ELECTROBOT_OUTPUT_5, val);
}
// _____________________________________________________________________
void CPowerElectrobot::onChanged_output6(bool val)
{
    sendCommand(cCDE_PWR_ELECTROBOT_OUTPUT_6, val);
}
// _____________________________________________________________________
void CPowerElectrobot::onChanged_output7(bool val)
{
    sendCommand(cCDE_PWR_ELECTROBOT_OUTPUT_7, val);
}
// _____________________________________________________________________
void CPowerElectrobot::onChanged_output8(bool val)
{
    sendCommand(cCDE_PWR_ELECTROBOT_OUTPUT_8, val);
}
// _____________________________________________________________________
void CPowerElectrobot::onClicked_allOutputsOff()
{
    // Petite astuce :
    // Bloque l'envoie des le messages pour éviter que les 8 messages partent succéssivement
    // Mais que seul le message dédié pour tout éteindre d'un coup
    m_application->m_data_center->write("COMMANDE_POWER_ELECTROBOT_TxSync", true);

    m_application->m_data_center->write("PowerElectrobot.output1", 0);
    m_application->m_data_center->write("PowerElectrobot.output2", 0);
    m_application->m_data_center->write("PowerElectrobot.output3", 0);
    m_application->m_data_center->write("PowerElectrobot.output4", 0);
    m_application->m_data_center->write("PowerElectrobot.output5", 0);
    m_application->m_data_center->write("PowerElectrobot.output6", 0);
    m_application->m_data_center->write("PowerElectrobot.output7", 0);
    m_application->m_data_center->write("PowerElectrobot.output8", 0);

    // Le COMMANDE_POWER_ELECTROBOT_TxSync sera libéré par la fonction sendCommand
    sendCommand(cCDE_PWR_ELECTROBOT_ALL_OUTPUTS, 0);
}


// _____________________________________________________________________
// Change les noms des "output<x>" en son alias s'il existe
void CPowerElectrobot::updateOutputsLabels(void)
{
 QString str_val;

 str_val = m_application->m_data_center->getDataProperty("PowerElectrobot.output1", "Alias").toString();
 if (str_val != "") { m_ihm.ui.output_1->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerElectrobot.output2", "Alias").toString();
 if (str_val != "") { m_ihm.ui.output_2->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerElectrobot.output3", "Alias").toString();
 if (str_val != "") { m_ihm.ui.output_3->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerElectrobot.output4", "Alias").toString();
 if (str_val != "") { m_ihm.ui.output_4->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerElectrobot.output5", "Alias").toString();
 if (str_val != "") { m_ihm.ui.output_5->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerElectrobot.output6", "Alias").toString();
 if (str_val != "") { m_ihm.ui.output_6->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerElectrobot.output7", "Alias").toString();
 if (str_val != "") { m_ihm.ui.output_7->setText(str_val); }

 str_val = m_application->m_data_center->getDataProperty("PowerElectrobot.output8", "Alias").toString();
 if (str_val != "") { m_ihm.ui.output_8->setText(str_val); }
}


// ========================================================================
//                           ONGLET CALIBRATION
// ========================================================================
void CPowerElectrobot::onClicked_CalibBattVoltagePoint1()
{
    unsigned short val = m_ihm.ui.calib_batt_voltage_p1->value();
    sendCommand(cCDE_PWR_ELECTROBOT_CALIB_BATTERY_VOLTAGE_POINT1, val);
}
void CPowerElectrobot::onClicked_CalibBattVoltagePoint2()
{
    unsigned short val = m_ihm.ui.calib_batt_voltage_p2->value();
    sendCommand(cCDE_PWR_ELECTROBOT_CALIB_BATTERY_VOLTAGE_POINT2, val);
}
void CPowerElectrobot::onClicked_CalibGlobalCurrentPoint1()
{
    unsigned short val = m_ihm.ui.calib_global_current_p1->value();
    sendCommand(cCDE_PWR_ELECTROBOT_CALIB_GLOBAL_CURRENT_POINT1, val);
}
void CPowerElectrobot::onClicked_CalibGlobalCurrentPoint2()
{
    unsigned short val = m_ihm.ui.calib_global_current_p2->value();
    sendCommand(cCDE_PWR_ELECTROBOT_CALIB_GLOBAL_CURRENT_POINT2, val);
}
void CPowerElectrobot::onClicked_CalibCurrentOut1Point1()
{
    unsigned short val = m_ihm.ui.calib_current_out1_p1->value();
    sendCommand(cCDE_PWR_ELECTROBOT_CALIB_CURRENT_OUT1_POINT1, val);
}
void CPowerElectrobot::onClicked_CalibCurrentOut1Point2()
{
    unsigned short val = m_ihm.ui.calib_current_out1_p2->value();
    sendCommand(cCDE_PWR_ELECTROBOT_CALIB_CURRENT_OUT1_POINT2, val);
}
void CPowerElectrobot::onClicked_CalibCurrentOut2Point1()
{
    unsigned short val = m_ihm.ui.calib_current_out2_p1->value();
    sendCommand(cCDE_PWR_ELECTROBOT_CALIB_CURRENT_OUT2_POINT1, val);
}
void CPowerElectrobot::onClicked_CalibCurrentOut2Point2()
{
    unsigned short val = m_ihm.ui.calib_current_out2_p2->value();
    sendCommand(cCDE_PWR_ELECTROBOT_CALIB_CURRENT_OUT2_POINT2, val);
}


// ========================================================================
//                           ONGLET SPECIAL
// ========================================================================
void CPowerElectrobot::enableDisable_SendEEPROM()
{
    m_ihm.ui.valid_calib_batt_voltag_p1->setEnabled(m_eeprom_unlocked);
    m_ihm.ui.valid_calib_batt_voltag_p2->setEnabled(m_eeprom_unlocked);
    m_ihm.ui.valid_calib_global_current_p1->setEnabled(m_eeprom_unlocked);
    m_ihm.ui.valid_calib_global_current_p2->setEnabled(m_eeprom_unlocked);
    m_ihm.ui.valid_calib_current_out1_p1->setEnabled(m_eeprom_unlocked);
    m_ihm.ui.valid_calib_current_out1_p2->setEnabled(m_eeprom_unlocked);
    m_ihm.ui.valid_calib_current_out2_p1->setEnabled(m_eeprom_unlocked);
    m_ihm.ui.valid_calib_current_out2_p2->setEnabled(m_eeprom_unlocked);
    m_ihm.ui.valid_i2c_address->setEnabled(m_eeprom_unlocked);
    m_ihm.ui.valid_reset_eeprom_factory->setEnabled(m_eeprom_unlocked);
}

void CPowerElectrobot::onClicked_UnlockEEPROM()
{
    unsigned short dummy = 0;
    sendCommand(cCDE_PWR_ELECTROBOT_UNLOCK_EEPROM, dummy);
    m_eeprom_unlocked = true;
    enableDisable_SendEEPROM();
}

void CPowerElectrobot::onClicked_ResetFactoryEEPROM()
{
    unsigned short dummy = 0;
    sendCommand(cCDE_PWR_ELECTROBOT_RESET_FACTORY, dummy);
}

void CPowerElectrobot::onClicked_ChangeI2CAddress()
{
    sendCommand(cCDE_PWR_ELECTROBOT_CHANGE_I2C_ADDR, m_ihm.ui.new_i2c_address->value());
}
