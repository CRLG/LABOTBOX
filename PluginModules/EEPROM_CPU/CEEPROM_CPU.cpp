/*! \file CEEPROM_CPU.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CEEPROM_CPU.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CMessagerieBot.h"
#include "CTrameFactory.h"


/*! \addtogroup Module_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CEEPROM_CPU::CEEPROM_CPU(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_EEPROM_CPU, AUTEUR_EEPROM_CPU, INFO_EEPROM_CPU)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CEEPROM_CPU::~CEEPROM_CPU()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CEEPROM_CPU::init(CApplication *application)
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

  connect(m_ihm.ui.pushButton, SIGNAL(clicked(bool)), this, SLOT(onTest()));

}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CEEPROM_CPU::close(void)
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
void CEEPROM_CPU::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

// _____________________________________________________________________
void CEEPROM_CPU::onTest()
{
    CTrameBot *trame = m_application->m_MessagerieBot->getTrameFactory()->getTrameFromID(ID_READ_EEPROM_REQ);
    CTrame_READ_EEPROM_REQ *trame_eeprom_req = (CTrame_READ_EEPROM_REQ *)trame;
    trame_eeprom_req->start_address = 0;
    trame_eeprom_req->count = 100;
    trame->Encode();
}

