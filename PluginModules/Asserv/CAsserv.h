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

    CLaBotBox *m_application;

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

typedef enum {
    cMODE_MANUEL = 0,
    cMODE_XY_AUTO,
    cMODE_XY_AUTO_A,
    cMODE_XY_AUTO_B,
    cMODE_DISTANCE_ANGLE,
    cMODE_XY_TETA
} tTypeAsservissement;

//! La liste des codes possibles dans le champ "commande_ax" de la trame ELECTROBOT_CDE_SERVOS_AX
// (enum commun MBED<->LaBotBox)
typedef enum {
   cASSERV_SEUIL_CONV_DIST = 0,
   cASSERV_SEUIL_CONV_ANGLE,
   cASSERV_DIAG_WR_KI_ANGLE,
   cASSERV_DIAG_WR_KP_ANGLE,
   cASSERV_DIAG_WR_KI_DISTANCE,
   cASSERV_DIAG_WR_KP_DISTANCE,
   cASSERV_DIAG_WR_CDE_MIN,
   cASSERV_DIAG_WR_CDE_MAX
}eASSERV_WRITE_PARAM;


public:
    CAsserv(const char *plugin_name);
    ~CAsserv();

    Cihm_Asserv *getIHM(void) { return(&m_ihm); }

    virtual void init(CLaBotBox *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/wheel.png")); }  // Pr�cise l'ic�ne qui repr�sente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Pr�cise le nom du menu de la fen�tre principale dans lequel le module appara�t

private:
    Cihm_Asserv m_ihm;

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
    // TODO : � revoir car les noms dans la messagerie sont identiques � Commande XY
/*
    void CdeXYTeta(void);
    void CdeXYTetaSynchroSend_right_clic(QPoint);
    void CdeXYTeta_X_changed(void);
    void CdeXYTeta_Y_changed(void);
    void CdeXYTeta_Teta_changed(void);
*/
    // Commande Distance Angle
    void CdeDistAngleSynchroSend_left_clic(void);
    void CdeDistAngleSynchroSend_right_clic(QPoint);
    void CdeDistAngle_Distance_changed(void);
    void CdeDistAngle_Angle_changed(void);

    void Convergence_changed(int val);
    void ModeAsservissement_changed(int val);

private slots :
    void keyPressed(int key);

// ____________________________________ DIAG ASSERV
private :
    void initList_ActionsDiagAsserv(void);
    int FacteurPhys2Brute_ActionDiagAsserv(int param);
private slots :
    void DiagAsserv_Send_clicked(void);

};

#endif // _CPLUGIN_MODULE_Asserv_H_

/*! @} */

