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

  //connect(m_ihm.ui.ascenseur_haut, SIGNAL(clicked(bool)), this, SLOT(ascenseur_haut()));
  //connect(m_ihm.ui.ascenseur_bas, SIGNAL(clicked(bool)), this, SLOT(ascenseur_bas()));

  // Code inline dans les connect pour faciliter l'écriture du code
  QObject::connect(m_ihm.ui.ascenseur_haut,             &QPushButton::clicked, [this]() { send_command(ASCENSEUR_MONTE); } );
  QObject::connect(m_ihm.ui.ascenseur_bas,              &QPushButton::clicked, [this]() { send_command(ASCENSEUR_DESCEND); } );

  QObject::connect(m_ihm.ui.pince_arg_fermee,           &QPushButton::clicked, [this]() { send_command(PINCE_ARG_FERMEE); } );
  QObject::connect(m_ihm.ui.pince_arg_ouverte,          &QPushButton::clicked, [this]() { send_command(PINCE_ARG_OUVERTE); } );
  QObject::connect(m_ihm.ui.pince_arg_intermediaire,    &QPushButton::clicked, [this]() { send_command(PINCE_ARG_INTERMEDIAIRE); } );

  QObject::connect(m_ihm.ui.pince_ard_fermee,           &QPushButton::clicked, [this]() { send_command(PINCE_ARD_FERMEE); } );
  QObject::connect(m_ihm.ui.pince_ard_ouverte,          &QPushButton::clicked, [this]() { send_command(PINCE_ARD_OUVERTE); } );
  QObject::connect(m_ihm.ui.pince_ard_intermediaire,    &QPushButton::clicked, [this]() { send_command(PINCE_ARD_INTERMEDIAIRE); } );

  QObject::connect(m_ihm.ui.pince_planche_fermee,       &QPushButton::clicked, [this]() { send_command(PINCE_PLANCHE_FERMEE); } );
  QObject::connect(m_ihm.ui.pince_planche_ouverte,      &QPushButton::clicked, [this]() { send_command(PINCE_PLANCHE_OUVERTE); } );
  QObject::connect(m_ihm.ui.pince_planche_repos,        &QPushButton::clicked, [this]() { send_command(PINCE_PLANCHE_INTERMEDIAIRE); } );

  QObject::connect(m_ihm.ui.banderole_rangee,           &QPushButton::clicked, [this]() { send_command(BANDEROLE_RANGEE); } );
  QObject::connect(m_ihm.ui.banderole_deployee,         &QPushButton::clicked, [this]() { send_command(BANDEROLE_DEPLOYEE); } );
  QObject::connect(m_ihm.ui.banderole_horizontale,      &QPushButton::clicked, [this]() { send_command(BANDEROLE_INTERMEDIAIRE); } );

  QObject::connect(m_ihm.ui.verrin_haut,                &QPushButton::clicked, [this]() { send_command(VERRIN_HAUT); } );
  QObject::connect(m_ihm.ui.verrin_bas,                 &QPushButton::clicked, [this]() { send_command(VERRIN_BAS); } );
  QObject::connect(m_ihm.ui.verrin_intermediaire,       &QPushButton::clicked, [this]() { send_command(VERRIN_INTERMEDIAIRE); } );

  QObject::connect(m_ihm.ui.can_int_off,                &QPushButton::clicked, [this]() { send_command(CAN_MOVER_INT_OFF); } );
  QObject::connect(m_ihm.ui.can_int_on,                 &QPushButton::clicked, [this]() { send_command(CAN_MOVER_INT_ON); } );

  QObject::connect(m_ihm.ui.can_ext_off,                &QPushButton::clicked, [this]() { send_command(CAN_MOVER_EXT_OFF); } );
  QObject::connect(m_ihm.ui.can_ext_on,                 &QPushButton::clicked, [this]() { send_command(CAN_MOVER_EXT_ON); } );
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
void CRobotPanelControl::send_command(unsigned long command, unsigned long value)
{
    CTrameBot *trame = m_application->m_MessagerieBot->getTrameFactory()->getTrameFromID(ID_ACTION_ROBOT);
    CTrame_ACTION_ROBOT *trame_action = (CTrame_ACTION_ROBOT *)trame;
    trame_action->command = command;
    trame_action->value = value;
    trame->Encode();
}

