/*! \file CModuleTemplate.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_ModuleTemplate_H_
#define _CBASIC_MODULE_ModuleTemplate_H_

#include <QMainWindow>

#include "CBasicModule.h"
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
class CModuleTemplate : public CBasicModule
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
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Pr�cise l'ic�ne qui repr�sente le module
    virtual QString getMenuName(void)   { return("BasicModule"); }                  // Pr�cise le nom du menu de la fen�tre principale dans lequel le module appara�t

private:
    Cihm_ModuleTemplate m_ihm;
};

#endif // _CBASIC_MODULE_ModuleTemplate_H_

/*! @} */

