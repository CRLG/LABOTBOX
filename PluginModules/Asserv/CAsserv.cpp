/*! \file CAsserv.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CAsserv.h"
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
void CAsserv::init(CApplication *application)
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

  connect(&m_ihm, SIGNAL(keyPressed(int)), this, SLOT(keyPressed(int)));

  initList_ActionsDiagAsserv();
  connect(m_ihm.ui.DiagAsserv_Send, SIGNAL(clicked()), this, SLOT(DiagAsserv_Send_clicked()));


  // COMMANDE_MANUELLE
  m_ihm.ui.CdeManuelle_SendLock->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ihm.ui.CdeManuelle_SendLock, SIGNAL(clicked()), this, SLOT(CdeManuelleSynchroSend_left_clic()));
  connect(m_ihm.ui.CdeManuelle_SendLock, SIGNAL(customContextMenuRequested(QPoint)) , this, SLOT(CdeManuelleSynchroSend_right_clic(QPoint)));
  connect(m_ihm.ui.CdeManuelle_Droit, SIGNAL(editingFinished()), this, SLOT(CdeManuelle_Droit_changed()));
  connect(m_ihm.ui.CdeManuelle_Gauche, SIGNAL(editingFinished()), this, SLOT(CdeManuelle_Gauche_changed()));
  connect(m_ihm.ui.stopAll, SIGNAL(clicked()), this, SLOT(CdeManuelle_stopAll()));

  // COMMANDE_XY
  m_ihm.ui.CdeXY_SendLock->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ihm.ui.CdeXY_SendLock, SIGNAL(clicked()), this, SLOT(CdeXYSynchroSend_left_clic()));
  connect(m_ihm.ui.CdeXY_SendLock, SIGNAL(customContextMenuRequested(QPoint)) , this, SLOT(CdeXYSynchroSend_right_clic(QPoint)));
  connect(m_ihm.ui.CdeXY_X, SIGNAL(editingFinished()), this, SLOT(CdeXY_X_changed()));
  connect(m_ihm.ui.CdeXY_Y, SIGNAL(editingFinished()), this, SLOT(CdeXY_Y_changed()));

  // COMMANDE_XYTeta
  m_ihm.ui.CdeXYTeta_SendLock->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ihm.ui.CdeXYTeta_SendLock, SIGNAL(clicked()), this, SLOT(CdeXYTSynchroSend_left_clic()));
  connect(m_ihm.ui.CdeXYTeta_SendLock, SIGNAL(customContextMenuRequested(QPoint)) , this, SLOT(CdeXYTSynchroSend_right_clic(QPoint)));
  connect(m_ihm.ui.CdeXYTeta_X, SIGNAL(editingFinished()), this, SLOT(CdeXYT_X_changed()));
  connect(m_ihm.ui.CdeXYTeta_Y, SIGNAL(editingFinished()), this, SLOT(CdeXYT_Y_changed()));
  connect(m_ihm.ui.CdeXYTeta_Teta, SIGNAL(editingFinished()), this, SLOT(CdeXYT_Teta_changed()));

  // COMMANDE_INIT_XYTeta
  m_ihm.ui.CdeInitXYTeta_SendLock->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ihm.ui.CdeInitXYTeta_SendLock, SIGNAL(clicked()), this, SLOT(CdeInitXYTSynchroSend_left_clic()));
  connect(m_ihm.ui.CdeInitXYTeta_SendLock, SIGNAL(customContextMenuRequested(QPoint)) , this, SLOT(CdeInitXYTSynchroSend_right_clic(QPoint)));
  connect(m_ihm.ui.CdeInitXYTeta_X, SIGNAL(editingFinished()), this, SLOT(CdeInitXYT_X_changed()));
  connect(m_ihm.ui.CdeInitXYTeta_Y, SIGNAL(editingFinished()), this, SLOT(CdeInitXYT_Y_changed()));
  connect(m_ihm.ui.CdeInitXYTeta_Teta, SIGNAL(editingFinished()), this, SLOT(CdeInitXYT_Teta_changed()));


  // COMMANDE_DISTANCE ANGLE
  m_ihm.ui.CdeDistAngle_SendLock->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ihm.ui.CdeDistAngle_SendLock, SIGNAL(clicked()), this, SLOT(CdeDistAngleSynchroSend_left_clic()));
  connect(m_ihm.ui.CdeDistAngle_SendLock, SIGNAL(customContextMenuRequested(QPoint)) , this, SLOT(CdeDistAngleSynchroSend_right_clic(QPoint)));
  connect(m_ihm.ui.CdeDistAngle_Distance, SIGNAL(editingFinished()), this, SLOT(CdeDistAngle_Distance_changed()));
  connect(m_ihm.ui.CdeDistAngle_Angle, SIGNAL(editingFinished()), this, SLOT(CdeDistAngle_Angle_changed()));

  // COMMANDE_VITESSE MVT
  m_ihm.ui.CdeVitesseMVT_SendLock->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_ihm.ui.CdeVitesseMVT_SendLock, SIGNAL(clicked()), this, SLOT(CdeVitesseMVTSynchroSend_left_clic()));
  connect(m_ihm.ui.CdeVitesseMVT_SendLock, SIGNAL(customContextMenuRequested(QPoint)) , this, SLOT(CdeVitesseMVTSynchroSend_right_clic(QPoint)));
  connect(m_ihm.ui.CdeVitesseMVT_VitAvance, SIGNAL(editingFinished()), this, SLOT(CdeVitesseMVT_VitAvance_changed()));
  connect(m_ihm.ui.CdeVitesseMVT_VitRotation, SIGNAL(editingFinished()), this, SLOT(CdeVitesseMVT_VitRotation_changed()));
  connect(m_ihm.ui.CdeVitesseMVT_IdSportAcc, SIGNAL(editingFinished()), this, SLOT(CdeVitesseMVT_IdSportAcc_changed()));
  connect(m_ihm.ui.CdeVitesseMVT_IdSportDec, SIGNAL(editingFinished()), this, SLOT(CdeVitesseMVT_IdSportDec_changed()));


  // Les données reçues à afficher
  connect(m_application->m_data_center->getData("cde_moteur_D"), SIGNAL(valueChanged(int)), m_ihm.ui.Etat_CdeMotDroit, SLOT(setValue(int)));
  connect(m_application->m_data_center->getData("cde_moteur_G"), SIGNAL(valueChanged(int)), m_ihm.ui.Etat_CdeMotGauche, SLOT(setValue(int)));
  connect(m_application->m_data_center->getData("compteur_diag_blocage"), SIGNAL(valueChanged(int)), m_ihm.ui.Etat_Cpt_diag_blocage, SLOT(setValue(int)));

  connect(m_application->m_data_center->getData("x_pos"), SIGNAL(valueChanged(double)), m_ihm.ui.PosRobot_X, SLOT(setValue(double)));
  connect(m_application->m_data_center->getData("y_pos"), SIGNAL(valueChanged(double)), m_ihm.ui.PosRobot_Y, SLOT(setValue(double)));
  connect(m_application->m_data_center->getData("teta_pos"), SIGNAL(valueChanged(double)), m_ihm.ui.PosRobot_Teta, SLOT(setValue(double)));

  connect(m_application->m_data_center->getData("Convergence"), SIGNAL(valueChanged(int)), this, SLOT(Convergence_changed(int)));
  connect(m_application->m_data_center->getData("ModeAsservissement"), SIGNAL(valueChanged(int)), this, SLOT(ModeAsservissement_changed(int)));

  connect(m_application->m_data_center->getData("rack_convergence"), SIGNAL(valueChanged(QVariant)), this, SLOT(ConvRack_changed(QVariant)));
  connect(m_application->m_data_center->getData("Codeur_3"), SIGNAL(valueChanged(QVariant)), this, SLOT(Codeur_3_changed(QVariant)));


  // COMMANDE_CHARIOT
  connect(m_ihm.ui.chariot_set, SIGNAL(clicked()), this, SLOT(setConsigneChariot()));
  connect(m_ihm.ui.chariot_stop, SIGNAL(clicked()) , this, SLOT(stopChariot()));
  connect(m_ihm.ui.chariot_recal, SIGNAL(clicked()) , this, SLOT(recalChariot()));
  connect(m_ihm.ui.pB_rackGainPosVit,SIGNAL(clicked(bool)),this,SLOT(sendRackKPosVit()));
  connect(m_ihm.ui.pB_rackCommandeMax,SIGNAL(clicked(bool)),this,SLOT(sendRackCdeMax()));
  connect(m_ihm.ui.pB_rackGainProp,SIGNAL(clicked(bool)),this,SLOT(sendRackKP()));
  connect(m_ihm.ui.pB_rackGainInt,SIGNAL(clicked(bool)),this,SLOT(sendRackKI()));
  connect(m_ihm.ui.pB_rackSeuilConv,SIGNAL(clicked(bool)),this,SLOT(sendRackSeuilConv()));


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
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CAsserv::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}


// _____________________________________________________________________
void CAsserv::keyPressed(int key)
{
  switch(key) {
    case Qt::Key_Escape:    CdeManuelle_stopAll();      break;
    // default : ne rien faire
  }
}


// ==================================================
// COMMANDE MANUELLE
// ==================================================
// _____________________________________________________________________
// Clic gauche : envoie la trame
// Clic droit : vérouille la trame
void CAsserv::CdeManuelleSynchroSend_left_clic(void)
{
  // enchaine 1->0 sur le flag de synchro pour forcer l'émission de la trame
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


void CAsserv::CdeManuelle_stopAll(void)
{
 // enchaine 1->0 sur le flag de synchro pour forcer l'émission de la trame
 m_application->m_data_center->write("COMMANDE_MVT_MANUEL_TxSync", true);
 m_ihm.ui.CdeManuelle_Droit->setValue(0);
 m_ihm.ui.CdeManuelle_Gauche->setValue(0);
 m_application->m_data_center->write("PuissanceMotD", 0);
 m_application->m_data_center->write("PuissanceMotG", 0);
 m_application->m_data_center->write("COMMANDE_MVT_MANUEL_TxSync", false); // emet la trame
 m_ihm.ui.CdeManuelle_SendLock->setChecked(false);
}


// ==================================================
// COMMANDE XY
// ==================================================
// _____________________________________________________________________
// Clic gauche : envoie la trame
// Clic droit : vérouille la trame
void CAsserv::CdeXYSynchroSend_left_clic(void)
{
  // enchaine 1->0 sur le floag de synchro pour forcer l'émission de la trame
  m_application->m_data_center->write("COMMANDE_MVT_XY_TxSync", true);
  m_application->m_data_center->write("COMMANDE_MVT_XY_TxSync", false);
  m_ihm.ui.CdeXY_SendLock->setChecked(false);
}

void CAsserv::CdeXYSynchroSend_right_clic(QPoint pt)
{
  Q_UNUSED(pt)
  m_ihm.ui.CdeXY_SendLock->setCheckable(true);
  m_ihm.ui.CdeXY_SendLock->setChecked(!m_ihm.ui.CdeXY_SendLock->isChecked());
  m_application->m_data_center->write("COMMANDE_MVT_XY_TxSync", m_ihm.ui.CdeXY_SendLock->isChecked());
}

void CAsserv::CdeXY_X_changed(void) { m_application->m_data_center->write("X_consigne", m_ihm.ui.CdeXY_X->value()); }
void CAsserv::CdeXY_Y_changed(void) { m_application->m_data_center->write("Y_consigne", m_ihm.ui.CdeXY_Y->value()); }

// ==================================================
// COMMANDE XYT
// ==================================================
// _____________________________________________________________________
// Clic gauche : envoie la trame
// Clic droit : vérouille la trame
void CAsserv::CdeXYTSynchroSend_left_clic(void)
{
  // enchaine 1->0 sur le floag de synchro pour forcer l'émission de la trame
  m_application->m_data_center->write("COMMANDE_MVT_XY_TETA_TxSync", true);
  m_application->m_data_center->write("COMMANDE_MVT_XY_TETA_TxSync", false);
  m_ihm.ui.CdeXYTeta_SendLock->setChecked(false);
}

void CAsserv::CdeXYTSynchroSend_right_clic(QPoint pt)
{
  Q_UNUSED(pt)
  m_ihm.ui.CdeXYTeta_SendLock->setCheckable(true);
  m_ihm.ui.CdeXYTeta_SendLock->setChecked(!m_ihm.ui.CdeXYTeta_SendLock->isChecked());
  m_application->m_data_center->write("COMMANDE_MVT_XY_TETA_TxSync", m_ihm.ui.CdeXYTeta_SendLock->isChecked());
}

void CAsserv::CdeXYT_X_changed(void) { m_application->m_data_center->write("XYT_X_consigne", m_ihm.ui.CdeXYTeta_X->value()); }
void CAsserv::CdeXYT_Y_changed(void) { m_application->m_data_center->write("XYT_Y_consigne", m_ihm.ui.CdeXYTeta_Y->value()); }
void CAsserv::CdeXYT_Teta_changed(void) { m_application->m_data_center->write("XYT_angle_consigne", m_ihm.ui.CdeXYTeta_Teta->value()); }


// ==================================================
// COMMANDE REINIT XYT
// ==================================================
// _____________________________________________________________________
// Clic gauche : envoie la trame
// Clic droit : vérouille la trame
void CAsserv::CdeInitXYTSynchroSend_left_clic(void)
{
  // enchaine 1->0 sur le floag de synchro pour forcer l'émission de la trame
  m_application->m_data_center->write("COMMANDE_REINIT_XY_TETA_TxSync", true);
  m_application->m_data_center->write("COMMANDE_REINIT_XY_TETA_TxSync", false);
  m_ihm.ui.CdeInitXYTeta_SendLock->setChecked(false);
}

void CAsserv::CdeInitXYTSynchroSend_right_clic(QPoint pt)
{
  Q_UNUSED(pt)
  m_ihm.ui.CdeInitXYTeta_SendLock->setCheckable(true);
  m_ihm.ui.CdeInitXYTeta_SendLock->setChecked(!m_ihm.ui.CdeInitXYTeta_SendLock->isChecked());
  m_application->m_data_center->write("COMMANDE_REINIT_XY_TETA_TxSync", m_ihm.ui.CdeInitXYTeta_SendLock->isChecked());
}

void CAsserv::CdeInitXYT_X_changed(void) { m_application->m_data_center->write("reinit_x_pos", m_ihm.ui.CdeInitXYTeta_X->value()); }
void CAsserv::CdeInitXYT_Y_changed(void) { m_application->m_data_center->write("reinit_y_pos", m_ihm.ui.CdeInitXYTeta_Y->value()); }
void CAsserv::CdeInitXYT_Teta_changed(void) { m_application->m_data_center->write("reinit_teta_pos", m_ihm.ui.CdeInitXYTeta_Teta->value()); }



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


// ==================================================
// COMMANDE VITESSE MVT
// ==================================================
// _____________________________________________________________________
// Clic gauche : envoie la trame
// Clic droit : vérouille la trame
void CAsserv::CdeVitesseMVTSynchroSend_left_clic(void)
{
  // enchaine 1->0 sur le floag de synchro pour forcer l'émission de la trame
  m_application->m_data_center->write("COMMANDE_VITESSE_MVT_TxSync", true);
  m_application->m_data_center->write("COMMANDE_VITESSE_MVT_TxSync", false);
  m_ihm.ui.CdeVitesseMVT_SendLock->setChecked(false);
}

void CAsserv::CdeVitesseMVTSynchroSend_right_clic(QPoint pt)
{
  Q_UNUSED(pt)
  m_ihm.ui.CdeVitesseMVT_SendLock->setCheckable(true);
  m_ihm.ui.CdeVitesseMVT_SendLock->setChecked(!m_ihm.ui.CdeVitesseMVT_SendLock->isChecked());
  m_application->m_data_center->write("COMMANDE_VITESSE_MVT_TxSync", m_ihm.ui.CdeVitesseMVT_SendLock->isChecked());
}

void CAsserv::CdeVitesseMVT_VitAvance_changed(void)
{
    m_application->m_data_center->write("vitesse_avance_max", m_ihm.ui.CdeVitesseMVT_VitAvance->value());
    m_ihm.ui.CdeVitesseMVT_VitAvance->setValue(80);
}
void CAsserv::CdeVitesseMVT_VitRotation_changed(void)
{
    m_application->m_data_center->write("vitesse_rotation_max", m_ihm.ui.CdeVitesseMVT_VitRotation->value());
    m_ihm.ui.CdeVitesseMVT_VitRotation->setValue(3.0);
}
void CAsserv::CdeVitesseMVT_IdSportAcc_changed(void)   { m_application->m_data_center->write("indice_sportivite_accel", m_ihm.ui.CdeVitesseMVT_IdSportAcc->value()); }
void CAsserv::CdeVitesseMVT_IdSportDec_changed(void)   { m_application->m_data_center->write("indice_sportivite_decel", m_ihm.ui.CdeVitesseMVT_IdSportDec->value()); }



// ==================================================
// ETAT ASSERVISSEMENT
// ==================================================

void CAsserv::Convergence_changed(int val)
{
  switch(val) {
    case cMOUVEMENT_EN_COURS :
       m_ihm.ui.EtatConvergence->setText("Mouvement en cours");
    break;

    case cCONVERGENCE_OK :
      m_ihm.ui.EtatConvergence->setText("Convergence OK");
    break;

    case cBLOCAGE :
      m_ihm.ui.EtatConvergence->setText("Blocage");
    break;

    default :
      m_ihm.ui.EtatConvergence->setText("?");
    break;
  }
}
// _____________________________________________________________________
void CAsserv::ModeAsservissement_changed(int val)
{
  switch(val) {
    case cMODE_MANUEL :
       m_ihm.ui.EtatModeAsservissement->setText("Mode manuel");
    break;

    case cMODE_XY_AUTO :
      m_ihm.ui.EtatModeAsservissement->setText("Mode XY auto");
    break;

    case cMODE_XY_AUTO_A :
      m_ihm.ui.EtatModeAsservissement->setText("Mode XY auto A");
    break;

    case cMODE_XY_AUTO_B :
      m_ihm.ui.EtatModeAsservissement->setText("Mode XY auto B");
    break;

    case cMODE_DISTANCE_ANGLE :
      m_ihm.ui.EtatModeAsservissement->setText("Mode Distance angle");
    break;

    case cMODE_XY_TETA :
      m_ihm.ui.EtatModeAsservissement->setText("Mode XYTeta");
    break;

    default :
      m_ihm.ui.EtatModeAsservissement->setText("?");
    break;
  }
}

// ==================================================
// DIAG ASSERVISSEMENT
// ==================================================
// ____________________________________________________
void CAsserv::initList_ActionsDiagAsserv(void)
{
  QStringList lst;
  // à mettre dans le même ordre que l'énuméré "eASSERV_WRITE_PARAM" (commun avec le CPU)
  lst << "SEUIL_CONV_DIST"
      << "SEUIL_CONV_ANGLE"
      << "DIAG_WR_KI_ANGLE"
      << "DIAG_WR_KP_ANGLE"
      << "DIAG_WR_KI_DISTANCE"
      << "DIAG_WR_KP_DISTANCE"
      << "DIAG_WR_CDE_MIN"
      << "DIAG_WR_CDE_MAX"
      << "DIAG_WR_K_ANGLE"
      << "DIAG_WR_COMPTEUR_MAX"
      << "DIAG_WR_ZONE_MORTE_D"
      << "DIAG_WR_ZONE_MORTE_G";

  m_ihm.ui.DiagAsserv_Action->addItems(lst);
}

// ____________________________________________________
void CAsserv::DiagAsserv_Send_clicked(void)
{
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM_TxSync", true);
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM", m_ihm.ui.DiagAsserv_Action->currentIndex());
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_VALUE", (int)(FacteurPhys2Brute_ActionDiagAsserv(m_ihm.ui.DiagAsserv_Action->currentIndex()) * m_ihm.ui.DiagAsserv_Value->value()));
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM_TxSync", false);
}

// ____________________________________________________
// Les coeficients de l'asserv ne sont pas tous dans le même range
// Adapte le range au coeficient
// Attention : doit être en cohérence avec le code côté CPU
int CAsserv::FacteurPhys2Brute_ActionDiagAsserv(int param)
{
    switch (param) {
        case cASSERV_SEUIL_CONV_DIST:
        case cASSERV_SEUIL_CONV_ANGLE:
        case cASSERV_DIAG_WR_KI_ANGLE:
        case cASSERV_DIAG_WR_KP_ANGLE:
        case cASSERV_DIAG_WR_KI_DISTANCE:
        case cASSERV_DIAG_WR_KP_DISTANCE:
        case cASSERV_DIAG_RACK_K_POSVIT:
        case cASSERV_DIAG_RACK_KP:
        case cASSERV_DIAG_RACK_KI:
        case cASSERV_DIAG_WR_K_ANGLE :
        //case cASSERV_DIAG_RACK_CONV:
            return (100);
        break;

        default :
            return (1);
        break;
    }
}

void CAsserv::setConsigneChariot(void)
{
    float consigne=m_ihm.ui.chariot_consigne->value();
    int sens_position=0;
    if(consigne>=0)
        sens_position=5;
    else
        sens_position=15;
    float consigne_messagerie=qRound(qAbs(consigne)/10.0);
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", true);
    m_application->m_data_center->write("NumeroServoMoteur1", 50);
    m_application->m_data_center->write("PositionServoMoteur1", consigne_messagerie);
    m_application->m_data_center->write("VitesseServoMoteur1", sens_position);
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", false);
    //qDebug() << "set rack ASSER at "<<consigne_messagerie<<"and sens"<<sens_position<<"on trame 50";
}

void CAsserv::stopChariot(void)
{
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", true);
    m_application->m_data_center->write("NumeroServoMoteur1", 51);
    m_application->m_data_center->write("PositionServoMoteur1", 1);
    m_application->m_data_center->write("VitesseServoMoteur1", 0);
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", false);
    //qDebug() << "set rack STOP with (1,0) on trame 51";
}
void CAsserv::recalChariot(void)
{
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", true);
    m_application->m_data_center->write("NumeroServoMoteur1", 52);
    m_application->m_data_center->write("PositionServoMoteur1", 1);
    m_application->m_data_center->write("VitesseServoMoteur1", 0);
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", false);
    //qDebug() << "set rack RECAL with (1,0) on trame 52";
}
/*
 * cASSERV_DIAG_RACK_CDE_MAX,
   cASSERV_DIAG_RACK_K_POSVIT,
   cASSERV_DIAG_RACK_KP,
   cASSERV_DIAG_RACK_KI,
   cASSERV_DIAG_RACK_CONV*/

void CAsserv::sendRackCdeMax(void)
{
    int param=(int)(FacteurPhys2Brute_ActionDiagAsserv(cASSERV_DIAG_RACK_CDE_MAX) * m_ihm.ui.rackCommandeMax->value());
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM_TxSync", true);
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM",cASSERV_DIAG_RACK_CDE_MAX );
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_VALUE",param );
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM_TxSync", false);
    //qDebug() << "set rack CDE_MAX at "<<param;
}

void CAsserv::sendRackKPosVit(void)
{
    int param=(int)(FacteurPhys2Brute_ActionDiagAsserv(cASSERV_DIAG_RACK_K_POSVIT) * m_ihm.ui.rackGainPosVit->value());
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM_TxSync", true);
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM",cASSERV_DIAG_RACK_K_POSVIT );
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_VALUE", param );
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM_TxSync", false);
    //qDebug() << "set rack K_POSVIT at "<<param;
}
void CAsserv::sendRackKP(void)
{
    int param=(int)(FacteurPhys2Brute_ActionDiagAsserv(cASSERV_DIAG_RACK_KP) * m_ihm.ui.rackGainProp->value());
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM_TxSync", true);
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM",cASSERV_DIAG_RACK_KP );
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_VALUE", param );
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM_TxSync", false);
    //qDebug() << "set rack KP at "<<param;
}
void CAsserv::sendRackKI(void)
{
    int param=(int)(FacteurPhys2Brute_ActionDiagAsserv(cASSERV_DIAG_RACK_KI) * m_ihm.ui.rackGainInt->value());
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM_TxSync", true);
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM",cASSERV_DIAG_RACK_KI );
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_VALUE", param );
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM_TxSync", false);
    //qDebug() << "set rack KI at "<<param;
}
void CAsserv::sendRackSeuilConv(void)
{
    int param=(int)(FacteurPhys2Brute_ActionDiagAsserv(cASSERV_DIAG_RACK_CONV) * m_ihm.ui.rackSeuilConv->value());
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM_TxSync", true);
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM",cASSERV_DIAG_RACK_CONV );
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_VALUE", param);
    m_application->m_data_center->write("ASSERV_DIAG_WRITE_PARAM_TxSync", false);
    //qDebug() << "set rack SEUIL_CONV at "<<param;
}

void CAsserv::ConvRack_changed(QVariant value) {m_ihm.ui.ConvRack->setValue(value.toBool());}

void CAsserv::Codeur_3_changed(QVariant val){m_ihm.ui.PosRack->setValue(val.toInt());}
