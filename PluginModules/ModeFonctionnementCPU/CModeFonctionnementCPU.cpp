/*! \file CModeFonctionnementCPU.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CModeFonctionnementCPU.h"
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
CModeFonctionnementCPU::CModeFonctionnementCPU(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_ModeFonctionnementCPU, AUTEUR_ModeFonctionnementCPU, INFO_ModeFonctionnementCPU)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CModeFonctionnementCPU::~CModeFonctionnementCPU()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CModeFonctionnementCPU::init(CApplication *application)
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

  connect(m_ihm.ui.pb_send, SIGNAL(clicked(bool)), this, SLOT(on_send()));
  connect(m_ihm.ui.pb_reset_cpu, SIGNAL(clicked(bool)), this, SLOT(on_reset_cpu()));
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CModeFonctionnementCPU::close(void)
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
void CModeFonctionnementCPU::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

// _____________________________________________________________________
void CModeFonctionnementCPU::on_send()
{
    m_application->m_data_center->write("COMMANDE_MODE_FONCTIONNEMENT_CPU_TxSync", true);
    m_application->m_data_center->write("CommandeModeFonctionnementCPU", m_ihm.ui.ModeFonctionnement->currentIndex());
    m_application->m_data_center->write("COMMANDE_MODE_FONCTIONNEMENT_CPU_TxSync", false);
}

// _____________________________________________________________________
const unsigned int SECURE_CODE_RESET_CPU = 0x69;
void CModeFonctionnementCPU::on_reset_cpu()
{
    m_application->m_data_center->write("RESET_CPU_TxSync", true);
    m_application->m_data_center->write("SECURITE_RESET_CPU", SECURE_CODE_RESET_CPU);
    m_application->m_data_center->write("RESET_CPU_TxSync", false);
}
