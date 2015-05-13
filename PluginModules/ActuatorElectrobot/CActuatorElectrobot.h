/*! \file CActuatorElectrobot.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_ActuatorElectrobot_H_
#define _CPLUGIN_MODULE_ActuatorElectrobot_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_ActuatorElectrobot.h"

 class Cihm_ActuatorElectrobot : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_ActuatorElectrobot(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_ActuatorElectrobot() { }

    Ui::ihm_ActuatorElectrobot ui;

    CLaBotBox *m_application;
 };



 /*! \addtogroup ActuatorElectrobot
   * 
   *  @{
   */

	   
/*! @brief class CActuatorElectrobot
 */	   
class CActuatorElectrobot : public CPluginModule
{
    Q_OBJECT
#define     VERSION_ActuatorElectrobot   "1.0"
#define     AUTEUR_ActuatorElectrobot    "Nico"
#define     INFO_ActuatorElectrobot      "Pilotage des actionneurs Electrobot"

public:
    CActuatorElectrobot(const char *plugin_name);
    ~CActuatorElectrobot();

    Cihm_ActuatorElectrobot *getIHM(void) { return(&m_ihm); }

    virtual void init(CLaBotBox *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/pince.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_ActuatorElectrobot m_ihm;

    void updateAliasLabels(void);
    void updateMotorsTooltips(void);

// ____________________________________________________Moteurs
private slots :
    void CdeMoteursSynchroSend_left_clic(void);
    void CdeMoteursSynchroSend_right_clic(QPoint pt);

    void CdeMoteur1_changed(void);
    void CdeMoteur2_changed(void);
    void CdeMoteur3_changed(void);
    void CdeMoteur4_changed(void);
    void CdeMoteur5_changed(void);
    void CdeMoteur6_changed(void);

    void Moteurs_StopAll_clicked(void);

// ____________________________________________________Servo-moteurs SD20
private :
    //! La liste des codes possibles dans le champ "commande_sd20" de la trame ELECTROBOT_CDE_SERVOS_SD20
    // (enum commun MBED<->LaBotBox)
    typedef enum {
      cSERVO_SD20_POSITION = 0,
      cSERVO_SD20_BUTEE_MIN,
      cSERVO_SD20_BUTEE_MAX,
      cSERVO_SD20_POSITION_INIT
    }eCOMMANDES_SERVOS_SD20;

    void PosServoMoteur_changed(int id, int position);
    void initList_ActionsServosSD20(void);

private slots :
    //void CdeServoMoteur20_changed(int val);
    void CdeServoMoteur20_changed();
    void CdeServoMoteur19_changed();
    void CdeServoMoteur18_changed();
    void CdeServoMoteur17_changed();
    void CdeServoMoteur16_changed();
    void CdeServoMoteur15_changed();
    void CdeServoMoteur14_changed();
    void CdeServoMoteur13_changed();

    void ServosSD20Config_Send_clicked(void);

// ____________________________________________________Servo-moteurs AX
private :
    //! La liste des codes possibles dans le champ "commande_ax" de la trame ELECTROBOT_CDE_SERVOS_AX
    // (enum commun MBED<->LaBotBox)
    typedef enum {
      cSERVO_AX_POSITION = 0,
      cSERVO_AX_VITESSE,
      cSERVO_AX_COUPLE,
      cSERVO_AX_CHANGE_ID,
      cSERVO_AX_LED_STATE,
      cSERVO_AX_BUTEE_MIN,
      cSERVO_AX_BUTEE_MAX,
      cSERVO_AX_POSITION_INIT
    }eCOMMANDES_SERVOS_AX;

    void PosServoMoteurAX_changed(int id, int position);
    void initList_ActionsServosAX(void);

private slots :
    void CdeServoMoteurAX0_changed();
    void CdeServoMoteurAX1_changed();
    void CdeServoMoteurAX2_changed();
    void CdeServoMoteurAX3_changed();
    void CdeServoMoteurAX4_changed();
    void CdeServoMoteurAX5_changed();
    void CdeServoMoteurAX6_changed();
    void CdeServoMoteurAX7_changed();

    void ServosAXConfig_Send_clicked(void);
};

#endif // _CPLUGIN_MODULE_ActuatorElectrobot_H_

/*! @} */

