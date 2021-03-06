/*! \file CTestUnitaire.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_TestUnitaire_H_
#define _CPLUGIN_MODULE_TestUnitaire_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_TestUnitaire.h"
#include "QProtectedQueue.h"
#include "CExternalControlerClient.h"

 class Cihm_TestUnitaire : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_TestUnitaire(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_TestUnitaire() { }

    Ui::ihm_TestUnitaire ui;

    CApplication *m_application;
 };



 /*! \addtogroup TestUnitaire
   * 
   *  @{
   */

	   
/*! @brief class CTestUnitaire
 */	   
class CTestUnitaire : public CPluginModule
{
    Q_OBJECT
#define     VERSION_TestUnitaire   "1.0"
#define     AUTEUR_TestUnitaire    "Nico"
#define     INFO_TestUnitaire      "Test unitaire de module"

public:
    CTestUnitaire(const char *plugin_name);
    ~CTestUnitaire();

    Cihm_TestUnitaire *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private slots :
    void onRightClicGUI(QPoint pos);

private:
    Cihm_TestUnitaire m_ihm;
    CExternalControlerClient m_external_client;

public slots :
    void cb_Bouton(void);
    void cb_Bouton2(void);
    void cb_Bouton3(void);
    void receive_rs232(QByteArray data);
    void connected_rs232(void);
    void disconnected_rs232(void);

    void connected_to_robot(bool state);

    void connected_to_external();
    void disconnected_to_external();

};


#endif // _CPLUGIN_MODULE_TestUnitaire_H_

/*! @} */

