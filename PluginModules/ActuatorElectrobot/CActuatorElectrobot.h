/*! \file CActuatorElectrobot.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_ActuatorElectrobot_H_
#define _CPLUGIN_MODULE_ActuatorElectrobot_H_

#include <QMainWindow>
#include <QKeyEvent>

#include "CPluginModule.h"
#include "ui_ihm_ActuatorElectrobot.h"

 class Cihm_ActuatorElectrobot : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_ActuatorElectrobot(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_ActuatorElectrobot() { }

    Ui::ihm_ActuatorElectrobot ui;

    CApplication *m_application;

    // renvoie les appuis touches
    void keyReleaseEvent(QKeyEvent * event) { emit keyPressed(event->key()); }

signals :
    void keyPressed(int key);

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

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/pince.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private slots :
    void onRightClicGUI(QPoint pos);

private:
    Cihm_ActuatorElectrobot m_ihm;

// ____________________________________________________Moteurs
private :
    void updateMotorsAliasLabels(void);
    void updateMotorsTooltips(void);
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

// ____________________________________________________Servo-moteurs classiques et SD20
private :
    //! La liste des codes possibles dans le champ "commande_sd20" de la trame ELECTROBOT_CONFIG_SERVOS
    // (enum commun CPU<->LaBotBox)
    typedef enum {
      cCONFIG_SERVO_POSITION = 0,
      cCONFIG_SERVO_BUTEE_MIN,
      cCONFIG_SERVO_BUTEE_MAX,
      cCONFIG_SERVO_POSITION_INIT
    }eCONFIG_SERVOS;

    typedef enum {
        cSERVOS_CLASSIQUES = 0,
        cSERVOS_SD20
    }eTYPE_SERVO;
    int m_choix_type_servos;

    void PosServoMoteur_changed(int id, int position, int speed=0);
    void initList_ActionsServos(void);

private slots :
    // Servos moteurs
    void CdeServoMoteur1_changed();
    void CdeServoMoteur2_changed();
    void CdeServoMoteur3_changed();
    void CdeServoMoteur4_changed();
    void CdeServoMoteur5_changed();
    void CdeServoMoteur6_changed();
    void CdeServoMoteur7_changed();
    void CdeServoMoteur8_changed();

    void choix_type_servo_classique_sd20();
    unsigned int servo_id_to_servo_num(unsigned int servo_id);

    void ServosConfig_Send_clicked(void);

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

    // ____________________________________________________ Power Switch
private :
    void updatePowerSwitchAliasLabels(void);
    void updatePowerSwitchTooltips(void);

private slots :
    void CdePowerSwitchSynchroSend_left_clic(void);
    void CdePowerSwitchSynchroSend_right_clic(QPoint pt);

    void CdePowerSwitch_xt1_changed(bool val);
    void CdePowerSwitch_xt2_changed(bool val);
    void CdePowerSwitch_xt3_changed(bool val);
    void CdePowerSwitch_xt4_changed(bool val);
    void CdePowerSwitch_xt5_changed(bool val);
    void CdePowerSwitch_xt6_changed(bool val);
    void CdePowerSwitch_xt7_changed(bool val);
    void CdePowerSwitch_xt8_changed(bool val);

    void PowerSwitch_StopAll_clicked(void);

private slots :
    void keyPressed(int key);
};

#endif // _CPLUGIN_MODULE_ActuatorElectrobot_H_

/*! @} */

