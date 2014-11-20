/*! \file CPluginModule.h
 * A brief Classe de base pour tous les PluginModules.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_H_
#define _CPLUGIN_MODULE_H_

#include <QObject>
#include <QString>

#include "CLaBotBox.h"
#include "CModule.h"

 /*! \addtogroup PluginModule
   * 
   *  @{
   */
	   
/*! @brief class CPluginModule in @link DataManager basic module.
 */	   
class CPluginModule : public CModule
{
//    Q_OBJECT

public:
    CPluginModule() { }
    CPluginModule(const char *name, const char *version, const char *auteur, const char *description)
        :CModule(name, version, auteur, description)
    {
    }
    virtual void init(CLaBotBox *application)   { CModule::init(application); }
    virtual bool hasGUI(void)                   { return(CModule::hasGUI()); }
    virtual QIcon getIcon(void)                 { return(CModule::getIcon()); }
    virtual QString getMenuName(void)           { return("PluginModule"); }
};

#endif // _CPLUGIN_MODULE_H_

/*! @} */

