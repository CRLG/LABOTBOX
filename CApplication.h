/*! \file CApplication.h
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
class CRS232;
class CJoystick;
class CEcran;
class CExternalControler;
class CRaspiGPIO;
//##_NEW_CLASS_BASIC_MODULE_HERE_##


class CModuleDesigner;
class CTestUnitaire;
class CDataGraph;
class CSimuBot;
class CStrategyDesigner;
class CMessagerieBot;
class CSensorElectroBot;
class CActuatorElectrobot;
class CSensorView;
class CAsserv;
class CUserGuides;
class CActuatorSequencer;
class CBotCam;
class CImageProcessing;
class CXBEE;
class CXbeeNetworkMessenger;
class CBalise;
//##_NEW_CLASS_PLUGIN_MODULE_HERE_##

class CBasicModule;
class CPluginModule;
class CModule;


typedef QList <CBasicModule *> t_list_basic_modules;
typedef QList <CPluginModule *> t_list_plugin_modules;
typedef QList <CModule *> t_list_modules;


/*! @brief class CApplication in @link DataManager.
 */	   
class CApplication : public QObject
{
    Q_OBJECT

public:
    CApplication();
    ~CApplication();

    void run(void);

public:
#ifdef MODULE_MainWindow
   CMainWindow          *m_main_windows;
#endif // MODULE_MainWindow
#ifdef MODULE_DataManager
   CDataManager         *m_data_center;
#endif // MODULE_DataManager
#ifdef MODULE_PrintView
   CPrintView         *m_print_view;
#endif // MODULE_PrintView
#ifdef MODULE_EEPROM
   CEEPROM              *m_eeprom;
#endif // MODULE_EEPROM
#ifdef MODULE_DataView
   CDataView     *m_DataView;
#endif // MODULE_DataView
#ifdef MODULE_DataGraph
   CDataGraph     *m_DataGraph;
#endif // MODULE_DataGraph
#ifdef MODULE_DataPlayer
   CDataPlayer     *m_DataPlayer;
#endif // MODULE_DataPlayer
#ifdef MODULE_RS232
   CRS232     *m_RS232_robot;
#endif // MODULE_RS232
#ifdef MODULE_RS232
   CRS232     *m_RS232_cmucam;
#endif // MODULE_RS232
#ifdef MODULE_RS232
   CRS232     *m_RS232_xbee;
#endif // MODULE_RS232
#ifdef MODULE_Joystick
   CJoystick     *m_Joystick;
#endif // MODULE_Joystick
#ifdef MODULE_UserGuides
   CUserGuides     *m_UserGuides;
#endif // MODULE_UserGuides
#ifdef MODULE_ModuleDesigner
   CModuleDesigner       *m_module_creator;
#endif // MODULE_ModuleDesigner
#ifdef MODULE_ExternalControler
   CExternalControler     *m_ExternalControler;
#endif // MODULE_ExternalControler
   //##_NEW_BASIC_MODULE_CLASS_POINTER_DEFINITION_##
#ifdef MODULE_RaspiGPIO
   CRaspiGPIO     *m_RaspiGPIO;
#endif // MODULE_RaspiGPIO

#ifdef MODULE_TestUnitaire
   CTestUnitaire     *m_TestUnitaire;
#endif // MODULE_TestUnitaire
#ifdef MODULE_SimuBot
   CSimuBot     *m_SimuBot;
#endif // MODULE_SimuBot
#ifdef MODULE_StrategyDesigner
   CStrategyDesigner     *m_StrategyDesigner;
#endif // MODULE_StrategyDesigner
#ifdef MODULE_MessagerieBot
   CMessagerieBot     *m_MessagerieBot;
#endif // MODULE_MessagerieBot
#ifdef MODULE_SensorElectroBot
   CSensorElectroBot     *m_SensorElectroBot;
#endif // MODULE_SensorElectroBot
#ifdef MODULE_ActuatorElectrobot
   CActuatorElectrobot     *m_ActuatorElectrobot;
#endif // MODULE_ActuatorElectrobot
#ifdef MODULE_SensorView
   CSensorView     *m_SensorView;
#endif // MODULE_SensorView
#ifdef MODULE_Asserv
   CAsserv     *m_Asserv;
#endif // MODULE_Asserv
#ifdef MODULE_ActuatorSequencer
   CActuatorSequencer     *m_ActuatorSequencer;
#endif // MODULE_ActuatorSequencer
#ifdef MODULE_BotCam
   CBotCam     *m_BotCam;
#endif // MODULE_BotCam
#ifdef MODULE_Ecran
   CEcran     *m_Ecran;
#endif // MODULE_Ecran
#ifdef MODULE_ImageProcessing
   CImageProcessing     *m_ImageProcessing;
#endif // MODULE_ImageProcessing
#ifdef MODULE_XBEE
   CXBEE     *m_XBEE;
#endif // MODULE_XBEE
#ifdef MODULE_XbeeNetworkMessenger
   CXbeeNetworkMessenger     *m_XbeeNetworkMessenger;
#endif // MODULE_XbeeNetworkMessenger
#ifdef MODULE_Balise
   CBalise     *m_Balise;
#endif // MODULE_Balise
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

