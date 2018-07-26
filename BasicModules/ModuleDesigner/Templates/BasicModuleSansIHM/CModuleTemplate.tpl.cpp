/*! \file CModuleTemplate.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CModuleTemplate.h"
#include "CLaBotBox.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"



/*! \addtogroup Module_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CModuleTemplate::CModuleTemplate(const char *plugin_name)
    :CBasicModule(plugin_name, VERSION_ModuleTemplate, AUTEUR_ModuleTemplate, INFO_ModuleTemplate)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CModuleTemplate::~CModuleTemplate()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CModuleTemplate::init(CLaBotBox *application)
{
  CModule::init(application);

  // Restore le niveau d'affichage
  QVariant val = m_application->m_eeprom->read(getName(), "niveau_trace", QVariant(MSG_TOUS));
  setNiveauTrace(val.toUInt());
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CModuleTemplate::close(void)
{
    m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
}







