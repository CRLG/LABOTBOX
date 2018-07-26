/*! \file CModuleTemplate.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_ModuleTemplate_H_
#define _CBASIC_MODULE_ModuleTemplate_H_

#include <QMainWindow>

#include "CBasicModule.h"


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

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void) { return(false); }
    virtual QIcon getIcon(void) { return(QIcon(":/icons/edit_add.png")); }
};

#endif // _CBASIC_MODULE_ModuleTemplate_H_

/*! @} */

