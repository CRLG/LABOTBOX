/*! \file CXBEE.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CXBEE.h"
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
CXBEE::CXBEE(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_XBEE, AUTEUR_XBEE, INFO_XBEE)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CXBEE::~CXBEE()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CXBEE::init(CApplication *application)
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

  connect(m_ihm.ui.pushButton, SIGNAL(clicked(bool)), this,  SLOT(test()));
  connect(m_ihm.ui.init, SIGNAL(clicked(bool)), this,  SLOT(initXbee()));

  m_messenger.initApp(application);

  // Connexion avec le module m_rs232_xbee et fait le lien avec la fonction de decodage
  connect(m_application->m_RS232_xbee, SIGNAL(readyBytes(QByteArray)), this, SLOT(receiveSerialData(QByteArray)));
  connect(this, SIGNAL(SIG_sendToRS232(QByteArray)), m_application->m_RS232_xbee, SLOT(write(QByteArray)));
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CXBEE::close(void)
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
void CXBEE::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

// _____________________________________________________________________
void CXBEE::test()
{
    m_messenger.test();
}

// _____________________________________________________________________
void CXBEE::initXbee()
{
    m_messenger.initXbee();
}

// _____________________________________________________________________
void CXBEE::receiveSerialData(QByteArray datas)
{
    m_messenger.receiveSerialDatas(datas);
}


// _____________________________________________________________________
void CXBEE::sendSerialData(QByteArray datas)
{
    emit SIG_sendToRS232(datas);
}
