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
    void updateButeesServos(void);

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

// ____________________________________________________Servo-moteurs
private slots :
    void CdeServoMoteur1_changed(int val);
    void CdeServoMoteur2_changed(int val);
    void CdeServoMoteur3_changed(int val);
    void CdeServoMoteur4_changed(int val);
    void CdeServoMoteur5_changed(int val);
    void CdeServoMoteur6_changed(int val);
    void CdeServoMoteur7_changed(int val);
    void CdeServoMoteur8_changed(int val);

    void ButeeServo_changed(void);

private :
    QVector<unsigned char> m_butees_min;
    QVector<unsigned char> m_butees_max;
};

#endif // _CPLUGIN_MODULE_ActuatorElectrobot_H_

/*! @} */

