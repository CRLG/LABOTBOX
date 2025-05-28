/*! \file CRobotPanelControl.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CRobotPanelControl.h"
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
CRobotPanelControl::CRobotPanelControl(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_RobotPanelControl, AUTEUR_RobotPanelControl, INFO_RobotPanelControl)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CRobotPanelControl::~CRobotPanelControl()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CRobotPanelControl::init(CApplication *application)
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

  connect(m_ihm.ui.ascenseur_haut, SIGNAL(clicked(bool)), this, SLOT(ascenseur_haut()));
  connect(m_ihm.ui.ascenseur_bas, SIGNAL(clicked(bool)), this, SLOT(ascenseur_bas()));
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CRobotPanelControl::close(void)
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
void CRobotPanelControl::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

// _____________________________________________________________________
void CRobotPanelControl::ascenseur_haut()
{
    send_command(ASCENSEUR_MONTE);
}

// _____________________________________________________________________
void CRobotPanelControl::ascenseur_bas()
{
    send_command(ASCENSEUR_DESCEND);
}

// _____________________________________________________________________
void CRobotPanelControl::send_command(unsigned long command, unsigned long value)
{
    CTrameBot *trame = m_application->m_MessagerieBot->getTrameFactory()->getTrameFromID(ID_ACTION_ROBOT);
    CTrame_ACTION_ROBOT *trame_action = (CTrame_ACTION_ROBOT *)trame;
    trame_action->command = command;
    trame_action->value = value;
    trame->Encode();
}

