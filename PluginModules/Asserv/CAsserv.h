/*! \file CAsserv.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_Asserv_H_
#define _CPLUGIN_MODULE_Asserv_H_

#include <QMainWindow>
#include <QDebug>
#include <QKeyEvent>

#include "CPluginModule.h"
#include "ui_ihm_Asserv.h"

 class Cihm_Asserv : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_Asserv(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_Asserv() { }

    Ui::ihm_Asserv ui;

    CApplication *m_application;

    // renvoie les appuis touches
    void keyReleaseEvent(QKeyEvent * event) { emit keyPressed(event->key()); }

signals :
    void keyPressed(int key);

 };



 /*! \addtogroup Asserv
   * 
   *  @{
   */

	   
/*! @brief class CAsserv
 */	   
class CAsserv : public CPluginModule
{
    Q_OBJECT
#define     VERSION_Asserv   "1.0"
#define     AUTEUR_Asserv    "Nico"
#define     INFO_Asserv      "Pilotage et analyse de l'asservissement robot"

    // Etat asserv
typedef enum {
 cMOUVEMENT_EN_COURS = 0,
 cCONVERGENCE_OK,
 cBLOCAGE
}tConvergence;

    // Etat asserv rack
/*typedef enum {
 cMOUVEMENT_EN_COURS = 0,
 cCONVERGENCE_OK,
 cBLOCAGE
}tConvergence;*/

typedef enum {
    cMODE_MANUEL = 0,
    cMODE_XY_AUTO,
    cMODE_XY_AUTO_A,
    cMODE_XY_AUTO_B,
    cMODE_DISTANCE_ANGLE,
    cMODE_XY_TETA
} tTypeAsservissement;

//! La liste des codes possibles dans le champ "commande" de la trame ASSERV_DIAG_WRITE_PARAM
// (enum commun CPU<->LaBotBox)
typedef enum {
    cASSERV_SEUIL_CONV_DIST = 0,
    cASSERV_SEUIL_CONV_ANGLE,
    cASSERV_DIAG_WR_KI_ANGLE,
    cASSERV_DIAG_WR_KP_ANGLE,
    cASSERV_DIAG_WR_KI_DISTANCE,
    cASSERV_DIAG_WR_KP_DISTANCE,
    cASSERV_DIAG_WR_CDE_MIN,
    cASSERV_DIAG_WR_CDE_MAX,
    cASSERV_DIAG_WR_K_ANGLE,
    cASSERV_DIAG_WR_COMPTEUR_MAX,
    cASSERV_DIAG_WR_ZONE_MORTE_D,
    cASSERV_DIAG_WR_ZONE_MORTE_G,
    cASSERV_DIAG_RACK_CDE_MAX,
    cASSERV_DIAG_RACK_K_POSVIT,
    cASSERV_DIAG_RACK_KP,
    cASSERV_DIAG_RACK_KI,
    cASSERV_DIAG_RACK_CONV
}eASSERV_WRITE_PARAM;


public:
    CAsserv(const char *plugin_name);
    ~CAsserv();

    Cihm_Asserv *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/wheel.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_Asserv m_ihm;

private slots :
    void onRightClicGUI(QPoint pos);

public slots :
    // Commande manuelle
    void CdeManuelleSynchroSend_left_clic(void);
    void CdeManuelleSynchroSend_right_clic(QPoint);
    void CdeManuelle_Droit_changed(void);
    void CdeManuelle_Gauche_changed(void);
    void CdeManuelle_stopAll(void);

    // Commande XY
    void CdeXYSynchroSend_left_clic(void);
    void CdeXYSynchroSend_right_clic(QPoint);
    void CdeXY_X_changed(void);
    void CdeXY_Y_changed(void);

    // Commande XYTeta
    void CdeXYTSynchroSend_left_clic(void);
    void CdeXYTSynchroSend_right_clic(QPoint);
    void CdeXYT_X_changed(void);
    void CdeXYT_Y_changed(void);
    void CdeXYT_Teta_changed(void);

    // Commande XYTeta
    void CdeInitXYTSynchroSend_left_clic(void);
    void CdeInitXYTSynchroSend_right_clic(QPoint);
    void CdeInitXYT_X_changed(void);
    void CdeInitXYT_Y_changed(void);
    void CdeInitXYT_Teta_changed(void);

    // Commande Distance Angle
    void CdeDistAngleSynchroSend_left_clic(void);
    void CdeDistAngleSynchroSend_right_clic(QPoint);
    void CdeDistAngle_Distance_changed(void);
    void CdeDistAngle_Angle_changed(void);

    // Commande Vitesse Mouvement
    void CdeVitesseMVTSynchroSend_left_clic();
    void CdeVitesseMVTSynchroSend_right_clic(QPoint pt);
    void CdeVitesseMVT_VitAvance_changed();
    void CdeVitesseMVT_IdSportAcc_changed();
    void CdeVitesseMVT_IdSportDec_changed();
    void CdeVitesseMVT_VitRotation_changed();

    // Commande Diag Asserv
    void DiagAsserv_Send_clicked(void);
    void sendRackCdeMax();
    void sendRackKPosVit();
    void sendRackKP();
    void sendRackKI();
    void sendRackSeuilConv();

    //retour asser
    void Convergence_changed(int val);
    void ModeAsservissement_changed(int val);

    //commande chariot
    void setConsigneChariot(void);
    void stopChariot(void);
    void recalChariot(void);


    void ConvRack_changed(QVariant value);
    void Codeur_3_changed(QVariant val);
private slots :
    void keyPressed(int key);

// ____________________________________ DIAG ASSERV
private :
    void initList_ActionsDiagAsserv(void);
    int FacteurPhys2Brute_ActionDiagAsserv(int param);
};

#endif // _CPLUGIN_MODULE_Asserv_H_

/*! @} */

