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
    Cihm_DataView(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_DataView() { }

    Ui::ihm_DataView ui;

    CLaBotBox *m_application;
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
#define     VERSION_DataView   "1.1"
#define     AUTEUR_DataView    "Nico"
#define     INFO_DataView      "Visualise l'�volution d'une ou plusieurs donn�es du DataCenter"

#define     PERIODE_ECHANTILLONNAGE_VARIABLES   30 // msec

public:
    CDataView(const char *plugin_name);
    ~CDataView();

    Cihm_DataView *getIHM(void) { return(&m_ihm); }

    virtual void init(CLaBotBox *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/eye.png")); }  // Pr�cise l'ic�ne qui repr�sente le module
    virtual QString getMenuName(void)   { return("BasicModule"); }
    virtual void setVisible(void);

private :
    void activeInspectorTemporel(void);
    void activeInspectorInstantane(void);
    void finInspectorTemporel(void);
    void finInspectorInstantane(void);
    void connectDiscconnectVariablesTemporel(bool choix);
    void addTraceVariable(QString name, QString value = "");

private slots :
    void onRightClicGUI(QPoint pos);

private:
    Cihm_DataView m_ihm;

    QTimer m_timer_lecture_variables;

public slots : 
    void variableChanged(QVariant val);
    void refreshListeVariables(void);
    void activeInspector(bool state);
    void refreshValeursVariables(void);
    void upVariable(void);
    void downVariable(void);
    void effacerListeVariablesValues(void);
    void editingFinishedWriteValueInstantane(void);


};

#endif // _CBASIC_MODULE_DataView_H_

/*! @} */

