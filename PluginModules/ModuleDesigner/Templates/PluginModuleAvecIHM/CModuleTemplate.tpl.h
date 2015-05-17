/*! \file CModuleTemplate.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_ModuleTemplate_H_
#define _CPLUGIN_MODULE_ModuleTemplate_H_

#include <QMainWindow>

#include "CPluginModule.h"
#include "ui_ihm_ModuleTemplate.h"

 class Cihm_ModuleTemplate : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_ModuleTemplate(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_ModuleTemplate() { }

    Ui::ihm_ModuleTemplate ui;

    CLaBotBox *m_application;
 };



 /*! \addtogroup ModuleTemplate
   * 
   *  @{
   */

	   
/*! @brief class CModuleTemplate
 */	   
class CModuleTemplate : public CPluginModule
{
    Q_OBJECT
#define     VERSION_ModuleTemplate   "1.0"
#define     AUTEUR_ModuleTemplate    "##AUTEUR##"
#define     INFO_ModuleTemplate      "##DESCRIPTION##"

public:
    CModuleTemplate(const char *plugin_name);
    ~CModuleTemplate();

    Cihm_ModuleTemplate *getIHM(void) { return(&m_ihm); }

    virtual void init(CLaBotBox *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

private:
    Cihm_ModuleTemplate m_ihm;

private slots :
    void onRightClicGUI(QPoint pos);
};

#endif // _CPLUGIN_MODULE_ModuleTemplate_H_

/*! @} */

