/*! \file CBalise.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include "CBalise.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CToolBox.h"
#include "CXbeeNetworkMessenger.h"
#include "xbeedriverbase.h"

const QString CBalise::BASE_LOGGER_FILENAME = "balise_log_";

/*! \addtogroup Module_BALISE
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CBalise::CBalise(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_Balise, AUTEUR_Balise, INFO_Balise),
      m_xbee_ntw_database(NULL)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CBalise::~CBalise()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CBalise::init(CApplication *application)
{
  CModule::init(application);
  setGUI(&m_ihm); // indique à la classe de base l'IHM

  // Gère les actions sur clic droit sur le panel graphique du module
  m_ihm.setContextMenuPolicy(Qt::CustomContextMenu);
  connect(&m_ihm, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRightClicGUI(QPoint)));

  // Restore la taille de la fenêtre
  QVariant val;
  val = m_application->m_eeprom->read(getName(), "geometry", QRect(50, 50, 150, 150));
  m_ihm.setGeometry(val.toRect());
  // Restore le fait que la fenêtre est visible ou non
  val = m_application->m_eeprom->read(getName(), "visible", QVariant(true));
  if (val.toBool()) { m_ihm.show(); }
  else              { m_ihm.hide(); }
  // Restore le niveau d'affichage
  val = m_application->m_eeprom->read(getName(), "niveau_trace", QVariant(MSG_TOUS));
  setNiveauTrace(val.toUInt());
  // Restore la couleur de fond
  val = m_application->m_eeprom->read(getName(), "background_color", QVariant(DEFAULT_MODULE_COLOR));
  setBackgroundColor(val.value<QColor>());

  // Crée les data dans le data manager pour être certain qu'elles existent
  // Coordonnées X, Y, Teta pour les 4 robots détectés par le module de traitement d'image
  // + coordonnées X, Y, Teta remonté par Grosbot
  // + coordonnées X, Y, Teta remonté par Minibot
  m_application->m_data_center->write("Robot1_X",  -32000);
  m_application->m_data_center->write("Robot1_Y",  -32000);
  m_application->m_data_center->write("Robot1_Teta",  -32000);

  m_application->m_data_center->write("Robot2_X",  -32000);
  m_application->m_data_center->write("Robot2_Y",  -32000);
  m_application->m_data_center->write("Robot2_Teta",  -32000);

  m_application->m_data_center->write("Robot3_X",  -32000);
  m_application->m_data_center->write("Robot3_Y",  -32000);
  m_application->m_data_center->write("Robot3_Teta",  -32000);

  m_application->m_data_center->write("Robot4_X",  -32000);
  m_application->m_data_center->write("Robot4_Y",  -32000);
  m_application->m_data_center->write("Robot4_Teta",  -32000);

  m_application->m_data_center->write("GROSBOT_POSITION.Position_X",  -32000);
  m_application->m_data_center->write("GROSBOT_POSITION.Position_Y",  -32000);
  m_application->m_data_center->write("GROSBOT_POSITION.Angle",  -32000);

  m_application->m_data_center->write("ROBOTLEGO_STATUS.Position_X",  -32000);
  m_application->m_data_center->write("ROBOTLEGO_STATUS.Position_Y",  -32000);
  m_application->m_data_center->write("ROBOTLEGO_STATUS.Angle",  -32000);

  m_application->m_data_center->write("TIMESTAMP_MATCH.Timestamp", -1);
  m_application->m_data_center->write("XbeeMsngNodePresent.GROSBOT", 0);

  m_application->m_data_center->write("VideoActive", 0);

  connect(m_application->m_data_center->getData("Robot1_X"), SIGNAL(valueChanged(QVariant)), this, SLOT(Robot1_DetectedPosition_changed()));
  connect(m_application->m_data_center->getData("Robot1_Y"), SIGNAL(valueChanged(QVariant)), this, SLOT(Robot1_DetectedPosition_changed()));
  connect(m_application->m_data_center->getData("Robot1_Teta"), SIGNAL(valueChanged(QVariant)), this, SLOT(Robot1_DetectedPosition_changed()));
  connect(m_application->m_data_center->getData("Robot2_X"), SIGNAL(valueChanged(QVariant)), this, SLOT(Robot2_DetectedPosition_changed()));
  connect(m_application->m_data_center->getData("Robot2_Y"), SIGNAL(valueChanged(QVariant)), this, SLOT(Robot2_DetectedPosition_changed()));
  connect(m_application->m_data_center->getData("Robot2_Teta"), SIGNAL(valueChanged(QVariant)), this, SLOT(Robot2_DetectedPosition_changed()));
  connect(m_application->m_data_center->getData("Robot3_X"), SIGNAL(valueChanged(QVariant)), this, SLOT(Robot3_DetectedPosition_changed()));
  connect(m_application->m_data_center->getData("Robot3_Y"), SIGNAL(valueChanged(QVariant)), this, SLOT(Robot3_DetectedPosition_changed()));
  connect(m_application->m_data_center->getData("Robot3_Teta"), SIGNAL(valueChanged(QVariant)), this, SLOT(Robot3_DetectedPosition_changed()));
  connect(m_application->m_data_center->getData("Robot4_X"), SIGNAL(valueChanged(QVariant)), this, SLOT(Robot4_DetectedPosition_changed()));
  connect(m_application->m_data_center->getData("Robot4_Y"), SIGNAL(valueChanged(QVariant)), this, SLOT(Robot4_DetectedPosition_changed()));
  connect(m_application->m_data_center->getData("Robot4_Teta"), SIGNAL(valueChanged(QVariant)), this, SLOT(Robot4_DetectedPosition_changed()));
  connect(m_application->m_data_center->getData("GROSBOT_POSITION.Position_X"), SIGNAL(valueChanged(QVariant)), this, SLOT(Grosbot_Position_changed()));
  connect(m_application->m_data_center->getData("GROSBOT_POSITION.Position_Y"), SIGNAL(valueChanged(QVariant)), this, SLOT(Grosbot_Position_changed()));
  connect(m_application->m_data_center->getData("GROSBOT_POSITION.Angle"), SIGNAL(valueChanged(QVariant)), this, SLOT(Grosbot_Position_changed()));
  connect(m_application->m_data_center->getData("ROBOTLEGO_STATUS.Position_X"), SIGNAL(valueChanged(QVariant)), this, SLOT(Minibot_Position_changed()));
  connect(m_application->m_data_center->getData("ROBOTLEGO_STATUS.Position_Y"), SIGNAL(valueChanged(QVariant)), this, SLOT(Minibot_Position_changed()));
  connect(m_application->m_data_center->getData("ROBOTLEGO_STATUS.Angle"), SIGNAL(valueChanged(QVariant)), this, SLOT(Minibot_Position_changed()));

  connect(m_application->m_data_center->getData("XbeeMsngNodePresent.GROSBOT"), SIGNAL(valueChanged(QVariant)), this, SLOT(grosbot_present_changed(QVariant)));
  connect(m_application->m_data_center->getData("VideoActive"), SIGNAL(valueChanged(QVariant)), this, SLOT(video_state_changed(QVariant)));

  connect(m_ihm.ui.RPI_Reboot,SIGNAL(clicked(bool)),this,SLOT(onRPI_Reboot()));
  connect(m_ihm.ui.RPI_Shutdown,SIGNAL(clicked(bool)),this,SLOT(onRPI_Shutdown()));

  val = m_application->m_eeprom->read(getName(), "logger_active", QVariant(false));
  if (val.toBool()) initDataLogger();

    m_xbee_ntw_database = m_application->m_XbeeNetworkMessenger->getDatabase();
    if (m_xbee_ntw_database) {
        // Pour la coupe 2019, ce message est envoyé uniquement à l'outil diag
        m_xbee_ntw_database->m_BalisePositions.setDestinationAddress(m_xbee_ntw_database->m_node_diag_tool.getID());
        m_xbee_ntw_database->m_BalisePositions.setTransmitPeriod(1000);
    }

    // Mise en cohérence des données de l'IHM et messagerie avec le data manager
    Robot1_DetectedPosition_changed();
    Robot2_DetectedPosition_changed();
    Robot3_DetectedPosition_changed();
    Robot4_DetectedPosition_changed();
    Grosbot_Position_changed();
    Minibot_Position_changed();
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CBalise::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CBalise::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

// =====================================================================
//                      Gestion du logger
// =====================================================================
// _____________________________________________________________________
/*!
*  Initialise les données à logger
*
*/
void CBalise::initDataLogger()
{
    QVector <CData *> data_list;
    data_list  << m_application->m_data_center->getData("TIMESTAMP_MATCH.Timestamp")
               << m_application->m_data_center->getData("Robot1_X")
               << m_application->m_data_center->getData("Robot1_Y")
               << m_application->m_data_center->getData("Robot1_Teta")
               << m_application->m_data_center->getData("Robot2_X")
               << m_application->m_data_center->getData("Robot2_Y")
               << m_application->m_data_center->getData("Robot2_Teta")
               << m_application->m_data_center->getData("Robot3_X")
               << m_application->m_data_center->getData("Robot3_Y")
               << m_application->m_data_center->getData("Robot3_Teta")
               << m_application->m_data_center->getData("Robot4_X")
               << m_application->m_data_center->getData("Robot4_Y")
               << m_application->m_data_center->getData("Robot4_Teta")
               << m_application->m_data_center->getData("GROSBOT_POSITION.Position_X")
               << m_application->m_data_center->getData("GROSBOT_POSITION.Position_Y")
               << m_application->m_data_center->getData("GROSBOT_POSITION.Angle")
               << m_application->m_data_center->getData("ROBOTLEGO_STATUS.Position_X")
               << m_application->m_data_center->getData("ROBOTLEGO_STATUS.Position_Y")
               << m_application->m_data_center->getData("ROBOTLEGO_STATUS.Angle")
                  ;
    m_data_logger.setDataList(data_list);
    m_data_logger.activeDatation(true);
    m_data_logger.setRefreshPeriod(LOGGER_PERIOD);
    QString pathfilename = getLogFilename();
    m_data_logger.start(pathfilename);
    m_application->m_print_view->print_debug(this, "Start logger : " + pathfilename);
}

// _____________________________________________________________________
QString CBalise::getLogFilename()
{
    QString pathfilename;

    for (int i=1; i<10000; i++) {
        pathfilename =  m_application->m_pathname_log_file +
                        "/" +
                        BASE_LOGGER_FILENAME +
                        QString::number(i) +
                        ".csv";
        QFileInfo fileinfo(pathfilename);
        if (!fileinfo.exists()) break;
    }
    return pathfilename;
}

// =====================================================================
//      Données en provenance du module de traitement d'image
// =====================================================================
// _____________________________________________________________________
void CBalise::Robot1_DetectedPosition_changed()
{
    float posx = m_application->m_data_center->getData("Robot1_X")->read().toFloat();
    float posy = m_application->m_data_center->getData("Robot1_Y")->read().toFloat();
    float teta = m_application->m_data_center->getData("Robot1_Teta")->read().toFloat();

    m_ihm.ui.Robot1_PosX->setValue(posx);
    m_ihm.ui.Robot1_PosY->setValue(posy);
    m_ihm.ui.Robot1_Teta->setValue(teta);

    if (m_xbee_ntw_database) {
        m_xbee_ntw_database->m_BalisePositions.PosX_Grosbot = posx;
        m_xbee_ntw_database->m_BalisePositions.PosY_Grosbot = posy;
    }
}

// _____________________________________________________________________
void CBalise::Robot2_DetectedPosition_changed()
{
    float posx = m_application->m_data_center->getData("Robot2_X")->read().toFloat();
    float posy = m_application->m_data_center->getData("Robot2_Y")->read().toFloat();
    float teta = m_application->m_data_center->getData("Robot2_Teta")->read().toFloat();

    m_ihm.ui.Robot2_PosX->setValue(posx);
    m_ihm.ui.Robot2_PosY->setValue(posy);
    m_ihm.ui.Robot2_Teta->setValue(teta);

    if (m_xbee_ntw_database) {
        m_xbee_ntw_database->m_BalisePositions.PosX_Minibot = posx;
        m_xbee_ntw_database->m_BalisePositions.PosY_Minibot = posy;
    }
}

// _____________________________________________________________________
void CBalise::Robot3_DetectedPosition_changed()
{
    float posx = m_application->m_data_center->getData("Robot3_X")->read().toFloat();
    float posy = m_application->m_data_center->getData("Robot3_Y")->read().toFloat();
    float teta = m_application->m_data_center->getData("Robot3_Teta")->read().toFloat();

    m_ihm.ui.Robot3_PosX->setValue(posx);
    m_ihm.ui.Robot3_PosY->setValue(posy);
    m_ihm.ui.Robot3_Teta->setValue(teta);

    if (m_xbee_ntw_database) {
        m_xbee_ntw_database->m_BalisePositions.PosX_Adversaire1 = posx;
        m_xbee_ntw_database->m_BalisePositions.PosY_Adversaire1 = posy;
    }
}

// _____________________________________________________________________
void CBalise::Robot4_DetectedPosition_changed()
{
    float posx = m_application->m_data_center->getData("Robot4_X")->read().toFloat();
    float posy = m_application->m_data_center->getData("Robot4_Y")->read().toFloat();
    float teta = m_application->m_data_center->getData("Robot4_Teta")->read().toFloat();

    m_ihm.ui.Robot4_PosX->setValue(posx);
    m_ihm.ui.Robot4_PosY->setValue(posy);
    m_ihm.ui.Robot4_Teta->setValue(teta);

    if (m_xbee_ntw_database) {
        m_xbee_ntw_database->m_BalisePositions.PosX_Adversaire2 = posx;
        m_xbee_ntw_database->m_BalisePositions.PosY_Adversaire2 = posy;
    }
}

// =====================================================================
//      Données en provenance des robots
// =====================================================================
// _____________________________________________________________________
void CBalise::Grosbot_Position_changed()
{
    m_ihm.ui.Grosbot_PosX->setValue(m_application->m_data_center->getData("GROSBOT_POSITION.Position_X")->read().toFloat());
    m_ihm.ui.Grosbot_PosY->setValue(m_application->m_data_center->getData("GROSBOT_POSITION.Position_Y")->read().toFloat());
    m_ihm.ui.Grosbot_Teta->setValue(m_application->m_data_center->getData("GROSBOT_POSITION.Angle")->read().toFloat());
}

// _____________________________________________________________________
void CBalise::Minibot_Position_changed()
{
    m_ihm.ui.Minibot_PosX->setValue(m_application->m_data_center->getData("ROBOTLEGO_STATUS.Position_X")->read().toFloat());
    m_ihm.ui.Minibot_PosY->setValue(m_application->m_data_center->getData("ROBOTLEGO_STATUS.Position_Y")->read().toFloat());
    m_ihm.ui.Minibot_Teta->setValue(m_application->m_data_center->getData("ROBOTLEGO_STATUS.Angle")->read().toFloat());
}

// =====================================================================
//      Extinction Raspberry
// =====================================================================
// _____________________________________________________________________
void CBalise::onRPI_Shutdown()
{
    int ret = QMessageBox::warning(&m_ihm, tr("Warning"),
                                   tr("Power Off\n"
                                      "Are you sure ?"),
                                   QMessageBox::Ok | QMessageBox::Cancel);
    if (ret == QMessageBox::Ok) {
        QProcess *myProcess = new QProcess();
        myProcess->start("shutdown --poweroff now");
    }
}

// _____________________________________________________________________
void CBalise::onRPI_Reboot()
{
    int ret = QMessageBox::warning(&m_ihm, tr("Warning"),
                                   tr("Reboot\n"
                                      "Are you sure ?"),
                                   QMessageBox::Ok | QMessageBox::Cancel);
    if (ret == QMessageBox::Ok) {
        QProcess *myProcess = new QProcess();
        myProcess->start("shutdown --reboot now");
    }
}

// =====================================================================
//      Gestion des LEDs sur l'onglet "Général"
// =====================================================================
// _____________________________________________________________________
void CBalise::video_state_changed(QVariant val)
{
    m_ihm.ui.led_video_state->setValue(val.toBool());
}

// _____________________________________________________________________
void CBalise::grosbot_present_changed(QVariant val)
{
    m_ihm.ui.led_grosbot_present->setValue(val.toBool());
}
