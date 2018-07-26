/*! \file CEEPROM.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CEEPROM.h"

#include "CPrintView.h"

/*! \addtogroup EEPROM
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CEEPROM::CEEPROM(const char *plugin_name)
    :   CBasicModule(plugin_name, VERSION_EEPROM, AUTEUR_EEPROM, INFO_EEPROM),
        m_initialized(false)
{
    setNiveauTrace(MSG_TOUS);
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CEEPROM::~CEEPROM()
{
  if (m_settings) { delete m_settings; }
}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*  \remarks le nom du fichier est le nom du module .ini
*/
void CEEPROM::init(CApplication *application)
{
  QString pathfilename;

  CBasicModule::init(application);

  pathfilename = m_application->m_pathname_config_file + "/" + getName() + ".ini";
  m_settings = new QSettings(pathfilename, QSettings::IniFormat);
  m_settings->setIniCodec("UTF-8");
  m_application->m_print_view->print_debug(this, "Fichier EEPROM = " + pathfilename);

  m_initialized = true;
}


// _____________________________________________________________________
/*!
*  Ecrit la valeur d'un paramètre en EEPROM
*
*  \param [in] section nom de la section (nom du module à l'origine  de la demande)
*  \param [in] param nom du paramètre
*  \param [in] val valeur du paramètre au format générique QVariant
*/
void CEEPROM::write(QString section, QString param, QVariant val)
{
  QString completname;

  if (!m_initialized) { return; }

  if (section != "")    { completname = section + "/" + param; }
  else                  { completname = param; }
  m_mutex.lock();
  m_settings->setValue(completname, val);
  m_mutex.unlock();
  m_application->m_print_view->print_debug(this, "Ecriture du parametre: " + completname + " = " + val.toString());
}

// _____________________________________________________________________
/*!
*  Ecrit la valeur d'un paramètre en EEPROM
*
*  \param [in] param nom du paramètre
*  \param [in] val valeur du paramètre au format générique QVariant
*  \remarks l'écriture se fait dans la section "General"
*/
void CEEPROM::write(QString param, QVariant val)
{
  write("", param, val);
}


// _____________________________________________________________________
/*!
*  Lit la valeur d'un paramètre en EEPROM
*
*  \param [in] section nom de la section (nom du module à l'origine  de la demande)
*  \param [in] param nom du paramètre
*  \param [in] def_val valeur par défaut retournée si le paramètre n'existe pas en EEPROM
*  \remarks si la data n'existe pas, elle est cree automatiquement
*/
QVariant CEEPROM::read(QString section, QString param, QVariant def_val)
{
  QVariant val("");
  QString completname;

  if (!m_initialized) { return(val); }

  if (section != "")    { completname = section + "/" + param; }
  else                  { completname = param; }

  m_mutex.lock();
  val = m_settings->value(completname, def_val);
  m_mutex.unlock();
  m_application->m_print_view->print_debug(this, "Lecture du parametre: " + completname + " = " + val.toString());
  return(val);
}

// _____________________________________________________________________
/*!
*  Lit la valeur d'un paramètre en EEPROM
*
*  \param [in] param nom du paramètre
*  \param [in] def_val valeur par défaut retournée si le paramètre n'existe pas en EEPROM
*  \remarks la lecture se fait dans la section "General"
*/
QVariant CEEPROM::read(QString param, QVariant def_val)
{
 return(read("", param, def_val));
}







/*! @} */
