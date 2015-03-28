/*! \file CAsserv.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CAsserv.h"
#include "CLaBotBox.h"
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
CAsserv::CAsserv(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_Asserv, AUTEUR_Asserv, INFO_Asserv)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CAsserv::~CAsserv()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CAsserv::init(CLaBotBox *application)
{
  CModule::init(application);
  setGUI(&m_ihm); // indique à la classe de base l'IHM

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


  // COMMANDE_MANUELLE
  m_ihm.ui.CdeManuelle_SendLock->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ihm.ui.CdeManuelle_SendLock, SIGNAL(clicked()), this, SLOT(CdeManuelleSynchroSend_left_clic()));
  connect(m_ihm.ui.CdeManuelle_SendLock, SIGNAL(customContextMenuRequested(QPoint)) , this, SLOT(CdeManuelleSynchroSend_right_clic(QPoint)));
  connect(m_ihm.ui.CdeManuelle_Droit, SIGNAL(editingFinished()), this, SLOT(CdeManuelle_Droit_changed()));
  connect(m_ihm.ui.CdeManuelle_Gauche, SIGNAL(editingFinished()), this, SLOT(CdeManuelle_Gauche_changed()));

  // COMMANDE_XY
  m_ihm.ui.CdeXY_SendLock->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ihm.ui.CdeXY_SendLock, SIGNAL(clicked()), this, SLOT(CdeXYSynchroSend_left_clic()));
  connect(m_ihm.ui.CdeXY_SendLock, SIGNAL(customContextMenuRequested(QPoint)) , this, SLOT(CdeXYSynchroSend_right_clic(QPoint)));
  connect(m_ihm.ui.CdeXY_X, SIGNAL(editingFinished()), this, SLOT(CdeXY_X_changed()));
  connect(m_ihm.ui.CdeXY_Y, SIGNAL(editingFinished()), this, SLOT(CdeXY_Y_changed()));

  // COMMANDE_XYTeta
/*
Messagerie à corriger car les noms de variables sont les mêmes que pour le trame COMMANDE_MVT_XY
  m_ihm.ui.CdeXYTeta_SendLock->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ihm.ui.CdeXYTeta_SendLock, SIGNAL(clicked()), this, SLOT(CdeXYTetaSynchroSend_left_clic()));
  connect(m_ihm.ui.CdeXYTeta_SendLock, SIGNAL(customContextMenuRequested(QPoint)) , this, SLOT(CdeXYTetaSynchroSend_right_clic(QPoint)));
  connect(m_ihm.ui.CdeXYTeta_X, SIGNAL(editingFinished()), this, SLOT(CdeXYTeta_X_changed()));
  connect(m_ihm.ui.CdeXYTeta_Y, SIGNAL(editingFinished()), this, SLOT(CdeXYTeta_Y_changed()));
*/

  // COMMANDE_DISTANCE ANGLE
  m_ihm.ui.CdeDistAngle_SendLock->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ihm.ui.CdeDistAngle_SendLock, SIGNAL(clicked()), this, SLOT(CdeDistAngleSynchroSend_left_clic()));
  connect(m_ihm.ui.CdeDistAngle_SendLock, SIGNAL(customContextMenuRequested(QPoint)) , this, SLOT(CdeDistAngleSynchroSend_right_clic(QPoint)));
  connect(m_ihm.ui.CdeDistAngle_Distance, SIGNAL(editingFinished()), this, SLOT(CdeDistAngle_Distance_changed()));
  connect(m_ihm.ui.CdeDistAngle_Angle, SIGNAL(editingFinished()), this, SLOT(CdeDistAngle_Angle_changed()));


}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CAsserv::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
}


// ==================================================
// COMMANDE MANUELLE
// ==================================================
// _____________________________________________________________________
// Clic gauche : envoie la trame
// Clic droit : vérouille la trame
void CAsserv::CdeManuelleSynchroSend_left_clic(void)
{
  // enchaine 1->0 sur le floag de synchro pour forcer l'émission de la trame
  m_application->m_data_center->write("COMMANDE_MVT_MANUEL_TxSync", true);
  m_application->m_data_center->write("COMMANDE_MVT_MANUEL_TxSync", false);
  m_ihm.ui.CdeManuelle_SendLock->setChecked(false);
}

void CAsserv::CdeManuelleSynchroSend_right_clic(QPoint pt)
{
  Q_UNUSED(pt)
  m_ihm.ui.CdeManuelle_SendLock->setCheckable(true);
  m_ihm.ui.CdeManuelle_SendLock->setChecked(!m_ihm.ui.CdeManuelle_SendLock->isChecked());
  m_application->m_data_center->write("COMMANDE_MVT_MANUEL_TxSync", m_ihm.ui.CdeManuelle_SendLock->isChecked());
}

void CAsserv::CdeManuelle_Droit_changed(void)   { m_application->m_data_center->write("PuissanceMotD", m_ihm.ui.CdeManuelle_Droit->value()); }
void CAsserv::CdeManuelle_Gauche_changed(void)  { m_application->m_data_center->write("PuissanceMotG", m_ihm.ui.CdeManuelle_Gauche->value()); }



// ==================================================
// COMMANDE XY
// ==================================================
// _____________________________________________________________________
// Clic gauche : envoie la trame
// Clic droit : vérouille la trame
void CAsserv::CdeXYSynchroSend_left_clic(void)
{
  // enchaine 1->0 sur le floag de synchro pour forcer l'émission de la trame
  m_application->m_data_center->write("COMMANDE_XY_TxSync", true);
  m_application->m_data_center->write("COMMANDE_XY_TxSync", false);
  m_ihm.ui.CdeXY_SendLock->setChecked(false);
}

void CAsserv::CdeXYSynchroSend_right_clic(QPoint pt)
{
  Q_UNUSED(pt)
  m_ihm.ui.CdeXY_SendLock->setCheckable(true);
  m_ihm.ui.CdeXY_SendLock->setChecked(!m_ihm.ui.CdeXY_SendLock->isChecked());
  m_application->m_data_center->write("COMMANDE_XY_TxSync", m_ihm.ui.CdeXY_SendLock->isChecked());
}

void CAsserv::CdeXY_X_changed(void) { m_application->m_data_center->write("X_consigne", m_ihm.ui.CdeXY_X->value()); }
void CAsserv::CdeXY_Y_changed(void) { m_application->m_data_center->write("Y_consigne", m_ihm.ui.CdeXY_Y->value()); }


// ==================================================
// COMMANDE DISTANCE ANGLE
// ==================================================
// _____________________________________________________________________
// Clic gauche : envoie la trame
// Clic droit : vérouille la trame
void CAsserv::CdeDistAngleSynchroSend_left_clic(void)
{
  // enchaine 1->0 sur le floag de synchro pour forcer l'émission de la trame
  m_application->m_data_center->write("COMMANDE_DISTANCE_ANGLE_TxSync", true);
  m_application->m_data_center->write("COMMANDE_DISTANCE_ANGLE_TxSync", false);
  m_ihm.ui.CdeDistAngle_SendLock->setChecked(false);
}

void CAsserv::CdeDistAngleSynchroSend_right_clic(QPoint pt)
{
  Q_UNUSED(pt)
  m_ihm.ui.CdeDistAngle_SendLock->setCheckable(true);
  m_ihm.ui.CdeDistAngle_SendLock->setChecked(!m_ihm.ui.CdeDistAngle_SendLock->isChecked());
  m_application->m_data_center->write("COMMANDE_DISTANCE_ANGLE_TxSync", m_ihm.ui.CdeDistAngle_SendLock->isChecked());
}

void CAsserv::CdeDistAngle_Distance_changed(void)   { m_application->m_data_center->write("distance_consigne", m_ihm.ui.CdeDistAngle_Distance->value()); }
void CAsserv::CdeDistAngle_Angle_changed(void)      { m_application->m_data_center->write("angle_consigne", m_ihm.ui.CdeDistAngle_Angle->value()); }


