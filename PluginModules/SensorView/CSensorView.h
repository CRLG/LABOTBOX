/*! \file CSensorView.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_SensorView_H_
#define _CPLUGIN_MODULE_SensorView_H_

#include <qled.h>
#include <QLCDNumber>
#include <QMainWindow>
#include <QTimer>

#include "CPluginModule.h"
#include "ui_ihm_SensorView.h"

typedef enum {
    mime_default,
    mime_sensor_tor,
    mime_sensor_ana,
    mime_computed_signal,
    mime_sensor_actuator
} eLBB_typeMime;

 class Cihm_SensorView : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_SensorView(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_SensorView() { }

    Ui::ihm_SensorView ui;

    CApplication *m_application;
 };



 /*! \addtogroup SensorView
   * 
   *  @{
   */

 class viewWidget : public QObject
 {
    Q_OBJECT
 public:
    viewWidget(QObject *parent = 0): QObject(parent){ }
    viewWidget(QString name, QPoint position, int type, QLCDNumber* widgetLCD){
        addedSignalName=name;
        addedSignalPosition=position;
        typeSignal=type;
        addedLCDNumber=widgetLCD;
    }
    viewWidget(QString name, QPoint position, int type, QLed* widgetLed){
        addedSignalName=name;
        addedSignalPosition=position;
        typeSignal=type;
        addedLed=widgetLed;
    }
    ~viewWidget() { }

     QString addedSignalName;
     QPoint addedSignalPosition;
     int typeSignal;

     QLCDNumber* addedLCDNumber;
     QLed* addedLed;
 };
	   
/*! @brief class CSensorView
 */	   
class CSensorView : public CPluginModule
{
    Q_OBJECT
#define     VERSION_SensorView   "1.0"
#define     AUTEUR_SensorView    "Laguiche"
#define     INFO_SensorView      "Visualise l'état des capteurs"

public:
    CSensorView(const char *plugin_name);
    ~CSensorView();

    Cihm_SensorView *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private slots :
    void onRightClicGUI(QPoint pos);

private:
    const int PERIODE_ECHANTILLONNAGE_VARIABLES = 100; // msec

    QList<QPoint> listePointsSignalsAdded;
    QStringList listeStringSignalsAdded;
    QList<viewWidget*> listeAddedSignals;
    QTimer m_timer_lecture_variables;
    bool isAdded(QString var_name);
    bool isLocked;

public slots:
    void refreshValeursVariables();
    void start(bool value);
    void lock(bool value);
    void uncheckAllSignals(void);

private slots:
    void refreshListeVariables();
    void addWidget(QString var_name, QPoint var_pos);
    void removeWidget(QString var_name);
private:
    Cihm_SensorView m_ihm;

};




#endif // _CPLUGIN_MODULE_SensorView_H_

/*! @} */

