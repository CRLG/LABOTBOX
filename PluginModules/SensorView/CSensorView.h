/*! \file CSensorView.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_SensorView_H_
#define _CPLUGIN_MODULE_SensorView_H_

#include <qled.h>
#include <QLCDNumber>
#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_SensorView.h"

 class Cihm_SensorView : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_SensorView(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_SensorView() { }

    Ui::ihm_SensorView ui;

    CLaBotBox *m_application;
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
    viewWidget(QString name, QPoint position){
        addedSignalName=name;
        addedSignalPosition=position;
    }
    ~viewWidget() { }

     QString addedSignalName;
     QPoint addedSignalPosition;
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

    virtual void init(CLaBotBox *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît
private:
    QList<QPoint> listePointsSignalsAdded;
    QStringList listeStringSignalsAdded;
    QList<viewWidget*> listeAddedSignals;

private slots:
    void refreshListeVariables();
    void addWidget(QString var_name, QPoint var_pos);
private:
    Cihm_SensorView m_ihm;
};




#endif // _CPLUGIN_MODULE_SensorView_H_

/*! @} */

