/*! \file CEEPROM.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CEEPROM_H_
#define _CEEPROM_H_

#include <QVariant>
#include <QMutex>
#include <QSettings>

#include "CBasicModule.h"

 /*! \addtogroup DataManager
   * 
   *  @{
   */

	   
/*! @brief class CEEPROM in @link DataManager basic module.
 */	   
class CEEPROM : public CBasicModule
{
    Q_OBJECT

private :
#define     VERSION_EEPROM         "1.0"
#define     AUTEUR_EEPROM          "Nico"
#define     INFO_EEPROM            "Gestionnaire de la sauvegarde des param�tres de configuration"


public:
    CEEPROM(const char *plugin_name);
    ~CEEPROM();

    void write(QString section, QString param, QVariant val);
    void write(QString param, QVariant val);
    QVariant read(QString section, QString param, QVariant def_val);
    QVariant read(QString param, QVariant def_val);

    void init(CLaBotBox *application);
private:
    //! Mutex de protection d'acc�s multiple � la ressource partag�e
    QMutex m_mutex;

    //! Ressource de m�morisation propre Qt
    QSettings *m_settings;

    //! Indique si l'initialisation du module � d�j� �t�t faite
    bool m_initialized;

};

#endif // _CEEPROM_H_

/*! @} */

