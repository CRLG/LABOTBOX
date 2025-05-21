/*! \file CPowerElectrobot.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_PowerElectrobot_H_
#define _CPLUGIN_MODULE_PowerElectrobot_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_PowerElectrobot.h"

 class Cihm_PowerElectrobot : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_PowerElectrobot(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_PowerElectrobot() { }

    Ui::ihm_PowerElectrobot ui;

    CApplication *m_application;
 };



 /*! \addtogroup PowerElectrobot
   * 
   *  @{
   */

	   
/*! @brief class CPowerElectrobot
 */	   
class CPowerElectrobot : public CPluginModule
{
    Q_OBJECT
#define     VERSION_PowerElectrobot   "1.0"
#define     AUTEUR_PowerElectrobot    "Nico"
#define     INFO_PowerElectrobot      "Module de contrôle de la carte PowerElectrobot"

    //! La liste des codes possibles dans le champ "commande" de la trame COMMANDE_POWER_ELECTROBOT
    // (enum commun CPU<->LaBotBox)
    typedef enum {
        cCDE_PWR_ELECTROBOT_ALL_OUTPUTS = 0,
        cCDE_PWR_ELECTROBOT_OUTPUT_1,
        cCDE_PWR_ELECTROBOT_OUTPUT_2,
        cCDE_PWR_ELECTROBOT_OUTPUT_3,
        cCDE_PWR_ELECTROBOT_OUTPUT_4,
        cCDE_PWR_ELECTROBOT_OUTPUT_5,
        cCDE_PWR_ELECTROBOT_OUTPUT_6,
        cCDE_PWR_ELECTROBOT_OUTPUT_7,
        cCDE_PWR_ELECTROBOT_OUTPUT_8,
        cCDE_PWR_ELECTROBOT_CHANGE_I2C_ADDR,
        cCDE_PWR_ELECTROBOT_UNLOCK_EEPROM,
        cCDE_PWR_ELECTROBOT_RESET_FACTORY,
        cCDE_PWR_ELECTROBOT_CALIB_BATTERY_VOLTAGE_POINT1,
        cCDE_PWR_ELECTROBOT_CALIB_BATTERY_VOLTAGE_POINT2,
        cCDE_PWR_ELECTROBOT_CALIB_GLOBAL_CURRENT_POINT1,
        cCDE_PWR_ELECTROBOT_CALIB_GLOBAL_CURRENT_POINT2,
        cCDE_PWR_ELECTROBOT_CALIB_CURRENT_OUT1_POINT1,
        cCDE_PWR_ELECTROBOT_CALIB_CURRENT_OUT1_POINT2,
        cCDE_PWR_ELECTROBOT_CALIB_CURRENT_OUT2_POINT1,
        cCDE_PWR_ELECTROBOT_CALIB_CURRENT_OUT2_POINT2,
    }eCOMMANDES_POWER_ELECTROBOT;

public:
    CPowerElectrobot(const char *plugin_name);
    ~CPowerElectrobot();

    Cihm_PowerElectrobot *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_PowerElectrobot m_ihm;
    bool m_eeprom_unlocked;
    void updateOutputsLabels(void);
    void sendCommand(unsigned short command, unsigned short val);
    void enableDisable_SendEEPROM();


private slots :
    void onRightClicGUI(QPoint pos);

    // Onglet Général
    void onChanged_output1(bool val);
    void onChanged_output2(bool val);
    void onChanged_output3(bool val);
    void onChanged_output4(bool val);
    void onChanged_output5(bool val);
    void onChanged_output6(bool val);
    void onChanged_output7(bool val);
    void onChanged_output8(bool val);
    void onClicked_allOutputsOff();
    // Onglet Calibration
    void onClicked_CalibBattVoltagePoint1();
    void onClicked_CalibBattVoltagePoint2();
    void onClicked_CalibGlobalCurrentPoint1();
    void onClicked_CalibGlobalCurrentPoint2();
    void onClicked_CalibCurrentOut1Point1();
    void onClicked_CalibCurrentOut1Point2();
    void onClicked_CalibCurrentOut2Point1();
    void onClicked_CalibCurrentOut2Point2();

    // Onglet Spécial
    void onClicked_UnlockEEPROM();
    void onClicked_ResetFactoryEEPROM();
    void onClicked_ChangeI2CAddress();

};

#endif // _CPLUGIN_MODULE_PowerElectrobot_H_

/*! @} */

