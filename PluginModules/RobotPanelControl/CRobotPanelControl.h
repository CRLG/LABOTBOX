/*! \file CRobotPanelControl.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_RobotPanelControl_H_
#define _CPLUGIN_MODULE_RobotPanelControl_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_RobotPanelControl.h"

 class Cihm_RobotPanelControl : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_RobotPanelControl(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_RobotPanelControl() { }

    Ui::ihm_RobotPanelControl ui;

    CApplication *m_application;
 };



 /*! \addtogroup RobotPanelControl
   * 
   *  @{
   */

	   
/*! @brief class CRobotPanelControl
 */	   
class CRobotPanelControl : public CPluginModule
{
    Q_OBJECT
#define     VERSION_RobotPanelControl   "1.0"
#define     AUTEUR_RobotPanelControl    "Nico"
#define     INFO_RobotPanelControl      "?"

public:
    CRobotPanelControl(const char *plugin_name);
    ~CRobotPanelControl();

    Cihm_RobotPanelControl *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_RobotPanelControl m_ihm;

    // Spécifique à une coupe donnée
    // Enuméré du message Labotbox ACTION_ROBOT (champ "command")
    typedef enum {
        TOUS_ACTIONNEURS_AU_REPOS = 0,
        ACTIONNEURS_POSITION_INIT,

        ASCENSEUR_MONTE,
        ASCENSEUR_DESCEND,
        ASCENSEUR_STOP,

        PINCE_ARG_FERMEE,
        PINCE_ARG_OUVERTE,
        PINCE_ARG_INTERMEDIAIRE,
        PINCE_ARD_FERMEE,
        PINCE_ARD_OUVERTE,
        PINCE_ARD_INTERMEDIAIRE,

        VERRIN_HAUT,
        VERRIN_BAS,
        VERRIN_INTERMEDIAIRE,

        PINCE_PLANCHE_OUVERT,
        PINCE_PLANCHE_FERMEE,
        PINCE_PLANCHE_INTERMEDIAIRE,

        BANDEROLE_RANGEE,
        BANDEROLE_DEPLOYEE,
        BANDEROLE_INTERMEDIAIRE,

        CAN_MOVER_INT_ON,
        CAN_MOVER_INT_OFF,
        CAN_MOVER_EXT_ON,
        CAN_MOVER_EXT_OFF,

    }eCOMMANDE_ETAT_ROBOT;

private slots :
    void onRightClicGUI(QPoint pos);

    void ascenseur_haut();
    void ascenseur_bas();

    void send_command(unsigned long command, unsigned long value=0);

};

#endif // _CPLUGIN_MODULE_RobotPanelControl_H_

/*! @} */

