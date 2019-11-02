/*! \file CApplication.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

#ifdef MODULE_MainWindow
   #include "CMainWindow.h"
#endif // MODULE_MainWindow
#ifdef MODULE_DataManager
   #include "CDataManager.h"
#endif // MODULE_
#ifdef MODULE_PrintView
   #include "CPrintView.h"
#endif // MODULE_PrintView
#ifdef MODULE_EEPROM
   #include "CEEPROM.h"
#endif // MODULE_EEPROM
#ifdef MODULE_DataView
   #include "CDataView.h"
#endif // MODULE_DataView
#ifdef MODULE_DataGraph
   #include "CDataGraph.h"
#endif // MODULE_DataGraph
#ifdef MODULE_DataPlayer
   #include "CDataPlayer.h"
#endif // MODULE_DataPlayer
#ifdef MODULE_RS232
   #include "CRS232.h"
#endif // MODULE_RS232
#ifdef MODULE_Joystick
   #include "CJoystick.h"
#endif // MODULE_Joystick
#ifdef MODULE_ModuleDesigner
   #include "CModuleDesigner.h"
#endif // MODULE_ModuleDesigner
#ifdef MODULE_UserGuides
   #include "CUserGuides.h"
#endif // MODULE_UserGuides
#ifdef MODULE_ExternalControler
    #include "CExternalControler.h"
#endif // MODULE_ExternalControler
#ifdef MODULE_RaspiGPIO
    #include "CRaspiGPIO.h"
#endif  // RASPBERRY_PI
//_##NEW_INCLUDE_BASIC_MODULE_HERE_##

#ifdef MODULE_TestUnitaire
   #include "CTestUnitaire.h"
#endif // MODULE_TestUnitaire
#ifdef MODULE_SimuBot
   #include "CSimuBot.h"
#endif // MODULE_SimuBot
#ifdef MODULE_StrategyDesigner
   #include "CStrategyDesigner.h"
#endif // MODULE_StrategyDesigner
#ifdef MODULE_MessagerieBot
   #include "CMessagerieBot.h"
#endif // MODULE_MessagerieBot
#ifdef MODULE_SensorElectroBot
   #include "CSensorElectroBot.h"
#endif // MODULE_SensorElectroBot
#ifdef MODULE_ActuatorElectrobot
   #include "CActuatorElectrobot.h"
#endif // MODULE_ActuatorElectrobot
#ifdef MODULE_SensorView
   #include "CSensorView.h"
#endif // MODULE_SensorView
#ifdef MODULE_Asserv
   #include "CAsserv.h"
#endif // MODULE_Asserv
#ifdef MODULE_ActuatorSequencer
   #include "CActuatorSequencer.h"
#endif // MODULE_ActuatorSequencer
#ifdef MODULE_BotCam
   #include "CBotCam.h"
#endif // MODULE_BotCam
#ifdef MODULE_Ecran
   #include "CEcran.h"
#endif // MODULE_Ecran
#ifdef MODULE_ImageProcessing
   #include "CImageProcessing.h"
#endif // MODULE_ImageProcessing
#ifdef MODULE_XBEE
   #include "CXBEE.h"
#endif // MODULE_XBEE
#ifdef MODULE_XbeeNetworkMessenger
   #include "CXbeeNetworkMessenger.h"
#endif // MODULE_XbeeNetworkMessenger
#ifdef MODULE_Balise
   #include "CBalise.h"
#endif // MODULE_Balise
#ifdef MODULE_PowerElectrobot
   #include "CPowerElectrobot.h"
#endif // MODULE_PowerElectrobot
//_##NEW_INCLUDE_PLUGIN_MODULE_HERE_##

#include "CApplication.h"


/*! \addtogroup DataManager
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CApplication::CApplication()
    :   m_pathname_log_file("./Log"),
        m_pathname_config_file("./Config")
{
  // S'assure que le répertoire de sortie des logs existe bien
  QDir dir;
  dir.mkpath(m_pathname_log_file);

  createBasicModules();
  createPluginModules();
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CApplication::~CApplication()
{
  closePluginModules();
  closeBasicModules();

  for (int i=0; i<m_list_basic_modules.size(); i++) {
    if(m_list_basic_modules[i]) {
        delete m_list_basic_modules[i];
    }
  }
}


// _____________________________________________________________________
/*!
*  Cree chaque basic module et l'ajoute à la liste
*
*/
void CApplication::createBasicModules(void)
{
  // Mettre en premier les modules indépendants des autres modules
#ifdef MODULE_PrintView
  m_print_view    = new CPrintView("PrintView");
  m_list_basic_modules.append(m_print_view);
  m_list_modules.append(m_print_view);
#endif // MODULE_PrintView

#ifdef MODULE_EEPROM
  m_eeprom    = new CEEPROM("EEPROM");
  m_list_basic_modules.append(m_eeprom);
  m_list_modules.append(m_eeprom);
#endif // MODULE_

#ifdef MODULE_MainWindow
  m_main_windows    = new CMainWindow("MainWindow");
  m_list_basic_modules.append(m_main_windows);
  m_list_modules.append(m_main_windows);
#endif // MODULE_MainWindow

#ifdef MODULE_DataManager
  m_data_center     = new CDataManager("DataCenter");
  m_list_basic_modules.append(m_data_center);
  m_list_modules.append(m_data_center);
#endif // MODULE_DataManager

#ifdef MODULE_DataView
  m_DataView     = new CDataView("DataView");
  m_list_basic_modules.append(m_DataView);
  m_list_modules.append(m_DataView);
#endif // MODULE_DataView

#ifdef MODULE_DataGraph
  m_DataGraph     = new CDataGraph("DataGraph");
  m_list_basic_modules.append(m_DataGraph);
  m_list_modules.append(m_DataGraph);
#endif // MODULE_DataGraph

#ifdef MODULE_DataPlayer
  m_DataPlayer     = new CDataPlayer("DataPlayer");
  m_list_basic_modules.append(m_DataPlayer);
  m_list_modules.append(m_DataPlayer);
#endif // MODULE_DataPlayer

#ifdef MODULE_RS232
  m_RS232_robot   = new CRS232("RS232_robot");
  m_list_basic_modules.append(m_RS232_robot);
  m_list_modules.append(m_RS232_robot);
#endif // MODULE_RS232

#ifdef MODULE_RS232
  m_RS232_cmucam   = new CRS232("RS232_cmucam");
  m_list_basic_modules.append(m_RS232_cmucam);
  m_list_modules.append(m_RS232_cmucam);
#endif // MODULE_RS232

#ifdef MODULE_RS232
  m_RS232_xbee  = new CRS232("RS232_xbee");
  m_list_basic_modules.append(m_RS232_xbee);
  m_list_modules.append(m_RS232_xbee);
#endif // MODULE_RS232

#ifdef MODULE_Joystick
  m_Joystick     = new CJoystick("Joystick");
  m_list_basic_modules.append(m_Joystick);
  m_list_modules.append(m_Joystick);
#endif // MODULE_Joystick

#ifdef MODULE_UserGuides
  m_UserGuides     = new CUserGuides("UserGuides");
  m_list_basic_modules.append(m_UserGuides);
  m_list_modules.append(m_UserGuides);
#endif // MODULE_UserGuides

#ifdef MODULE_ModuleDesigner
  m_module_creator    = new CModuleDesigner("ModuleDesigner");
  m_list_basic_modules.append(m_module_creator);
  m_list_modules.append(m_module_creator);
#endif // MODULE_ModuleDesigner

#ifdef MODULE_ExternalControler
  m_ExternalControler     = new CExternalControler("ExternalControler");
  m_list_basic_modules.append(m_ExternalControler);
  m_list_modules.append(m_ExternalControler);
#endif // MODULE_ExternalControler

#ifdef MODULE_RaspiGPIO
  m_RaspiGPIO     = new CRaspiGPIO("RaspiGPIO");
  m_list_basic_modules.append(m_RaspiGPIO);
  m_list_modules.append(m_RaspiGPIO);
#endif // MODULE_RaspiGPIO

// ##_NEW_BASIC_MODULE_INSTANCIATION_HERE_##
}

// _____________________________________________________________________
/*!
*  Initialise chaque basic module
*
*/
void CApplication::initBasicModules(void)
{
  for (int i=0; i<m_list_basic_modules.size(); i++) {
      m_list_basic_modules[i]->init(this);
  }
}
// _____________________________________________________________________
/*!
*  Termine chaque basic module
*
* \remarks les modules sont déchargés dans le sens inverse du chargement
*/
void CApplication::closeBasicModules(void)
{
  for (int i=m_list_basic_modules.size()-1; i>=0; i--) {
      m_list_basic_modules[i]->close();
  }
}

// _____________________________________________________________________
/*!
*  Libère les instances de chaque basic module
*
* \remarks les modules sont supprimés dans le sens inverse de la création
*/
void CApplication::deleteBasicModules()
{
    for (int i=m_list_basic_modules.size()-1; i>=0; i--) {
        if (m_list_basic_modules[i]) {
            delete m_list_basic_modules[i];
            m_list_basic_modules[i] = NULL;
        }
    }
    m_list_basic_modules.clear();
}


// _____________________________________________________________________
/*!
*  Cree chaque plugin module et l'ajoute à la liste
*
*/
void CApplication::createPluginModules(void)
{
#ifdef MODULE_TestUnitaire
  m_TestUnitaire     = new CTestUnitaire("TestUnitaire");
  m_list_plugin_modules.append(m_TestUnitaire);
  m_list_modules.append(m_TestUnitaire);
#endif // MODULE_TestUnitaire

#ifdef MODULE_SimuBot
  m_SimuBot     = new CSimuBot("SimuBot");
  m_list_plugin_modules.append(m_SimuBot);
  m_list_modules.append(m_SimuBot);
#endif // MODULE_SimuBot

#ifdef MODULE_StrategyDesigner
  m_StrategyDesigner     = new CStrategyDesigner("StrategyDesigner");
  m_list_plugin_modules.append(m_StrategyDesigner);
  m_list_modules.append(m_StrategyDesigner);
#endif // MODULE_StrategyDesigner

#ifdef MODULE_MessagerieBot
  m_MessagerieBot     = new CMessagerieBot("MessagerieBot");
  m_list_plugin_modules.append(m_MessagerieBot);
  m_list_modules.append(m_MessagerieBot);
#endif // MODULE_MessagerieBot

#ifdef MODULE_SensorElectroBot
  m_SensorElectroBot     = new CSensorElectroBot("SensorElectroBot");
  m_list_plugin_modules.append(m_SensorElectroBot);
  m_list_modules.append(m_SensorElectroBot);
#endif // MODULE_SensorElectroBot

#ifdef MODULE_ActuatorElectrobot
  m_ActuatorElectrobot     = new CActuatorElectrobot("ActuatorElectrobot");
  m_list_plugin_modules.append(m_ActuatorElectrobot);
  m_list_modules.append(m_ActuatorElectrobot);
#endif // MODULE_ActuatorElectrobot

#ifdef MODULE_SensorView
  m_SensorView     = new CSensorView("SensorView");
  m_list_plugin_modules.append(m_SensorView);
  m_list_modules.append(m_SensorView);
#endif // MODULE_SensorView

#ifdef MODULE_Asserv
  m_Asserv     = new CAsserv("Asserv");
  m_list_plugin_modules.append(m_Asserv);
  m_list_modules.append(m_Asserv);
#endif // MODULE_Asserv

#ifdef MODULE_ActuatorSequencer
  m_ActuatorSequencer     = new CActuatorSequencer("ActuatorSequencer");
  m_list_plugin_modules.append(m_ActuatorSequencer);
  m_list_modules.append(m_ActuatorSequencer);
#endif // MODULE_ActuatorSequencer

#ifdef MODULE_BotCam
  m_BotCam     = new CBotCam("BotCam");
  m_list_plugin_modules.append(m_BotCam);
  m_list_modules.append(m_BotCam);
#endif // MODULE_BotCam

#ifdef MODULE_Ecran
  m_Ecran     = new CEcran("Ecran");
  m_list_plugin_modules.append(m_Ecran);
  m_list_modules.append(m_Ecran);
#endif // MODULE_Ecran

#ifdef MODULE_ImageProcessing
  m_ImageProcessing     = new CImageProcessing("ImageProcessing");
  m_list_plugin_modules.append(m_ImageProcessing);
  m_list_modules.append(m_ImageProcessing);
#endif // MODULE_ImageProcessing

#ifdef MODULE_XBEE
  m_XBEE     = new CXBEE("XBEE");
  m_list_plugin_modules.append(m_XBEE);
  m_list_modules.append(m_XBEE);
#endif // MODULE_XBEE

#ifdef MODULE_XbeeNetworkMessenger
  m_XbeeNetworkMessenger     = new CXbeeNetworkMessenger("XbeeNetworkMessenger");
  m_list_plugin_modules.append(m_XbeeNetworkMessenger);
  m_list_modules.append(m_XbeeNetworkMessenger);
#endif // MODULE_XbeeNetworkMessenger

#ifdef MODULE_Balise
  m_Balise     = new CBalise("Balise");
  m_list_plugin_modules.append(m_Balise);
  m_list_modules.append(m_Balise);
#endif // MODULE_Balise

#ifdef MODULE_PowerElectrobot
  m_PowerElectrobot     = new CPowerElectrobot("PowerElectrobot");
  m_list_plugin_modules.append(m_PowerElectrobot);
  m_list_modules.append(m_PowerElectrobot);
#endif // MODULE_PowerElectrobot

// ##_NEW_PLUGIN_MODULE_INSTANCIATION_HERE_##
}

// _____________________________________________________________________
/*!
*  Initialise chaque plugin module
*
*/
void CApplication::initPluginModules(void)
{
  for (int i=0; i<m_list_plugin_modules.size(); i++) {
      m_list_plugin_modules[i]->init(this);
  }
}
// _____________________________________________________________________
/*!
*  Termine chaque plugin module
*
* \remarks les modules sont déchargés dans le sens inverse du chargement
*/
void CApplication::closePluginModules(void)
{
  for (int i=m_list_plugin_modules.size()-1; i>=0; i--) {
      m_list_plugin_modules[i]->close();
  }
}

// _____________________________________________________________________
/*!
*  Libère les instances de chaque basic module
*
* \remarks les modules sont supprimés dans le sens inverse de la création
*/
void CApplication::deletePluginModules()
{
    for (int i=m_list_plugin_modules.size()-1; i>=0; i--) {
        if (m_list_plugin_modules[i]) {
            delete m_list_plugin_modules[i];
            m_list_plugin_modules[i] = NULL;
        }
    }
    m_list_plugin_modules.clear();
}


// _____________________________________________________________________
/*!
*  Lance l'application
*
*/
void CApplication::run(void)
{
  initBasicModules();
  initPluginModules();


   // Pour essayer
   m_data_center->write("PosX_robot", QVariant(33.456));
   m_data_center->write("PosY_robot", QVariant(123.789));
   m_data_center->write("PosTeta_robot", QVariant(789.124));

   //qDebug() << m_data_center->getDataValues();

   /*for (int i=0; i<m_list_basic_modules.size(); i++) {
       qDebug() << m_list_basic_modules[i]->getName() << m_list_basic_modules[i]->getVersion();
   }*/

   /*if(m_Ecran->m_isMaximized==1)
       m_Ecran->getIHM()->showMaximized();*/
}



// _____________________________________________________________________
/*!
*  Fin propre de l'application
*
*/
void CApplication::TermineApplication(void)
{
  // commence par informer tous les modules de la fin iminente de l'application
  closePluginModules();
  closeBasicModules();

  // efface tous les modules
  deletePluginModules();
  deleteBasicModules();
  m_list_modules.clear();

  // met fin à l'application
  qApp->exit();
}

/*! @} */
