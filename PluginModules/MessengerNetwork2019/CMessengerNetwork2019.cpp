/*! \file CMessengerNetwork2019.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CMessengerNetwork2019.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CRS232.h"



/*! \addtogroup Module_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CMessengerNetwork2019::CMessengerNetwork2019(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_MessengerNetwork2019, AUTEUR_MessengerNetwork2019, INFO_MessengerNetwork2019)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CMessengerNetwork2019::~CMessengerNetwork2019()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CMessengerNetwork2019::init(CApplication *application)
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

  m_messenger.initApp(application);

  connect(m_application->m_RS232_xbee, SIGNAL(readyBytes(QByteArray)), this, SLOT(receiveSerialData(QByteArray)));

  connect(m_ihm.ui.pushButton, SIGNAL(pressed()), this, SLOT(test()));
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CMessengerNetwork2019::close(void)
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
void CMessengerNetwork2019::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}


// _____________________________________________________________________
void CMessengerNetwork2019::receiveSerialData(QByteArray datas)
{
    for (int i=0; i< datas.size(); i++) {
        m_messenger.decode(datas[i]);
    }
}

// _____________________________________________________________________
void CMessengerNetwork2019::test()
{
    m_messenger.test_TX();
}
