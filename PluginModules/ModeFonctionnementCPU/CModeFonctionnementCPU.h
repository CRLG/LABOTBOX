/*! \file CModeFonctionnementCPU.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_ModeFonctionnementCPU_H_
#define _CPLUGIN_MODULE_ModeFonctionnementCPU_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_ModeFonctionnementCPU.h"

 class Cihm_ModeFonctionnementCPU : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_ModeFonctionnementCPU(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_ModeFonctionnementCPU() { }

    Ui::ihm_ModeFonctionnementCPU ui;

    CApplication *m_application;
 };



 /*! \addtogroup ModeFonctionnementCPU
   * 
   *  @{
   */

	   
/*! @brief class CModeFonctionnementCPU
 */	   
class CModeFonctionnementCPU : public CPluginModule
{
    Q_OBJECT
#define     VERSION_ModeFonctionnementCPU   "1.0"
#define     AUTEUR_ModeFonctionnementCPU    "Nico"
#define     INFO_ModeFonctionnementCPU      "Mode de fonctionnement du CPU"

public:
    CModeFonctionnementCPU(const char *plugin_name);
    ~CModeFonctionnementCPU();

    Cihm_ModeFonctionnementCPU *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_ModeFonctionnementCPU m_ihm;

private slots :
    void onRightClicGUI(QPoint pos);
    void on_send();
    void on_reset_cpu();
};

#endif // _CPLUGIN_MODULE_ModeFonctionnementCPU_H_

/*! @} */

