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
  connect(m_ihm.ui.disarm, SIGNAL(clicked(bool)), this, SLOT(send_stop_and_disarm_clicked()));
  connect(m_ihm.ui.arm, SIGNAL(clicked(bool)), this, SLOT(send_arm()));
  connect(m_ihm.ui.fix_position, SIGNAL(clicked(bool)), this, SLOT(send_stop_and_fix_clicked()));
  connect(m_ihm.ui.arret_urgence, SIGNAL(clicked(bool)), this, SLOT(send_stop_and_disarm_clicked()));
  connect(m_ihm.ui.disarm_1, SIGNAL(clicked(bool)), this, SLOT(send_disarm_axis_1()));
  connect(m_ihm.ui.arm_1, SIGNAL(clicked(bool)), this, SLOT(send_arm_axis_1()));
  connect(m_ihm.ui.disarm_2, SIGNAL(clicked(bool)), this, SLOT(send_disarm_axis_2()));
  connect(m_ihm.ui.arm_2, SIGNAL(clicked(bool)), this, SLOT(send_arm_axis_2()));
  connect(m_ihm.ui.disarm_3, SIGNAL(clicked(bool)), this, SLOT(send_disarm_axis_3()));
  connect(m_ihm.ui.arm_3, SIGNAL(clicked(bool)), this, SLOT(send_arm_axis_3()));
  connect(m_ihm.ui.disarm_4, SIGNAL(clicked(bool)), this, SLOT(send_disarm_axis_4()));
  connect(m_ihm.ui.arm_4, SIGNAL(clicked(bool)), this, SLOT(send_arm_axis_4()));

  int num_kmar = m_application->m_eeprom->read(getName(), "num_kmar", 1).toInt();  // pour prévoir plusieurs instances du module pour chaque Kmar
  QString kmar_name = QString("Kmar%1").arg(num_kmar);
  connect(m_application->m_data_center->getData(QString("%1.axis1.moving").arg(kmar_name)), SIGNAL(valueChanged(bool)), m_ihm.ui.mvt_axe_1, SLOT(setValue(bool)));
  connect(m_application->m_data_center->getData(QString("%1.axis2.moving").arg(kmar_name)), SIGNAL(valueChanged(bool)), m_ihm.ui.mvt_axe_2, SLOT(setValue(bool)));
  connect(m_application->m_data_center->getData(QString("%1.axis3.moving").arg(kmar_name)), SIGNAL(valueChanged(bool)), m_ihm.ui.mvt_axe_3, SLOT(setValue(bool)));
  connect(m_application->m_data_center->getData(QString("%1.axis4.moving").arg(kmar_name)), SIGNAL(valueChanged(bool)), m_ihm.ui.mvt_axe_4, SLOT(setValue(bool)));
  connect(m_application->m_data_center->getData(QString("%1.axis1.position").arg(kmar_name)), SIGNAL(valueChanged(int)), m_ihm.ui.pos_axe1, SLOT(setValue(int)));
  connect(m_application->m_data_center->getData(QString("%1.axis2.position").arg(kmar_name)), SIGNAL(valueChanged(int)), m_ihm.ui.pos_axe2, SLOT(setValue(int)));
  connect(m_application->m_data_center->getData(QString("%1.axis3.position").arg(kmar_name)), SIGNAL(valueChanged(int)), m_ihm.ui.pos_axe3, SLOT(setValue(int)));
  connect(m_application->m_data_center->getData(QString("%1.axis4.position").arg(kmar_name)), SIGNAL(valueChanged(int)), m_ihm.ui.pos_axe4, SLOT(setValue(int)));
  connect(m_application->m_data_center->getData(QString("%1.moving").arg(kmar_name)), SIGNAL(valueChanged(bool)), m_ihm.ui.mouvement_en_cours, SLOT(setValue(bool)));
  connect(m_application->m_data_center->getData(QString("%1.num_mouvement_en_cours").arg(kmar_name)), SIGNAL(valueChanged(int)), m_ihm.ui.num_mouvement_en_cours, SLOT(setValue(int)));
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
// TODO : rendre commun ces définitions avec le robot
typedef enum {
    KMAR_CMD_MOUVEMENT = 1,
    KMAR_CMD_VITESSE,
    KMAR_CMD_STOP_AND_FIX_POSITION,
    KMAR_CMD_STOP_AND_DISARM_ALL,
    KMAR_CMD_ARM_ALL,
    KMAR_CMD_DISARM_AXIS,
    KMAR_CMD_ARM_AXIS
}KmarCmd;

typedef enum {
    AXIS_1 = 0,
    AXIS_2,
    AXIS_3,
    AXIS_4,
}tKmarAxis;

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

void CKMAR::send_stop_and_disarm_clicked()
{
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", true);
    m_application->m_data_center->write("num_kmar", m_ihm.ui.num_kmar->value());
    m_application->m_data_center->write("cmd_kmar", KMAR_CMD_STOP_AND_DISARM_ALL);
    m_application->m_data_center->write("value_cmd_kmar", 0);
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", false);
}

void CKMAR::send_stop_and_fix_clicked()
{
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", true);
    m_application->m_data_center->write("num_kmar", m_ihm.ui.num_kmar->value());
    m_application->m_data_center->write("cmd_kmar", KMAR_CMD_STOP_AND_FIX_POSITION);
    m_application->m_data_center->write("value_cmd_kmar", 0);
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", false);
}

void CKMAR::send_arm()
{
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", true);
    m_application->m_data_center->write("num_kmar", m_ihm.ui.num_kmar->value());
    m_application->m_data_center->write("cmd_kmar", KMAR_CMD_ARM_ALL);
    m_application->m_data_center->write("value_cmd_kmar", AXIS_1);
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", false);
}

void CKMAR::send_disarm_axis_1()
{
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", true);
    m_application->m_data_center->write("num_kmar", m_ihm.ui.num_kmar->value());
    m_application->m_data_center->write("cmd_kmar", KMAR_CMD_DISARM_AXIS);
    m_application->m_data_center->write("value_cmd_kmar", AXIS_1);
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", false);
}

void CKMAR::send_arm_axis_1()
{
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", true);
    m_application->m_data_center->write("num_kmar", m_ihm.ui.num_kmar->value());
    m_application->m_data_center->write("cmd_kmar", KMAR_CMD_ARM_AXIS);
    m_application->m_data_center->write("value_cmd_kmar", AXIS_1);
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", false);
}

void CKMAR::send_disarm_axis_2()
{
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", true);
    m_application->m_data_center->write("num_kmar", m_ihm.ui.num_kmar->value());
    m_application->m_data_center->write("cmd_kmar", KMAR_CMD_DISARM_AXIS);
    m_application->m_data_center->write("value_cmd_kmar", AXIS_2);
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", false);
}

void CKMAR::send_arm_axis_2()
{
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", true);
    m_application->m_data_center->write("num_kmar", m_ihm.ui.num_kmar->value());
    m_application->m_data_center->write("cmd_kmar", KMAR_CMD_ARM_AXIS);
    m_application->m_data_center->write("value_cmd_kmar", AXIS_2);
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", false);
}

void CKMAR::send_disarm_axis_3()
{
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", true);
    m_application->m_data_center->write("num_kmar", m_ihm.ui.num_kmar->value());
    m_application->m_data_center->write("cmd_kmar", KMAR_CMD_DISARM_AXIS);
    m_application->m_data_center->write("value_cmd_kmar", AXIS_3);
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", false);
}

void CKMAR::send_arm_axis_3()
{
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", true);
    m_application->m_data_center->write("num_kmar", m_ihm.ui.num_kmar->value());
    m_application->m_data_center->write("cmd_kmar", KMAR_CMD_ARM_AXIS);
    m_application->m_data_center->write("value_cmd_kmar", AXIS_3);
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", false);
}

void CKMAR::send_disarm_axis_4()
{
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", true);
    m_application->m_data_center->write("num_kmar", m_ihm.ui.num_kmar->value());
    m_application->m_data_center->write("cmd_kmar", KMAR_CMD_DISARM_AXIS);
    m_application->m_data_center->write("value_cmd_kmar", AXIS_4);
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", false);
}

void CKMAR::send_arm_axis_4()
{
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", true);
    m_application->m_data_center->write("num_kmar", m_ihm.ui.num_kmar->value());
    m_application->m_data_center->write("cmd_kmar", KMAR_CMD_ARM_AXIS);
    m_application->m_data_center->write("value_cmd_kmar", AXIS_4);
    m_application->m_data_center->write("COMMANDE_KMAR_TxSync", false);
}
