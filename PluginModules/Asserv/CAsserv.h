/*! \file CAsserv.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_Asserv_H_
#define _CPLUGIN_MODULE_Asserv_H_

#include <QMainWindow>

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

public:
    CAsserv(const char *plugin_name);
    ~CAsserv();

    Cihm_Asserv *getIHM(void) { return(&m_ihm); }

    virtual void init(CLaBotBox *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/wheel.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_Asserv m_ihm;

public slots :
    // Commande manuelle
    void CdeManuelleSynchroSend_left_clic(void);
    void CdeManuelleSynchroSend_right_clic(QPoint);
    void CdeManuelle_Droit_changed(void);
    void CdeManuelle_Gauche_changed(void);

    // Commande XY
    void CdeXYSynchroSend_left_clic(void);
    void CdeXYSynchroSend_right_clic(QPoint);
    void CdeXY_X_changed(void);
    void CdeXY_Y_changed(void);

    // Commande XYTeta
    // TODO : à revoir car les noms dans la messagerie sont identiques à Commande XY
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


};

#endif // _CPLUGIN_MODULE_Asserv_H_

/*! @} */

