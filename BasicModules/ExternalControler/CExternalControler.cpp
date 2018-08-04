/*! \file CExternalControler.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QByteArray>

#include "CExternalControler.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CInstructionsSet.h"
#include "CNetworkServerFactory.h"

/*! \addtogroup ExternalControler
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CExternalControler::CExternalControler(const char *plugin_name)
    :CBasicModule(plugin_name, VERSION_ExternalControler, AUTEUR_ExternalControler, INFO_ExternalControler),
      m_server(NULL),
      m_instructions_set(NULL)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CExternalControler::~CExternalControler()
{
}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CExternalControler::init(CApplication *application)
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

  // Command interpreter
  m_instructions_set = new CInstructionsSet(application);
  if (!m_instructions_set) {
      m_application->m_print_view->print_error(this, "Impossible de créer l'interpreteur de commande");
  }

  // Server
  // Toutes les options du serveur (clé / valeur par défaut si absent du fichier EEPROM)
  // Les noms sont identiques à ceux du fichier EEPROM.ini
  m_server_config.insert("server_type", "tcp");
  m_server_config.insert("port", "1234");
  // Lecture de chacune des options du serveur dans le fichier EEPROM
  for (tNtwConfig::iterator it=m_server_config.begin(); it!=m_server_config.end(); it++)
  {
      m_server_config[it.key()] = m_application->m_eeprom->read(getName(), it.key(), it.value());
  }

  m_server = CNetworksServerFactory::createInstance(m_server_config["server_type"].toString(), this);
  if (m_server) {
      connect(m_server, SIGNAL(opened(QString)), this, SLOT(srv_opened(QString)));
      connect(m_server, SIGNAL(errorMessage(QString)), this, SLOT(srv_errorMessage(QString)));
      connect(m_server, SIGNAL(receivedData(int,QByteArray)), this, SLOT(srv_receivedData(int,QByteArray)));
      connect(m_server, SIGNAL(bytesWritten(int,qint64)), this, SLOT(srv_bytesWritten(int,qint64)));
      connect(m_server, SIGNAL(connected(int)), this, SLOT(srv_newConnection(int)));
      connect(m_ihm.ui.pushButton, SIGNAL(released()), m_server, SLOT(close()));
      m_server->open(m_server_config);
  }
  else {
      QString srv_options;
      for (tNtwConfig::iterator it=m_server_config.begin(); it!=m_server_config.end(); it++) {
          srv_options+= QString("[%1]=%2 / ").arg(it.key()).arg(it.value().toString());
      }
      m_application->m_print_view->print_error(this, "Impossible de créer le server: " + srv_options);
  }
}

// _____________________________________________________________________
void CExternalControler::srv_bytesWritten(int client_id, qint64 bytes)
{
    Q_UNUSED(client_id)
    Q_UNUSED(bytes)
    //qDebug() << "CExternalControler::bytesWritten" << cliend_id << bytes;
}

// _____________________________________________________________________
void CExternalControler::srv_receivedData(int client_id, QByteArray data)
{
    //qDebug() << "CExternalControler::srv_receivedData [" << client_id << "]" << data;
    m_application->m_print_view->print_info(this, data);

    QByteArray ret = m_instructions_set->execute(data);
    m_server->write(client_id, ret);
}
// _____________________________________________________________________
void CExternalControler::srv_opened(QString msg)
{
    m_application->m_print_view->print_info(this, msg);
}

// _____________________________________________________________________
void CExternalControler::srv_newConnection(int client_id)
{
    QString msg = "New client connected : " + QString::number(client_id);
    m_application->m_print_view->print_info(this, msg);

    msg = "Connected to ExternalContoler. Your ID is " + QString::number((client_id));
    m_server->write(client_id, msg.toUtf8());
}
// _____________________________________________________________________
void CExternalControler::srv_errorMessage(QString msg)
{
    m_application->m_print_view->print_error(this, msg);
    qDebug() << msg;
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CExternalControler::close(void)
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
void CExternalControler::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}
