/*! \file CBasicModule.h
 * A brief Classe de base pour tous les BasicModules.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_H_
#define _CBASIC_MODULE_H_

#include <QObject>
#include <QString>

#include "CLaBotBox.h"
#include "CModule.h"

 /*! \addtogroup BasicModule
   * 
   *  @{
   */
	   
/*! @brief class CBasicModule in @link DataManager basic module.
 */	   
class CBasicModule : public CModule
{
//    Q_OBJECT

public:
    CBasicModule() { }
    CBasicModule(const char *name, const char *version, const char *auteur, const char *description)
        :CModule(name, version, auteur, description)
    {
    }
    virtual void init(CLaBotBox *application) { CModule::init(application); }
    virtual QString getMenuName(void)  { return("BasicModule"); }

};

#endif // _CBASIC_MODULE_H_

/*! @} */

