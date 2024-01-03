/*! \file CDataView.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_DataView_H_
#define _CBASIC_MODULE_DataView_H_

#include <QMainWindow>
#include <QTimer>

#include "CBasicModule.h"
#include "ui_ihm_DataView.h"

 class Cihm_DataView : public QMainWindow
{
    Q_OBJECT
public:
    enum    // Numéro de colonnes dans le widget
    {
         COL_TEMPS = 0,
         COL_NOM,
         COL_VALEUR
    };

     Cihm_DataView(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_DataView() { }

    Ui::ihm_DataView ui;

    CApplication *m_application;
 };



 /*! \addtogroup DataView
   * 
   *  @{
   */

	   
/*! @brief class CDataView
 */	   
class CDataView : public CBasicModule
{
    Q_OBJECT
#define     VERSION_DataView   "1.3"
#define     AUTEUR_DataView    "Nico"
#define     INFO_DataView      "Visualise l'évolution d'une ou plusieurs données du DataCenter"

public:
    CDataView(const char *plugin_name);
    ~CDataView();

    Cihm_DataView *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/eye.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("BasicModule"); }
    virtual void setVisible(void);

private :
    const int PERIODE_ECHANTILLONNAGE_VARIABLES = 30; // msec

    void activeInspectorTemporel(void);
    void activeInspectorInstantane(void);
    void finInspectorTemporel(void);
    void finInspectorInstantane(void);
    void connectDiscconnectVariablesTemporel(bool choix, bool only_diff);
    void addTraceVariable(QString name, QString value = "", quint64 time=0);
    double normaliseTemps(qint64 ms_epoch);
    void addVariablesObserver(QStringList liste_variables);
    QStringList getListeVariablesObservees();

private slots :
    void onRightClicGUI(QPoint pos);
    void onDataFilterChanged(QString filter_name);

private:
    Cihm_DataView m_ihm;

    QTimer m_timer_lecture_variables;
    qint64 m_start_time;

public slots : 
    void variableChangedUpdated(QVariant val, quint64 update_time);
    void refreshListeVariables(void);
    void activeInspector(bool state);
    void refreshValeursVariables(void);
    void upVariable(void);
    void downVariable(void);
    void effacerListeVariablesValues(void);
    void editingFinishedWriteValueInstantane(void);
    void saveToFile();
    void loadListeVariablesObservees(void);
    void saveListeVariablesObservees(void);
    void decocheToutesVariables();


};

#endif // _CBASIC_MODULE_DataView_H_

/*! @} */

