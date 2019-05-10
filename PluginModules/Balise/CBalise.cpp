/*! \file CBalise.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
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

  connect(&m_timer_logger, SIGNAL(timeout()), this, SLOT(tick_Log()));

  m_data_list_logged << m_application->m_data_center->getData("TIMESTAMP_MATCH.Timestamp")
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
    startLogger();

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
  stopLogger();

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
void CBalise::startLogger(QString pathfilename)
{
    if (pathfilename == "") {
        pathfilename =  getLogFilename();
    }
    // Ouvre et construit le fichier
    m_logger_file.setFileName(pathfilename);
    if (!m_logger_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_application->m_print_view->print_error(this, "Impossible d'ouvrir en écriture le fichier " + pathfilename);
        return;
    }

    // Ecrit l'entête du fichier
    QTextStream out(&m_logger_file);
    // 1ere ligne du fichier = nom des colonnes
    out << "DateTime;";
    for (int i=0; i<m_data_list_logged.size(); i++)
    {
        out << m_data_list_logged.at(i)->getName();
        if (i != (m_data_list_logged.size()-1))
            out << ";";
    }
    out << "\n";

    m_application->m_print_view->print_debug(this, "Start logger : " + pathfilename);
    m_elapsed_timer.restart();
    m_timer_logger.start(LOGGER_PERIOD);
}

// _____________________________________________________________________
void CBalise::stopLogger()
{
    m_timer_logger.stop();
    m_logger_file.close();
}

// _____________________________________________________________________
QString CBalise::getLogFilename()
{
    QString pathfilename;

    for (int i=1; i<10000; i++) {
        pathfilename =  m_application->m_pathname_log_file +
                        "/" +
                        QString(getName()).replace(" ", "") + // Supprime les espaces dans le nom généré
                        BASE_LOGGER_FILENAME +
                        QString::number(i) +
                        ".csv";
        QFileInfo fileinfo(pathfilename);
        if (!fileinfo.exists()) break;
    }
    return pathfilename;
}

// _____________________________________________________________________
void CBalise::tick_Log()
{
    QTextStream out(&m_logger_file);
    out << m_elapsed_timer.elapsed();
    out << ";";
    for (int i=0; i<m_data_list_logged.size(); i++)
    {
        out << m_data_list_logged.at(i)->read().toFloat();
        if (i != (m_data_list_logged.size()-1))
            out << ";";
    }
    out << "\n";
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
