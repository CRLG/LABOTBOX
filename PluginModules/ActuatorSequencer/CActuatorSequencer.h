/*! \file CActuatorSequencer.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_ActuatorSequencer_H_
#define _CPLUGIN_MODULE_ActuatorSequencer_H_

#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include "CPluginModule.h"
#include "ui_ihm_ActuatorSequencer.h"

// Etat asserv
typedef enum {
cMOUVEMENT_EN_COURS = 0,
cCONVERGENCE_OK,
cBLOCAGE
}tConvergence;

// Etat asserv rack
typedef enum {
cSTOP = 0,
cDEPLACEMENT,
cCONVERGE
}tConvergenceRack;


 class Cihm_ActuatorSequencer : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_ActuatorSequencer(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_ActuatorSequencer() { }

    Ui::ihm_ActuatorSequencer ui;

    CApplication *m_application;
 };



 /*! \addtogroup ActuatorSequencer
   * 
   *  @{
   */

	   
/*! @brief class CActuatorSequencer
 */	   
class CActuatorSequencer : public CPluginModule
{
    Q_OBJECT
#define     VERSION_ActuatorSequencer   "1.0"
#define     AUTEUR_ActuatorSequencer    "Laguiche"
#define     INFO_ActuatorSequencer      "Ability to operate a sequence of actuators (basic states machine)"

public:
    CActuatorSequencer(const char *plugin_name);
    ~CActuatorSequencer();

    Cihm_ActuatorSequencer *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Pr�cise l'ic�ne qui repr�sente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Pr�cise le nom du menu de la fen�tre principale dans lequel le module appara�t

public slots:
    void Slot_Generate();
    void Slot_SetPosFromSimu();
private:
    Cihm_ActuatorSequencer m_ihm;
    bool bPlay,bResume,bStop;

    QList<QTableWidget *> listSequence;

    QString defaultPath;

    void updateComboBox();
    void update_sequenceButtons();

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


private slots :
    void updateTooltip();
    void onRightClicGUI(QPoint pos);
    void addSequenceItem();
    void removeSequenceItem();
    void Slot_Play(bool oneStep=false, int idStart=0);
    void Slot_Resume();
    void Slot_Stop();
    void Slot_Save();
    void Slot_Load();
    void Slot_Clear();
    void Slot_Play_only_SD20();
    void Slot_Play_only_AX();
    void Slot_Play_only_motor();
    void Slot_Stop_only_motors();
    void Slot_Play_only_power();
    void Slot_Stop_only_power();
    QTableWidget *Slot_Add_Sequence();
    void Slot_Remove_Sequence(int index);
    void Slot_Stop_only_asser();
    void Slot_Play_only_asser();
    void Slot_convert_to_rad();
    void Slot_convert_to_deg();
    void Slot_get_XYTeta();
    void Slot_up();
    void Slot_down();
    void setEnableRepetition(bool state);
    void playStep();
};

#endif // _CPLUGIN_MODULE_ActuatorSequencer_H_

/*! @} */

