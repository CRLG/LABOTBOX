/*! \file CSensorElectroBot.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CSensorElectroBot.h"
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
CSensorElectroBot::CSensorElectroBot(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_SensorElectroBot, AUTEUR_SensorElectroBot, INFO_SensorElectroBot)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CSensorElectroBot::~CSensorElectroBot()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CSensorElectroBot::init(CLaBotBox *application)
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

  // Connexions avec la messagerie
  m_application->m_data_center->write("Etor1", 0);
  m_application->m_data_center->write("Etor2", 0);
  m_application->m_data_center->write("Etor3", 0);
  m_application->m_data_center->write("Etor4", 0);
  m_application->m_data_center->write("Etor5", 0);
  m_application->m_data_center->write("Etor6", 0);
  m_application->m_data_center->write("Etor_PGED1_dsPIC2", 0);
  m_application->m_data_center->write("Etor_PGED1_dsPIC1", 0);
  m_application->m_data_center->write("Etor_PGEC1_dsPIC2", 0);
  m_application->m_data_center->write("Etor_PGEC1_dsPIC1", 0);
  m_application->m_data_center->write("Etor_Codeur4_B", 0);
  m_application->m_data_center->write("Etor_Codeur4_A", 0);
  m_application->m_data_center->write("Etor_Codeur3_B", 0);
  m_application->m_data_center->write("Etor_Codeur3_A", 0);
  m_application->m_data_center->write("Etor_CAN_TX", 0);
  m_application->m_data_center->write("Etor_CAN_RX", 0);
  m_application->m_data_center->write("Eana1", 0);
  m_application->m_data_center->write("Eana2", 0);
  m_application->m_data_center->write("Eana3", 0);
  m_application->m_data_center->write("Eana4", 0);
  m_application->m_data_center->write("Eana5", 0);
  m_application->m_data_center->write("Eana6", 0);
  m_application->m_data_center->write("Eana7", 0);
  m_application->m_data_center->write("Eana8", 0);
  m_application->m_data_center->write("Eana9", 0);
  m_application->m_data_center->write("Eana10", 0);
  m_application->m_data_center->write("Eana11", 0);
  m_application->m_data_center->write("Eana12", 0);
  m_application->m_data_center->write("Eana13", 0);
  m_application->m_data_center->write("Vbat", 0);
  m_application->m_data_center->write("Codeur_1", 0);
  m_application->m_data_center->write("Codeur_2", 0);
  m_application->m_data_center->write("Codeur_3", 0);
  m_application->m_data_center->write("Codeur_4", 0);



  connect(m_application->m_data_center->getData("Etor1"), SIGNAL(valueChanged(QVariant)), this, SLOT(Etor1_changed(QVariant)));
  connect(m_application->m_data_center->getData("Etor2"), SIGNAL(valueChanged(QVariant)), this, SLOT(Etor2_changed(QVariant)));
  connect(m_application->m_data_center->getData("Etor3"), SIGNAL(valueChanged(QVariant)), this, SLOT(Etor3_changed(QVariant)));
  connect(m_application->m_data_center->getData("Etor4"), SIGNAL(valueChanged(QVariant)), this, SLOT(Etor4_changed(QVariant)));
  connect(m_application->m_data_center->getData("Etor5"), SIGNAL(valueChanged(QVariant)), this, SLOT(Etor5_changed(QVariant)));
  connect(m_application->m_data_center->getData("Etor6"), SIGNAL(valueChanged(QVariant)), this, SLOT(Etor6_changed(QVariant)));

  connect(m_application->m_data_center->getData("Etor_PGED1_dsPIC2"), SIGNAL(valueChanged(QVariant)), this, SLOT(Etor_PGED1_dsPIC2_changed(QVariant)));
  connect(m_application->m_data_center->getData("Etor_PGED1_dsPIC1"), SIGNAL(valueChanged(QVariant)), this, SLOT(Etor_PGED1_dsPIC1_changed(QVariant)));
  connect(m_application->m_data_center->getData("Etor_PGEC1_dsPIC2"), SIGNAL(valueChanged(QVariant)), this, SLOT(Etor_PGEC1_dsPIC2_changed(QVariant)));
  connect(m_application->m_data_center->getData("Etor_PGEC1_dsPIC1"), SIGNAL(valueChanged(QVariant)), this, SLOT(Etor_PGEC1_dsPIC1_changed(QVariant)));
  connect(m_application->m_data_center->getData("Etor_Codeur4_B"), SIGNAL(valueChanged(QVariant)), this, SLOT(Etor_Codeur4_B_changed(QVariant)));
  connect(m_application->m_data_center->getData("Etor_Codeur4_A"), SIGNAL(valueChanged(QVariant)), this, SLOT(Etor_Codeur4_A_changed(QVariant)));
  connect(m_application->m_data_center->getData("Etor_Codeur3_B"), SIGNAL(valueChanged(QVariant)), this, SLOT(Etor_Codeur3_B_changed(QVariant)));
  connect(m_application->m_data_center->getData("Etor_Codeur3_A"), SIGNAL(valueChanged(QVariant)), this, SLOT(Etor_Codeur3_A_changed(QVariant)));
  connect(m_application->m_data_center->getData("Etor_CAN_TX"), SIGNAL(valueChanged(QVariant)), this, SLOT(Etor_CAN_TX_changed(QVariant)));
  connect(m_application->m_data_center->getData("Etor_CAN_RX"), SIGNAL(valueChanged(QVariant)), this, SLOT(Etor_CAN_RX_changed(QVariant)));

  connect(m_application->m_data_center->getData("Eana1"), SIGNAL(valueChanged(QVariant)), this, SLOT(Eana1_changed(QVariant)));
  connect(m_application->m_data_center->getData("Eana2"), SIGNAL(valueChanged(QVariant)), this, SLOT(Eana2_changed(QVariant)));
  connect(m_application->m_data_center->getData("Eana3"), SIGNAL(valueChanged(QVariant)), this, SLOT(Eana3_changed(QVariant)));
  connect(m_application->m_data_center->getData("Eana4"), SIGNAL(valueChanged(QVariant)), this, SLOT(Eana4_changed(QVariant)));
  connect(m_application->m_data_center->getData("Eana5"), SIGNAL(valueChanged(QVariant)), this, SLOT(Eana5_changed(QVariant)));
  connect(m_application->m_data_center->getData("Eana6"), SIGNAL(valueChanged(QVariant)), this, SLOT(Eana6_changed(QVariant)));
  connect(m_application->m_data_center->getData("Eana7"), SIGNAL(valueChanged(QVariant)), this, SLOT(Eana7_changed(QVariant)));
  connect(m_application->m_data_center->getData("Eana8"), SIGNAL(valueChanged(QVariant)), this, SLOT(Eana8_changed(QVariant)));
  connect(m_application->m_data_center->getData("Eana9"), SIGNAL(valueChanged(QVariant)), this, SLOT(Eana9_changed(QVariant)));
  connect(m_application->m_data_center->getData("Eana10"), SIGNAL(valueChanged(QVariant)), this, SLOT(Eana10_changed(QVariant)));
  connect(m_application->m_data_center->getData("Eana11"), SIGNAL(valueChanged(QVariant)), this, SLOT(Eana11_changed(QVariant)));
  connect(m_application->m_data_center->getData("Eana12"), SIGNAL(valueChanged(QVariant)), this, SLOT(Eana12_changed(QVariant)));
  connect(m_application->m_data_center->getData("Eana13"), SIGNAL(valueChanged(QVariant)), this, SLOT(Eana13_changed(QVariant)));
  connect(m_application->m_data_center->getData("Vbat"), SIGNAL(valueChanged(QVariant)), this, SLOT(Vbat_changed(QVariant)));

  connect(m_application->m_data_center->getData("Codeur_1"), SIGNAL(valueChanged(QVariant)), this, SLOT(Codeur_1_changed(QVariant)));
  connect(m_application->m_data_center->getData("Codeur_2"), SIGNAL(valueChanged(QVariant)), this, SLOT(Codeur_2_changed(QVariant)));
  connect(m_application->m_data_center->getData("Codeur_3"), SIGNAL(valueChanged(QVariant)), this, SLOT(Codeur_3_changed(QVariant)));
  connect(m_application->m_data_center->getData("Codeur_4"), SIGNAL(valueChanged(QVariant)), this, SLOT(Codeur_4_changed(QVariant)));

  // Met à jour les labels en fonction de la propriété "Alias" de chaque data
  // Si la data n'a pas d'Alias dans  le data_center,  le label reste celui par défaut
  updateAliasLabels();
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CSensorElectroBot::close(void)
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
void CSensorElectroBot::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

// _____________________________________________________________________
void CSensorElectroBot::updateAliasLabels(void)
{
    // Gestion des alias sur les noms de capteurs
    QString str_val;
    str_val = m_application->m_data_center->getDataProperty("Etor1", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Etor_1->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Etor2", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Etor_2->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Etor3", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Etor_3->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Etor4", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Etor_4->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Etor5", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Etor_5->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Etor6", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Etor_6->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Etor_PGED1_dsPIC2", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Etor_PGED1_dsPIC2->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Etor_PGED1_dsPIC1", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Etor_PGED1_dsPIC1->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Etor_PGEC1_dsPIC2", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Etor_PGEC1_dsPIC2->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Etor_PGEC1_dsPIC1", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Etor_PGEC1_dsPIC1->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Etor_Codeur4_B", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Etor_Codeur4_B->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Etor_Codeur4_A", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Etor_Codeur4_A->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Etor_Codeur3_B", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Etor_Codeur3_B->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Etor_Codeur3_A", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Etor_Codeur3_A->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Etor_CAN_TX", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Etor_CAN_TX->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Etor_CAN_RX", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Etor_CAN_RX->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Eana1", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Eana_1->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Eana2", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Eana_2->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Eana3", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Eana_3->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Eana4", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Eana_4->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Eana5", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Eana_5->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Eana6", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Eana_6->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Eana7", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Eana_7->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Eana8", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Eana_8->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Eana9", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Eana_9->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Eana10", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Eana_10->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Eana11", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Eana_11->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Eana12", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Eana_12->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Eana13", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Eana_13->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Codeur_1", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Codeur_1->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Codeur_2", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Codeur_2->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Codeur_3", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Codeur_3->setText(str_val); }

    str_val = m_application->m_data_center->getDataProperty("Codeur_4", "Alias").toString();
    if (str_val != "") { m_ihm.ui.label_Codeur_4->setText(str_val); }
}

// _____________________________________________________________________
void CSensorElectroBot::Etor1_changed(QVariant val) { m_ihm.ui.Etor_1->setValue(val.toBool()); }
void CSensorElectroBot::Etor2_changed(QVariant val) { m_ihm.ui.Etor_2->setValue(val.toBool()); }
void CSensorElectroBot::Etor3_changed(QVariant val) { m_ihm.ui.Etor_3->setValue(val.toBool()); }
void CSensorElectroBot::Etor4_changed(QVariant val) { m_ihm.ui.Etor_4->setValue(val.toBool()); }
void CSensorElectroBot::Etor5_changed(QVariant val) { m_ihm.ui.Etor_5->setValue(val.toBool()); }
void CSensorElectroBot::Etor6_changed(QVariant val) { m_ihm.ui.Etor_6->setValue(val.toBool()); }

void CSensorElectroBot::Etor_PGED1_dsPIC2_changed(QVariant val) { m_ihm.ui.Etor_PGED1_dsPIC2->setValue(val.toBool()); }
void CSensorElectroBot::Etor_PGED1_dsPIC1_changed(QVariant val) { m_ihm.ui.Etor_PGED1_dsPIC1->setValue(val.toBool()); }
void CSensorElectroBot::Etor_PGEC1_dsPIC2_changed(QVariant val) { m_ihm.ui.Etor_PGEC1_dsPIC2->setValue(val.toBool()); }
void CSensorElectroBot::Etor_PGEC1_dsPIC1_changed(QVariant val) { m_ihm.ui.Etor_PGEC1_dsPIC1->setValue(val.toBool()); }
void CSensorElectroBot::Etor_Codeur4_B_changed(QVariant val)    { m_ihm.ui.Etor_Codeur4_B->setValue(val.toBool()); }
void CSensorElectroBot::Etor_Codeur4_A_changed(QVariant val)    { m_ihm.ui.Etor_Codeur4_A->setValue(val.toBool()); }
void CSensorElectroBot::Etor_Codeur3_B_changed(QVariant val)    { m_ihm.ui.Etor_Codeur3_B->setValue(val.toBool()); }
void CSensorElectroBot::Etor_Codeur3_A_changed(QVariant val)    { m_ihm.ui.Etor_Codeur3_A->setValue(val.toBool()); }
void CSensorElectroBot::Etor_CAN_TX_changed(QVariant val)       { m_ihm.ui.Etor_CAN_TX->setValue(val.toBool()); }
void CSensorElectroBot::Etor_CAN_RX_changed(QVariant val)       { m_ihm.ui.Etor_CAN_RX->setValue(val.toBool()); }

void CSensorElectroBot::Eana1_changed(QVariant val)     { m_ihm.ui.Eana_1->setValue(val.toDouble()); }
void CSensorElectroBot::Eana2_changed(QVariant val)     { m_ihm.ui.Eana_2->setValue(val.toDouble()); }
void CSensorElectroBot::Eana3_changed(QVariant val)     { m_ihm.ui.Eana_3->setValue(val.toDouble()); }
void CSensorElectroBot::Eana4_changed(QVariant val)     { m_ihm.ui.Eana_4->setValue(val.toDouble()); }
void CSensorElectroBot::Eana5_changed(QVariant val)     { m_ihm.ui.Eana_5->setValue(val.toDouble()); }
void CSensorElectroBot::Eana6_changed(QVariant val)     { m_ihm.ui.Eana_6->setValue(val.toDouble()); }
void CSensorElectroBot::Eana7_changed(QVariant val)     { m_ihm.ui.Eana_7->setValue(val.toDouble()); }
void CSensorElectroBot::Eana8_changed(QVariant val)     { m_ihm.ui.Eana_8->setValue(val.toDouble()); }
void CSensorElectroBot::Eana9_changed(QVariant val)     { m_ihm.ui.Eana_9->setValue(val.toDouble()); }
void CSensorElectroBot::Eana10_changed(QVariant val)    { m_ihm.ui.Eana_10->setValue(val.toDouble()); }
void CSensorElectroBot::Eana11_changed(QVariant val)    { m_ihm.ui.Eana_11->setValue(val.toDouble()); }
void CSensorElectroBot::Eana12_changed(QVariant val)    { m_ihm.ui.Eana_12->setValue(val.toDouble()); }
void CSensorElectroBot::Eana13_changed(QVariant val)    { m_ihm.ui.Eana_13->setValue(val.toDouble()); }
void CSensorElectroBot::Vbat_changed(QVariant val)      { m_ihm.ui.Vbat->setValue(val.toDouble()); }

void CSensorElectroBot::Codeur_1_changed(QVariant val)  { m_ihm.ui.Codeur_1->setValue(val.toInt()); }
void CSensorElectroBot::Codeur_2_changed(QVariant val)  { m_ihm.ui.Codeur_2->setValue(val.toInt()); }
void CSensorElectroBot::Codeur_3_changed(QVariant val)  { m_ihm.ui.Codeur_3->setValue(val.toInt()); }
void CSensorElectroBot::Codeur_4_changed(QVariant val)  { m_ihm.ui.Codeur_4->setValue(val.toInt()); }


