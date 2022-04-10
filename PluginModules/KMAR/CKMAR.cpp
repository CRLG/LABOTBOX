/*! \file CKMAR.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CKMAR.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"



/*! \addtogroup Module_KMAR
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CKMAR::CKMAR(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_KMAR, AUTEUR_KMAR, INFO_KMAR)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CKMAR::~CKMAR()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CKMAR::init(CApplication *application)
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

  connect(m_ihm.ui.send_mouvement, SIGNAL(clicked(bool)), this, SLOT(send_mouvement_clicked()));
  connect(m_ihm.ui.send_speed, SIGNAL(clicked(bool)), this, SLOT(send_vitesse_clicked()));
  connect(m_ihm.ui.disarm, SIGNAL(clicked(bool)), this, SLOT(send_disarm_clicked()));
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CKMAR::close(void)
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
void CKMAR::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

// ____________________________________________________
#define KMAR_CMD_MOUVEMENT  (1)
#define KMAR_CMD_VITESSE    (2)
#define KMAR_CMD_DISARM     (3)

void CKMAR::send_mouvement_clicked(void)
{
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", true);
    m_application->m_data_center->write("num_kmar", m_ihm.ui.num_kmar->value());
    m_application->m_data_center->write("cmd_kmar", KMAR_CMD_MOUVEMENT);
    m_application->m_data_center->write("value_cmd_kmar", m_ihm.ui.choix_mouvement->currentIndex()+1);
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", false);
}

void CKMAR::send_vitesse_clicked()
{
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", true);
    m_application->m_data_center->write("num_kmar", m_ihm.ui.num_kmar->value());
    m_application->m_data_center->write("cmd_kmar", KMAR_CMD_VITESSE);
    m_application->m_data_center->write("value_cmd_kmar", m_ihm.ui.facteur_vitesse->value());
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", false);
}

void CKMAR::send_disarm_clicked()
{
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", true);
    m_application->m_data_center->write("num_kmar", m_ihm.ui.num_kmar->value());
    m_application->m_data_center->write("cmd_kmar", KMAR_CMD_DISARM);
    m_application->m_data_center->write("value_cmd_kmar", 0);
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", false);
}



