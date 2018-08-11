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

#include "CMainWindow.h"
#include "CDataManager.h"
#include "CPrintView.h"
#include "CEEPROM.h"
#include "CDataView.h"
#include "CDataGraph.h"
#include "CDataPlayer.h"
#include "CRS232.h"
#include "CJoystick.h"
#include "CModuleDesigner.h"
#include "CUserGuides.h"
#include "CExternalControler.h"
//_##NEW_INCLUDE_BASIC_MODULE_HERE_##
#ifdef RASPBERRY_PI
    #include "CRaspiGPIO.h"
#endif

#include "CTestUnitaire.h"
#include "CSimuBot.h"
#include "CStrategyDesigner.h"
#include "CMessagerieBot.h"
#include "CSensorElectroBot.h"
#include "CActuatorElectrobot.h"
#include "CSensorView.h"
#include "CAsserv.h"
#include "CActuatorSequencer.h"
#include "CBotCam.h"
#include "CEcran.h"
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
  m_print_view    = new CPrintView("PrintView");
  m_list_basic_modules.append(m_print_view);
  m_list_modules.append(m_print_view);

  m_eeprom    = new CEEPROM("EEPROM");
  m_list_basic_modules.append(m_eeprom);
  m_list_modules.append(m_eeprom);

  m_main_windows    = new CMainWindow("MainWindow");
  m_list_basic_modules.append(m_main_windows);
  m_list_modules.append(m_main_windows);

  m_data_center     = new CDataManager("DataCenter");
  m_list_basic_modules.append(m_data_center);
  m_list_modules.append(m_data_center);

  m_DataView     = new CDataView("DataView");
  m_list_basic_modules.append(m_DataView);
  m_list_modules.append(m_DataView);

  m_DataGraph     = new CDataGraph("DataGraph");
  m_list_basic_modules.append(m_DataGraph);
  m_list_modules.append(m_DataGraph);

  m_DataPlayer     = new CDataPlayer("DataPlayer");
  m_list_basic_modules.append(m_DataPlayer);
  m_list_modules.append(m_DataPlayer);

  m_RS232_robot   = new CRS232("RS232_robot");
  m_list_basic_modules.append(m_RS232_robot);
  m_list_modules.append(m_RS232_robot);

  m_RS232_cmucam   = new CRS232("RS232_cmucam");
  m_list_basic_modules.append(m_RS232_cmucam);
  m_list_modules.append(m_RS232_cmucam);

  m_Joystick     = new CJoystick("Joystick");
  m_list_basic_modules.append(m_Joystick);
  m_list_modules.append(m_Joystick);

  m_UserGuides     = new CUserGuides("UserGuides");
  m_list_basic_modules.append(m_UserGuides);
  m_list_modules.append(m_UserGuides);

  m_module_creator    = new CModuleDesigner("ModuleDesigner");
  m_list_basic_modules.append(m_module_creator);
  m_list_modules.append(m_module_creator);

  m_ExternalControler     = new CExternalControler("ExternalControler");
  m_list_basic_modules.append(m_ExternalControler);
  m_list_modules.append(m_ExternalControler);

#ifdef RASPBERRY_PI
  m_RaspiGPIO     = new CRaspiGPIO("RaspiGPIO");
  m_list_basic_modules.append(m_RaspiGPIO);
  m_list_modules.append(m_RaspiGPIO);
#endif

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
  m_TestUnitaire     = new CTestUnitaire("TestUnitaire");
  m_list_plugin_modules.append(m_TestUnitaire);
  m_list_modules.append(m_TestUnitaire);

  m_SimuBot     = new CSimuBot("SimuBot");
  m_list_plugin_modules.append(m_SimuBot);
  m_list_modules.append(m_SimuBot);

  m_StrategyDesigner     = new CStrategyDesigner("StrategyDesigner");
  m_list_plugin_modules.append(m_StrategyDesigner);
  m_list_modules.append(m_StrategyDesigner);

  m_MessagerieBot     = new CMessagerieBot("MessagerieBot");
  m_list_plugin_modules.append(m_MessagerieBot);
  m_list_modules.append(m_MessagerieBot);

  m_SensorElectroBot     = new CSensorElectroBot("SensorElectroBot");
  m_list_plugin_modules.append(m_SensorElectroBot);
  m_list_modules.append(m_SensorElectroBot);

  m_ActuatorElectrobot     = new CActuatorElectrobot("ActuatorElectrobot");
  m_list_plugin_modules.append(m_ActuatorElectrobot);
  m_list_modules.append(m_ActuatorElectrobot);

  m_SensorView     = new CSensorView("SensorView");
  m_list_plugin_modules.append(m_SensorView);
  m_list_modules.append(m_SensorView);

  m_Asserv     = new CAsserv("Asserv");
  m_list_plugin_modules.append(m_Asserv);
  m_list_modules.append(m_Asserv);

  m_ActuatorSequencer     = new CActuatorSequencer("ActuatorSequencer");
  m_list_plugin_modules.append(m_ActuatorSequencer);
  m_list_modules.append(m_ActuatorSequencer);

  m_BotCam     = new CBotCam("BotCam");
  m_list_plugin_modules.append(m_BotCam);
  m_list_modules.append(m_BotCam);

  /*
  m_Ecran     = new CEcran("Ecran");
  m_list_plugin_modules.append(m_Ecran);
  m_list_modules.append(m_Ecran);
*/
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
