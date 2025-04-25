/*! \file CEEPROM_CPU.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_EEPROM_CPU_H_
#define _CPLUGIN_MODULE_EEPROM_CPU_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_EEPROM_CPU.h"

 class Cihm_EEPROM_CPU : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_EEPROM_CPU(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_EEPROM_CPU() { }

    Ui::ihm_EEPROM_CPU ui;

    CApplication *m_application;
 };



 /*! \addtogroup EEPROM_CPU
   * 
   *  @{
   */

	   
/*! @brief class CEEPROM_CPU
 */	   
class CEEPROM_CPU : public CPluginModule
{
    Q_OBJECT
#define     VERSION_EEPROM_CPU   "1.0"
#define     AUTEUR_EEPROM_CPU    "Nico"
#define     INFO_EEPROM_CPU      "Lecture / Ecriture EEPROM interne CPU"

public:
    CEEPROM_CPU(const char *plugin_name);
    ~CEEPROM_CPU();

    Cihm_EEPROM_CPU *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_EEPROM_CPU m_ihm;

private slots :
    void onRightClicGUI(QPoint pos);
    void onTest();
};

#endif // _CPLUGIN_MODULE_EEPROM_CPU_H_

/*! @} */

