/*! \file CModuleDesigner.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CMODULE_CREATOR_H_
#define _CMODULE_CREATOR_H_

#include "CPluginModule.h"
#include "ui_ihm_ModuleDesigner.h"


 /*! \addtogroup ModuleDesigner
   * 
   *  @{
   */

/*! @brief class Cihm_ModuleDesigner in @link PrintView basic module.
 */	   
class Cihm_ModuleDesigner : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_ModuleDesigner(QWidget *parent = 0)    : QMainWindow(parent)
	{
	   ui.setupUi(this);
	}
    ~Cihm_ModuleDesigner() { }

    Ui::ihm_ModuleDesigner ui;
 };
	   
	   
	   
	   
/*! @brief class CModuleDesigner in @link PrintView basic module.
 */	   
class CModuleDesigner : public CPluginModule
{
    Q_OBJECT
#define     VERSION_MODULE_CREATOR     "2.0"
#define     AUTEUR_MODULE_CREATOR      "Nico"
#define     INFO_MODULE_CREATOR        "Wizard de création/intégration/désintégration de modules"

#define 	PREFIXE_QRC_TEMPLATE_CODE "://Templates/"
		
public:
    CModuleDesigner(const char *plugin_name);
    ~CModuleDesigner();

    virtual void init(CLaBotBox *application);
    virtual void close(void);
    virtual bool hasGUI(void) 	{ return(true); }
    virtual QIcon getIcon(void) { return(QIcon(":/icons/designer.png")); }
    virtual QString getMenuName(void)  { return("Tools"); }

private slots :
    void onRightClicGUI(QPoint pos);

private:
    Cihm_ModuleDesigner m_ihm;

    QString readFile(QString pathfilename);
    void writeFile(QString path, QString filename, const QString &value);
    void writeFile(QString pathfilename, const QString &value);
    bool creerModule(void);
    bool genereSources(const QString &output_dirname);
    bool genereResources(const QString &output_dirname);
    bool genereResourcesUserGuideHtml(const QString &output_dirname);
    bool getCodeResourceUserGuide(QString &out_code);
    bool integrerNouveauModuleCreeAuProjet(void);
    bool integrerModuleAuProjet(QString type_module, QString repertoire_projet, QString nom_module);
    bool desintegrerModuleDuProjet(QString type_module, QString repertoire_projet, QString nom_module);

private slots :
    void genererModule(void);
    void choixRepertoireSortie(void);
    void choixRepertoireModuleExistant(void);
    bool integrerModuleExistantAuProjet(void);
    void choixRepertoireModuleDesintegration(void);
    bool desintegrerModuleExistantDuProjet(void);
};

#endif // _CMODULE_CREATOR_H_

/*! @} */

