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

        PINCE_NOISETTE_REPOS,
        PINCE_NOISETTE_OUVERTE,
        PINCE_NOISETTE_FERMEE,

        THERESE_1_DEPLOYEE,
        THERESE_1_INTERMEDIAIRE,
        THERESE_1_RANGEE,
        THERESE_2_DEPLOYEE,
        THERESE_2_INTERMEDIAIRE,
        THERESE_2_RANGEE,
        THERESE_3_DEPLOYEE,
        THERESE_3_INTERMEDIAIRE,
        THERESE_3_RANGEE,
        THERESE_4_DEPLOYEE,
        THERESE_4_INTERMEDIAIRE,
        THERESE_4_RANGEE,

        THERMOVE_ACTIF,
        THERMOVE_REPOS,

    }eCOMMANDE_ETAT_ROBOT;

private slots :
    void onRightClicGUI(QPoint pos);

    void send_command(unsigned long command, unsigned long value=0);

};

#endif // _CPLUGIN_MODULE_RobotPanelControl_H_

/*! @} */

