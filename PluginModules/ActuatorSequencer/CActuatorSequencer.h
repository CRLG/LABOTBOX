/*! \file CActuatorSequencer.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_ActuatorSequencer_H_
#define _CPLUGIN_MODULE_ActuatorSequencer_H_

#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QHash>
#include <QList>
#include <QMap>
#include <QPair>
#include <QRegExp>
#include "CPluginModule.h"
#include "ui_ihm_ActuatorSequencer.h"

// Structure pour stocker une valeur d'énumération
struct EnumValue {
    QString name;
    int value;
};

// Structure pour stocker un enum complet
struct EnumDefinition {
    QString enumName;
    QList<EnumValue> values;
};

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

// Types connus dans le SEQUENCEUR
typedef enum {
    SD20 = 0,
    SERVO,
    AX_POSITION,
    AX_SPEED,
    MOTOR,
    POWER,
    ASSER,
    WAIT,
    EVENT,
    SENSOR,
    FREE_EVENT,
    FREE_ACTION,
    NODE,
    ARM
}tType;

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
    void Slot_Generate_CPP();
    void Slot_SetPosFromSimu();
private:
    Cihm_ActuatorSequencer m_ihm;
    bool bPlay,bResume,bStop;

    QList<QTableWidget *> listSequence;
    QList<QCheckBox *> listChoice;
    QList<int> strategies2Combine;

    QString m_defaultPath_Modelia;
    QString m_defaultPath_SoftCPU;

    QHash<QString, int> m_hash_const_Servo;
    QHash<QString, int> m_hash_const_SD20;
    QHash<QString, int> m_hash_const_AX;
    QHash<QString, int> m_hash_const_arm;
    //bool getEnum(QString fileName, QString enum_name, QHash<QString, int> * results);
    QList<EnumDefinition> getEnum(const QString &filePath);

    void updateComboBox();
    void update_sequenceButtons();


    //! La liste des codes possibles dans le champ "commande_ax" de la trame ELECTROBOT_CDE_SERVOS_AX
    // (enum commun CPU<->LaBotBox)
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


    bool isTransition(QString sType);
    void formatSequence(QTableWidget *table_sequence);
    int findState(QTableWidget *table_sequence, QString stateName);
    QString getTypeText(QTableWidget *table_sequence, int idx);
    QString getIdText(QTableWidget *table_sequence, int idx);
    QString getValueText(QTableWidget *table_sequence, int idx);
    QString getSateNameText(QTableWidget *table_sequence, int idx);
    QString getCommentsText(QTableWidget *table_sequence, int idx);
    void fillRow(QTableWidget *table_sequence, int idx, QTableWidgetItem *type, QTableWidgetItem *id, QTableWidgetItem *value, QTableWidgetItem *state, QTableWidgetItem *comments, bool symetrie, bool prototype);
    void setPlayMessage(int elapsed, QString type, QString msg);
    void creatTabSequence(QWidget * newTab, QTableWidget * newSequence, QString strName, bool isFirstSequence);
    bool getSymChecked(QTableWidget *sequence, int row);
    void setSymChecked(QTableWidget *sequence, int row, bool state);
    int getType(QString sActuator);
    QList<int> findTransitionsIn(QTableWidget *table_sequence, QString StateName);
    QList<int> findTransitionsOut(QTableWidget *table_sequence, QString StateName);
    bool getProtoChecked(QTableWidget *sequence, int row);
    void setProtoChecked(QTableWidget *sequence, int row, bool state);
private slots :
    void updateTooltip();
    void onRightClicGUI(QPoint pos);
    void Slot_Add_Sequence_Item();
    void removeSequenceItem();
    void Slot_Play(bool oneStep=false, int idStart=0);
    void Slot_Resume();
    void Slot_Stop();
    void Slot_Save();
    void Slot_Load();
    void Slot_Clear();
    void Slot_Play_only_Servo();
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
    void Slot_editFreeItem();
    void Slot_loadFreeItem();
    void Slot_setStrategyName_tab(QString strName);
    void Slot_setStrategyName_text(int index);
    void Slot_Delete();
    void Slot_Clone();
    void Slot_chooseStrategy(int state);
    void Slot_combineStrategies();
    void Slot_moveStrategy();
    void Slot_Generate_XML(QString strPath);
    //void Slot_formatStrategy(QColor newColor);
    void Slot_Play_only_arm();
    void Slot_Stop_only_arm();
    void Slot_Generate_SCXML();
};

#endif // _CPLUGIN_MODULE_ActuatorSequencer_H_

/*! @} */

