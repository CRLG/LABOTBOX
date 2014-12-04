/*! \file CLaBotBox.h
 * A brief file description header.
 * A more elaborated file description.
 */

#ifndef _LABOTBOX_H_
#define _LABOTBOX_H_

#include <QObject>
#include <QList>

/*! \addtogroup LaBotBox
   *  Additional documentation for group DataManager
   *  @{
   */

class CDataManager;
class CMainWindow;
class CPrintView;
class CEEPROM;
class CDataView;
class CDataPlayer;
//##_NEW_CLASS_BASIC_MODULE_HERE_##


class CModuleDesigner;
class CDataGraph;
class CSimuBot;
class CStrategyDesigner;
class CSensorView;
//##_NEW_CLASS_PLUGIN_MODULE_HERE_##

class CBasicModule;
class CPluginModule;
class CModule;


typedef QList <CBasicModule *> t_list_basic_modules;
typedef QList <CPluginModule *> t_list_plugin_modules;
typedef QList <CModule *> t_list_modules;


/*! @brief class CLaBotBox in @link DataManager.
 */	   
class CLaBotBox : public QObject
{
    Q_OBJECT

public:
    CLaBotBox();
    ~CLaBotBox();

    void run(void);

public:
   CMainWindow          *m_main_windows;
   CDataManager         *m_data_center;
   CPrintView         *m_print_view;
   CEEPROM              *m_eeprom;
   CDataView     *m_DataView;
   CDataPlayer     *m_DataPlayer;
//##_NEW_BASIC_MODULE_CLASS_POINTER_DEFINITION_##

   CModuleDesigner       *m_module_creator;
   CDataGraph     *m_DataGraph;
   CSimuBot     *m_SimuBot;
   CStrategyDesigner     *m_StrategyDesigner;
   CSensorView     *m_SensorView;
//##_NEW_PLUGIN_MODULE_CLASS_POINTER_DEFINITION_##


   // Liste de l'ensemble des basic modules
   t_list_basic_modules m_list_basic_modules;

   // Liste de l'ensemble des plugin modules
   t_list_plugin_modules m_list_plugin_modules;

   // Liste de l'ensemble des modules (indépendamment du fait qu'il soit un BasicModule ou un PluginModule)
   t_list_modules m_list_modules;

public :
   //! Répertoire par défaut de sortie des fichiers de log
   QString m_pathname_log_file;
   //! Répertoire par défaut des fichiers de config
   QString m_pathname_config_file;

private:
   void createBasicModules(void);
   void initBasicModules(void);
   void closeBasicModules(void);
   void deleteBasicModules(void);

   void createPluginModules(void);
   void initPluginModules(void);
   void closePluginModules(void);
   void deletePluginModules(void);

public slots :
   void TermineApplication(void);


};


#endif // _LABOTBOX_H_

/*! @} */

