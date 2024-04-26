/*! \file CLidar.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QFileDialog>
#include "CLidar.h"
#include "lidar_data_filter_factory.h"
#include "lidar_filter_params.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CMessagerieBot.h"
#include "CTrameFactory.h"



/*! \addtogroup Module_LIDAR
   *
   *  @{
   */

// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CLidar::CLidar(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_Lidar, AUTEUR_Lidar, INFO_Lidar),
      m_lidar_data_filter(Q_NULLPTR),
      m_lidar_filter_params(Q_NULLPTR),
      m_polar_graph(Q_NULLPTR),
      m_angular_axis(Q_NULLPTR)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CLidar::~CLidar()
{
    on_filter_params_close();
}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CLidar::init(CApplication *application)
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

    QString sick_ip = m_application->m_eeprom->read(getName(), "sick_ip", "192.168.0.1").toString();
    m_ihm.ui.tim5xx_ip->setText(sick_ip);

    int sick_port = m_application->m_eeprom->read(getName(), "sick_port", 2112).toInt();
    m_ihm.ui.tim5xx_port->setValue(sick_port);

    bool active_graph = m_application->m_eeprom->read(getName(), "active_graph", true).toBool();
    m_ihm.ui.cb_active_graph->setChecked(active_graph);

    int lidar_sample_period = m_application->m_eeprom->read(getName(), "lidar_sample_period", 100).toInt();
    m_ihm.ui.lidar_sample_period->setValue(lidar_sample_period);

    int logger_refresh_period = m_application->m_eeprom->read(getName(), "logger_refresh_period", 0).toInt();
    m_ihm.ui.logger_sample_period->setValue(logger_refresh_period);

    int graph_type = m_application->m_eeprom->read(getName(), "graph_type", GRAPH_TYPE_POLAR).toInt();
    m_ihm.ui.type_affichage_graph->setCurrentIndex(graph_type);
    on_change_graph_type(m_ihm.ui.type_affichage_graph->currentIndex());  // met en cohérence le type de grapha affiché avec le choix de la liste

    int data_diplayed = m_application->m_eeprom->read(getName(), "data_diplayed", GRAPH_RAW_DATA).toInt();
    m_ihm.ui.data_affichees->setCurrentIndex(data_diplayed);
    on_change_data_displayed(m_ihm.ui.data_affichees->currentIndex());

    init_data_filter(); // ajoute à la liste déroulante tous les filtres existants
    int index_data_filter = m_application->m_eeprom->read(getName(), "index_data_filter", 0).toInt();
    m_ihm.ui.filter_data_choice->setCurrentIndex(index_data_filter);
    on_change_data_filter(m_ihm.ui.filter_data_choice->currentText());

    bool enable_update_datamanager = m_application->m_eeprom->read(getName(), "enable_update_datamanager", 1).toBool();
    m_ihm.ui.enable_datamanager_update->setChecked(enable_update_datamanager);

    connect(&m_read_timer, SIGNAL(timeout()), this, SLOT(read_sick()));
    connect(&m_lidar, SIGNAL(connected()), this, SLOT(lidar_connected()));
    connect(&m_lidar, SIGNAL(disconnected()), this, SLOT(lidar_disconnected()));
    connect(m_ihm.ui.lidar_sample_period, SIGNAL(valueChanged(int)), this, SLOT(on_change_read_period(int)));
    connect(m_ihm.ui.zoom_distance, SIGNAL(valueChanged(int)), this, SLOT(on_change_zoom_distance(int)));
    connect(m_ihm.ui.type_affichage_graph, SIGNAL(currentIndexChanged(int)), this, SLOT(on_change_graph_type(int)));
    connect(m_ihm.ui.filter_data_choice, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_change_data_filter(QString)));
    connect(m_ihm.ui.filter_param, SIGNAL(clicked(bool)), this, SLOT(on_filter_params_show()));
    connect(m_ihm.ui.data_affichees, SIGNAL(currentIndexChanged(int)), this, SLOT(on_change_data_displayed(int)));

    // Sick TIM561
    connect(m_ihm.ui.tim5xx_openport, SIGNAL(clicked(bool)), this, SLOT(open_sick()));
    connect(m_ihm.ui.tim5xx_closeport, SIGNAL(clicked(bool)), this, SLOT(close_sick()));

    // Logger
    connect(m_ihm.ui.PB_logger_select_file, SIGNAL(clicked(bool)), this, SLOT(logger_select_file()));
    connect(m_ihm.ui.PB_start_logger, SIGNAL(clicked(bool)), this, SLOT(logger_start()));
    connect(m_ihm.ui.PB_stop_logger, SIGNAL(clicked(bool)), this, SLOT(logger_stop()));

    // Simulateur / rejoueur de trace
    connect(m_ihm.ui.PB_choixTrace, SIGNAL(clicked()), this, SLOT(on_PB_player_choix_trace_clicked()));
    connect(m_ihm.ui.dureeStepReplay, SIGNAL(editingFinished()), this, SLOT(on_PlayerStepDurationChanged()));
    connect(&m_data_player, SIGNAL(new_data(CLidarData)), this, SLOT(new_data(CLidarData)));
    connect(&m_data_player, SIGNAL(played(int)), this, SLOT(on_PlayedStep(int)));

    int sick_autoreconnect = m_application->m_eeprom->read(getName(), "sick_autoreconnect", false).toBool();
    m_ihm.ui.tim5xx_enable_autoreconnect->setChecked(sick_autoreconnect);
    open_sick();

    logger_stop();
}

// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CLidar::close(void)
{
    // Mémorise en EEPROM l'état de la fenêtre
    m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
    m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
    m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
    m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
    m_application->m_eeprom->write(getName(), "sick_ip", m_ihm.ui.tim5xx_ip->text());
    m_application->m_eeprom->write(getName(), "sick_port", m_ihm.ui.tim5xx_port->value());
    m_application->m_eeprom->write(getName(), "sick_autoreconnect", m_ihm.ui.tim5xx_enable_autoreconnect->isChecked());
    m_application->m_eeprom->write(getName(), "active_graph",  m_ihm.ui.cb_active_graph->isChecked());
    m_application->m_eeprom->write(getName(), "logger_refresh_period",  m_ihm.ui.logger_sample_period->value());
    m_application->m_eeprom->write(getName(), "lidar_sample_period",  m_ihm.ui.lidar_sample_period->value());
    m_application->m_eeprom->write(getName(), "graph_type",  m_ihm.ui.type_affichage_graph->currentIndex());
    m_application->m_eeprom->write(getName(), "data_diplayed",  m_ihm.ui.data_affichees->currentIndex());
    m_application->m_eeprom->write(getName(), "index_data_filter", m_ihm.ui.filter_data_choice->currentIndex());
    m_application->m_eeprom->write(getName(), "enable_update_datamanager", m_ihm.ui.enable_datamanager_update->isChecked());
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CLidar::onRightClicGUI(QPoint pos)
{
    QMenu *menu = new QMenu();

    menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
    menu->exec(m_ihm.mapToGlobal(pos));
}


// ______________________________________________
/*!
 * \brief Ouvre le port de communication avec le SICK
 */
void CLidar::open_sick()
{
    const int sick_protocol = -1;  // -1 = Automatic
    bool status = m_lidar.open(m_ihm.ui.tim5xx_ip->text(), m_ihm.ui.tim5xx_port->value(), sick_protocol, m_ihm.ui.tim5xx_enable_autoreconnect->isChecked());
    if (!status) {
        m_ihm.ui.tim5xx_closeport->setEnabled(false);
        m_ihm.statusBar()->showMessage("Failed to connect to Lidar", 3000);
        if (m_ihm.ui.enable_datamanager_update->isChecked()) status_to_datamanager(LidarUtils::LIDAR_ERROR);
    }
}

// ______________________________________________
/*!
 * \brief Ferme le port de communication avec le SICK
 */
void CLidar::close_sick()
{
    m_lidar.close();
}

// _____________________________________________________________________
/*!
 *
 */
void CLidar::lidar_connected()
{
    qDebug() << "LIDAR CONNECTED";
    m_ihm.statusBar()->showMessage("Connected to Lidar", 3000);
    m_ihm.ui.tim5xx_openport->setEnabled(false);
    m_ihm.ui.tim5xx_closeport->setEnabled(true);
    m_ihm.ui.tabWidget->setCurrentIndex(0);  // positionne automatiquement sur la page graphique
    m_read_timer.start(m_ihm.ui.lidar_sample_period->value());
}

// _____________________________________________________________________
/*!
 *
 */
void CLidar::lidar_disconnected()
{
    m_read_timer.stop();
    qDebug() << "!!! LIDAR DISCONNECTED !!!";
    m_ihm.statusBar()->showMessage("Lidar Disconnected", 3000);
    m_ihm.ui.tim5xx_openport->setEnabled(true);
    m_ihm.ui.tim5xx_closeport->setEnabled(false);

    LidarUtils::tLidarObstacles dummy_obstacles;
    int status = LidarUtils::LIDAR_DISCONNECTED;
    send_ETAT_LIDAR(dummy_obstacles, status);
    if (m_ihm.ui.enable_datamanager_update->isChecked()) status_to_datamanager(status);
}

// _____________________________________________________________________
/*!
 * \brief CLidar::read_sick
 */
void CLidar::read_sick()
{
    CLidarData data;
    bool status = m_lidar.poll_one_telegram(&data);
    if (status) new_data(data);
    else {
        LidarUtils::tLidarObstacles dummy_obstacles;
        send_ETAT_LIDAR(dummy_obstacles, LidarUtils::LIDAR_ERROR);
        if (m_ihm.ui.enable_datamanager_update->isChecked()) status_to_datamanager(status);
    }
    /*
    if (status) {
        qDebug() << "Timestamp" << data.m_timestamp;
        qDebug() << "Nbre de mesures" << data.m_measures_count;
        qDebug() << "Start angle" << data.m_start_angle;
        qDebug() << "angle_step_resolution" << data.m_angle_step_resolution;
        QVector<double> x(data.m_measures_count), y(data.m_measures_count);
        for (int j=0; j<data.m_measures_count; ++j)
        {
            x[j] = data.m_start_angle + j * data.m_angle_step_resolution;
            y[j] = data.m_dist_measures[j];
            qDebug() << "Angle:" << x[j] << "=>" << y[j];
        }
    }
*/
}

// _____________________________________________________________
// Une nouvelle donnée est arrivée (en provenant du LIDAR ou du rejoueur)
void CLidar::new_data(const CLidarData &data)
{
    // filtre les données
    CLidarData filtered_data = data;
    if (m_lidar_data_filter) m_lidar_data_filter->filter(&data, &filtered_data);

    if (m_ihm.ui.cb_active_graph->isChecked()) {
        if (m_ihm.ui.data_affichees->currentIndex()==1) { // affiche les données filtrées
            refresh_graph(filtered_data);
        }
        else {  // affiche les données brutes
            refresh_graph(data);
        }
    }

    if (m_logger_active) log_data(data);

    // Met en forme la structure obstacles a partir des donnees filtrees
    // la recopie dans la structure obstacle a la capacite de dire qu'il y a un probleme (trop de points detectes par ex)
    // Envoie les infos vers le MBED
    LidarUtils::tLidarObstacles obstacles;
    int status = lidar_data_to_obstacles(filtered_data, obstacles); // TODO: cette fonction reste a completer (pour le moment, il y a juste un exemple)

    send_ETAT_LIDAR(obstacles, status);
    if (m_ihm.ui.enable_datamanager_update->isChecked())  {
        obstacles_to_datamanager(obstacles);
        status_to_datamanager(status);
    }
}

// _____________________________________________________________
void CLidar::on_change_read_period(int period)
{
    m_read_timer.setInterval(period);
}

// _____________________________________________________________
void CLidar::on_change_zoom_distance(int zoom_mm)
{
    if (m_angular_axis) m_angular_axis->radialAxis()->setRange(0, zoom_mm);
    else                m_ihm.ui.customPlot->yAxis->setRange(0, zoom_mm);
    m_ihm.ui.customPlot->replot();
}

// _____________________________________________________________
void CLidar::on_change_graph_type(int choice)
{
    if (choice==GRAPH_TYPE_POLAR)   init_polar_qcustomplot();
    else                            init_linear_qcustomplot();
    on_change_zoom_distance(m_ihm.ui.zoom_distance->value()); // met à jour l'echelle
}

// _____________________________________________________________
void CLidar::on_change_data_displayed(int choice)
{
    bool enabled = choice==GRAPH_FILTERED_DATA;
    m_ihm.ui.filter_data_choice->setEnabled(enabled);
    m_ihm.ui.filter_param->setEnabled(enabled);
    on_filter_params_close(); // ferme la fenetre de reglage des parametres du filtre (si elle etait ouverte)
}

// _____________________________________________________________
void CLidar::init_polar_qcustomplot()
{
    delete_current_graph();
    m_ihm.ui.customPlot->plotLayout()->clear();

      m_angular_axis = new QCPPolarAxisAngular(m_ihm.ui.customPlot);
      m_ihm.ui.customPlot->plotLayout()->addElement(0, 0, m_angular_axis);

      m_ihm.ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
      m_angular_axis->setRangeDrag(false);
      m_angular_axis->setTickLabelMode(QCPPolarAxisAngular::lmUpright);

      m_angular_axis->radialAxis()->setTickLabelMode(QCPPolarAxisRadial::lmUpright);
      m_angular_axis->radialAxis()->setTickLabelRotation(0);
      m_angular_axis->radialAxis()->setAngle(-45);

      m_angular_axis->grid()->setAngularPen(QPen(QColor(200, 200, 200), 0, Qt::SolidLine));
      m_angular_axis->grid()->setSubGridType(QCPPolarGrid::gtAll);

      m_angular_axis->setRange(-180, 180);
      m_angular_axis->setAngle(180);
      m_angular_axis->setRangeReversed(true);
      m_angular_axis->radialAxis()->setRange(0, 4000);  // échelle de valeur

      m_polar_graph = new QCPPolarGraph(m_angular_axis, m_angular_axis->radialAxis());
      m_polar_graph->setScatterStyle(QCPScatterStyle::ssDisc);
      m_polar_graph->setLineStyle(QCPPolarGraph::lsNone); // Pas de ligne tracée entre les points
}

// _____________________________________________________________________
void CLidar::init_linear_qcustomplot()
{
    //m_ihm.ui.customPlot->plotLayout()->clear();
    delete_current_graph();
    // create graph and assign data to it:
    m_ihm.ui.customPlot->addGraph();
    // give the axes some labels:
    m_ihm.ui.customPlot->xAxis->setLabel("angle [deg]");
    m_ihm.ui.customPlot->yAxis->setLabel("distance [mm]");
    // set axes ranges, so we see all data:
    m_ihm.ui.customPlot->xAxis->setRange(-180, 180);
    m_ihm.ui.customPlot->yAxis->setRange(0, 4000);
    m_ihm.ui.customPlot->graph()->setScatterStyle(QCPScatterStyle::ssDisc);
    m_ihm.ui.customPlot->graph()->setLineStyle(QCPGraph::lsNone);
}

// _____________________________________________________________________
void CLidar::delete_current_graph()
{
    if (m_polar_graph) {
        delete m_polar_graph;
        m_polar_graph = Q_NULLPTR;
    }
    if (m_angular_axis) {
        delete m_angular_axis;
        m_angular_axis = Q_NULLPTR;
    }

    if (m_ihm.ui.customPlot) delete m_ihm.ui.customPlot;
    m_ihm.ui.customPlot = new QCustomPlot(m_ihm.ui.tab);
    m_ihm.ui.customPlot->setObjectName(QStringLiteral("customPlot"));
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(m_ihm.ui.customPlot->sizePolicy().hasHeightForWidth());
    m_ihm.ui.customPlot->setSizePolicy(sizePolicy);
    m_ihm.ui.gridLayout->addWidget(m_ihm.ui.customPlot, 2, 0, 3, 5);
}

// _____________________________________________________________________
// Initialise la liste déroulante des filtres avec la liste existante
void CLidar::init_data_filter()
{
    QStringList filter_lst = CLidarDataFilterFactory::getExisting();
    m_ihm.ui.filter_data_choice->addItems(filter_lst);
}

// _____________________________________________________________________
void CLidar::refresh_graph(const CLidarData &data)
{
    if (m_polar_graph)  refresh_polar_graph(data);
    else                refresh_linear_graph(data);
}

// _____________________________________________________________________
// TODO ; voir pour utiliser setData sur des vecteurs X, Y deja tous faits et
// utiliser le 3eme parametre de setData a true pour indiquer que les donnees sont deja triees
void CLidar::refresh_polar_graph(const CLidarData &data)
{
    m_polar_graph->data().data()->clear();
    for (int i=0; i<data.m_measures_count; i++) {
        int angle = data.m_start_angle + i * data.m_angle_step_resolution;
        if (data.m_dist_measures[i] < m_ihm.ui.filtrage_distance->value()) m_polar_graph->addData(angle, data.m_dist_measures[i]);

    }
    m_ihm.ui.customPlot->replot();
}

// _____________________________________________________________________
void CLidar::refresh_linear_graph(const CLidarData &data)
{
    m_ihm.ui.customPlot->graph(0)->data().clear();
    QVector<double> x, y;
    for (int i=0; i<data.m_measures_count; i++) {
        double angle = data.m_start_angle + i * data.m_angle_step_resolution;
        double distance = data.m_dist_measures[i];
        if (distance < m_ihm.ui.filtrage_distance->value()) {
            x.append(angle);
            y.append(distance);
        }
    }
    // TODO : utiliser le 3eme parametre de setData a true pour indiquer que les donnees sont deja triees
    m_ihm.ui.customPlot->graph(0)->setData(x, y);
    m_ihm.ui.customPlot->replot();
}


// _____________________________________________________________________
void CLidar::on_change_data_filter(QString filter_name)
{
    if (m_lidar_data_filter) delete m_lidar_data_filter;
    m_lidar_data_filter = CLidarDataFilterFactory::createInstance(filter_name);
    bool visible = m_lidar_filter_params && m_lidar_filter_params->isVisible(); // memorise si la fenetre de reglage des parametres etait ouverte pour la rouvrir apres changement de filtre
    filter_params_load();
    if(visible) on_filter_params_show();  // si la fenetre des parametres du filtre etait affichee, il faut l'afficher pour le nouveau filtre selectionne
}

// _____________________________________________________________________
// les parametres des differents filtres sont memorises dans un fichier eeprom separe et commun a tous les filtres (1 section par filtre dans le fichier)
// le nom du fichier eeprom specifique aux parametres des filtres lidar doit etre passe au filtre qui n'a pas connaissance de l'application complete
void CLidar::on_filter_params_show()
{
    filter_params_load();
    m_lidar_filter_params->show();
}

// _____________________________________________________________________
void CLidar::filter_params_load()
{
    // soit le nom complet du fichier eeprom pour les parametres des filtres est donne dans le fichier de config general, soit il est recalcule
    QString eeprom_lidar_filter_params_file = m_application->m_eeprom->read(getName(), "lidar_params_eeprom_filename", "").toString();
    if (eeprom_lidar_filter_params_file.simplified() == "") {
        eeprom_lidar_filter_params_file = QString("%1/lidar_filter_params.ini").arg(m_application->m_pathname_config_file);
    }

    if(m_lidar_filter_params) delete m_lidar_filter_params;
    m_lidar_filter_params = new CLidarFilterParams(m_lidar_data_filter, eeprom_lidar_filter_params_file);
    connect(m_lidar_filter_params, SIGNAL(closed()), this, SLOT(on_filter_params_close()));
}

// _____________________________________________________________________
void CLidar::on_filter_params_close()
{
    if(m_lidar_filter_params) {
        delete m_lidar_filter_params;
        m_lidar_filter_params = Q_NULLPTR;
    }
}

// ==============================================================
//                          LOGGER
// ==============================================================
//
// _____________________________________________________________________
void CLidar::logger_start(QString pathfilename)
{
    // Ouvre et construit le fichier
    m_logger_file.setFileName(pathfilename);
    if (!m_logger_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }

    // Grise ou dégrise les éléments de l'onglet Logger
    m_ihm.ui.PB_start_logger->setEnabled(false);
    m_ihm.ui.logger_sample_period->setEnabled(false);
    m_ihm.ui.logger_pathfilename->setEnabled(false);
    m_ihm.ui.PB_logger_select_file->setEnabled(false);
    m_ihm.ui.logger_sample_period_label->setEnabled(false);
    m_ihm.ui.PB_stop_logger->setEnabled(true);

    m_logger_active = true;
    m_first_log = true;
}


// _____________________________________________________________________
void CLidar::logger_start()
{
    if (m_ihm.ui.logger_pathfilename->text().isEmpty())         logger_select_file();
    if (QFile(m_ihm.ui.logger_pathfilename->text()).exists())   logger_select_file();
    if (m_ihm.ui.logger_pathfilename->text().isEmpty()) return; // pas la peine d'aller plus loin si pas de fichier renseigné

    logger_start(m_ihm.ui.logger_pathfilename->text());
}


// _____________________________________________________________________
void CLidar::logger_stop()
{
    m_logger_active = false;
    if (!m_logger_file.isOpen()) m_logger_file.close();

    // Grise ou dégrise les éléments de l'onglet Logger
    m_ihm.ui.PB_start_logger->setEnabled(true);
    //m_ihm.ui.logger_sample_period->setEnabled(true);
    m_ihm.ui.logger_pathfilename->setEnabled(true);
    m_ihm.ui.PB_logger_select_file->setEnabled(true);
    //m_ihm.ui.logger_sample_period_label->setEnabled(true);
    m_ihm.ui.PB_stop_logger->setEnabled(false);
}


// _____________________________________________________________________
void CLidar::logger_select_file()
{
    QString fileName = QFileDialog::getSaveFileName(&m_ihm,
        tr("Choix du fichier de sortie"), "", tr("CSV Files (*.csv)"));

    m_ihm.ui.logger_pathfilename->setText(fileName);

}

// _____________________________________________________________________
// Enregistre les données dans le fichier de sortie
// On suppose que le la configuration du Lidar n'a pas changé dans une session de log
//      - Angle de début
//      - Résolution
void CLidar::log_data(const CLidarData &data)
{
    if (!m_logger_file.isOpen()) return;

    // Ecrit l'entête du fichier
    QTextStream out(&m_logger_file);

    // Gestion de l'entête du fichier csv sur la première réception de données
    if (m_first_log) {
        m_first_log = false;
        out << "timestamp [usec]";
        for (int i=0; i<data.m_measures_count; i++) {
            double angle = data.m_start_angle + data.m_angle_step_resolution * i;
            out << CSV_SEPARATOR << QString("Dist %1 deg [mm]").arg(angle);
        }
        out << endl;
    }

    // Les données
    out << data.m_timestamp;
    for (int i=0; i<data.m_measures_count; i++) {
        double measure = data.m_dist_measures[i];
        out << CSV_SEPARATOR << QString("%1").arg(measure);
    }
    out << endl;

    qDebug() << "Réception data lidar" << data.m_measures_count;
}

// _____________________________________________________________________
/*!
 * \brief Envoie des donnees mises en forme du Lidar vers le MBED
 * \param data les donnees filtrees
 */
void CLidar::send_ETAT_LIDAR(LidarUtils::tLidarObstacles obstacles, unsigned char lidar_status)
{
    CTrameBot *trame = m_application->m_MessagerieBot->getTrameFactory()->getTrameFromID(ID_ETAT_LIDAR);
    CTrame_ETAT_LIDAR *trame_lidar = (CTrame_ETAT_LIDAR *)trame;
    trame_lidar->m_status = lidar_status;
    LidarUtils::copy_tab_obstacles(obstacles, trame_lidar->m_obstacles);
    trame_lidar->Encode();
}

// _____________________________________________________________________
/*!
 * \brief Met en forme la structure de donnes d'obstacles a partir des donnees LIDAR
 * \param data les donnees filtrees
 * \return la structure de donnees d'obstacle
*  \return le statut du LIDAR
 */
int CLidar::lidar_data_to_obstacles(const CLidarData &in_data, LidarUtils::tLidarObstacles out_obstacles)
{
    for (int i=0; i<LidarUtils::NBRE_MAX_OBSTACLES; i++) {
        out_obstacles[i].angle = 0;
        out_obstacles[i].distance = LidarUtils::NO_OBSTACLE;
    }

    // juste pour l'exemple, renseigne une valeur pipo pour le premier obstacle (juste pour tester la communication)
    // TODO: faire le lien avec le post-traitement des donnees du LIDAR
    int index=45+60;
    int angle = in_data.m_start_angle + index*in_data.m_angle_step_resolution;
    int distance = in_data.m_dist_measures[index];
    out_obstacles[0].angle = angle;
    out_obstacles[0].distance = distance;

    return LidarUtils::LIDAR_OK; // pour le moment, toujours OK
}

// _____________________________________________________________________
/*!
 * \brief Publie les obstacles dans le DataManager
 * \param obstacles la structure contenant les obstales
 */
void CLidar::obstacles_to_datamanager(LidarUtils::tLidarObstacles obstacles)
{
    QString dataname;
    for (int i=0; i<LidarUtils::NBRE_MAX_OBSTACLES; i++) {
        dataname= QString("Lidar.Obstacle%1.Angle").arg(i+1);
        m_application->m_data_center->write(dataname, obstacles[i].angle);

        dataname= QString("Lidar.Obstacle%1.Distance").arg(i+1);
        m_application->m_data_center->write(dataname, obstacles[i].angle);
    }
}

// _____________________________________________________________________
/*!
 * \brief CLidar::status_to_datamanager
 * \param lidar_status
 */
void CLidar::status_to_datamanager(unsigned char lidar_status)
{
    QString dataname= QString("Lidar.Status");
    m_application->m_data_center->write(dataname, lidar_status);
}

// ================================================================================
//                      SIMULATEUR / REJOUEUR DE TRACE CSV
// ================================================================================

// _____________________________________________________________________
/*!
* Choix du fichier de trace à rejouer
*
*/
void CLidar::on_PB_player_choix_trace_clicked(void)
{
  QString fileName = QFileDialog::getOpenFileName(NULL,
      tr("Signal"), "./", tr("CSV Files (*.csv);;Signal Files (*.trc)"));

  QFileInfo file_info(fileName);
  QString trace_name = file_info.baseName();
  m_ihm.ui.trace_name->setText(trace_name);

  if (m_data_player.parse(fileName)) {
      m_ihm.ui.tabWidget->setCurrentIndex(0); // bascule sur l'onglet de visualisation des données (graph)
      m_ihm.ui.sliderStepNumTrace->setMaximum(m_data_player.get_step_count()-1);
      m_data_player.set_steps_duration(m_ihm.ui.dureeStepReplay->value());
      m_data_player.start();
  }
}

// _____________________________________________________________________
// Affiche le numero du step joue et le nombre total de step
void CLidar::on_PlayedStep(int step_num)
{
    m_ihm.ui.statusbar->showMessage(QString("Play step %1/%2").arg(step_num).arg(m_data_player.get_step_count()-1), 1000);
    m_ihm.ui.sliderStepNumTrace->setValue(step_num);
}

// _____________________________________________________________________
void CLidar::on_PlayerStepDurationChanged()
{
    m_data_player.set_steps_duration(m_ihm.ui.dureeStepReplay->value());
}
