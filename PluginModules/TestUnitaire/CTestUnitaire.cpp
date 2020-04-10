/*! \file CTestUnitaire.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CTestUnitaire.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CRS232.h"
#include "CMessagerieBot.h"



/*! \addtogroup Module_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CTestUnitaire::CTestUnitaire(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_TestUnitaire, AUTEUR_TestUnitaire, INFO_TestUnitaire)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CTestUnitaire::~CTestUnitaire()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CTestUnitaire::init(CApplication *application)
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

 //bool ret =  m_application->m_RS232_robot->openRS232("COM1");

 tConfigRS232 config;
 //m_application->m_RS232_robot->getConfigRS232(&config);

 m_application->m_eeprom->write(getName(), "baudrate", QVariant((int)config.baudrate));
 m_application->m_eeprom->write(getName(), "databits", QVariant((int)config.databits));
 m_application->m_eeprom->write(getName(), "flowcontrol", QVariant((int)config.flowcontrol));
 m_application->m_eeprom->write(getName(), "parity", QVariant((int)config.parity));
 m_application->m_eeprom->write(getName(), "stopbits", QVariant((int)config.stopbits));

 val = m_application->m_eeprom->read(getName(), "baudrate", 115200);
 val = m_application->m_eeprom->read(getName(), "databits", 8);
 val = m_application->m_eeprom->read(getName(), "flowcontrol", 0);
 val = m_application->m_eeprom->read(getName(), "parity", 0);
 val = m_application->m_eeprom->read(getName(), "stopbits", 0);

 connect(m_ihm.ui.pushButton, SIGNAL(clicked()), this, SLOT(cb_Bouton()));
 connect(m_ihm.ui.pushButton2, SIGNAL(clicked()), this, SLOT(cb_Bouton2()));
 connect(m_ihm.ui.pushButton3, SIGNAL(clicked()), this, SLOT(cb_Bouton3()));

 m_application->m_data_center->write("PosX_robot", 0);
 m_application->m_data_center->getData("PosX_robot")->setProperty("Unite", "secondes");
 m_application->m_data_center->getData("PosX_robot")->setProperty("Etat", "ON");

 m_application->m_data_center->write("PosY_robot", 0);
 m_application->m_data_center->getData("PosY_robot")->setProperty("Format", "float");
 m_application->m_data_center->getData("PosY_robot")->setProperty("Facteur", "2.34");


 CData myData;

 myData.write(34.45);
 myData.setProperty("Unite", "seconde");
 myData.setProperty("Facteur", "3");
 myData.setProperty("Facteurs", "14");
 QStringList liste_proprietes;
 myData.getPropertiesList(liste_proprietes);
 //qDebug() << liste_proprietes;
 //qDebug() << myData.getPropertiesString();



 QStringList var_list;
 m_application->m_data_center->getListeVariablesName(var_list);
 for (int i=0; i<var_list.size(); i++) {
   QString str="";
   str = "VARIABLE: " + var_list.at(i) + "\n";
   str+= m_application->m_data_center->getData(var_list.at(i))->getPropertiesString();
   m_application->m_print_view->print_info(this, str);
 }


 connect(m_application->m_RS232_robot, SIGNAL(readyBytes(QByteArray)), this, SLOT(receive_rs232(QByteArray)));
 connect(m_application->m_RS232_robot, SIGNAL(connected()), this, SLOT(connected_rs232()));
 connect(m_application->m_RS232_robot, SIGNAL(disconnected()), this, SLOT(disconnected_rs232()));

 connect(m_application->m_MessagerieBot, SIGNAL(connected(bool)), this, SLOT(connected_to_robot(bool)));

 connect(&m_external_client, SIGNAL(connected()), this, SLOT(connected_to_external()));
 connect(&m_external_client, SIGNAL(disconnected()), this, SLOT(disconnected_to_external()));
}






// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CTestUnitaire::close(void)
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
void CTestUnitaire::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}


void CTestUnitaire::cb_Bouton(void)
{
 for (char i='0'; i<='9'; i++) {
    // m_application->m_RS232_robot->write(i);
 }

#define _SIZE_BUFF_ 40000
 char buf[_SIZE_BUFF_];
 char car = 'A';
 for (unsigned int i=0; i<_SIZE_BUFF_; i++) {
     buf[i] = car;
     //m_application->m_RS232_robot->write(car);
     if (++car > 'Z') { car = 'A'; }
 }
 //m_application->m_RS232_robot->write(buf, _SIZE_BUFF_);

 QByteArray data(buf, 11000);
 m_application->m_RS232_robot->write(data);


 if (m_external_client.isConnected())   m_external_client.close();
 else                                   m_external_client.open("127.0.0.1", 1236);
}


void CTestUnitaire::cb_Bouton2(void)
{
  tStructTrameBrute trame;

  trame.ID = 0x123;
  trame.DLC = 7;
  trame.Data[0] = 1;
  trame.Data[1] = 2;
  trame.Data[2] = 3;
  trame.Data[3] = 4;
  trame.Data[4] = 5;
  trame.Data[5] = 6;
  m_application->m_MessagerieBot->SerialiseTrame(&trame);
}

void CTestUnitaire::cb_Bouton3(void)
{
    m_application->m_RS232_robot->reinitSerial();
}

void CTestUnitaire::receive_rs232(QByteArray data)
{
  m_application->m_print_view->print_info(this, QString(data));
}


void CTestUnitaire::connected_rs232(void)
{
  m_application->m_print_view->print_info(this, "RS232 connected");
}

void CTestUnitaire::disconnected_rs232(void)
{
  m_application->m_print_view->print_info(this, "RS232 disconnected");
}


// =========================================================

void CTestUnitaire::connected_to_robot(bool state)
{
 if (state) m_application->m_print_view->print_info(this, "Robot connected");
 else       m_application->m_print_view->print_info(this, "Robot disconnected");
}



void CTestUnitaire::connected_to_external()
{
    qDebug() << "Connected to External Client";
}
void CTestUnitaire::disconnected_to_external()
{
    qDebug() << "Disconnected to External Client";
}
