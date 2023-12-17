/*! \file CLidar.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QFileDialog>
#include "CLidar.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"



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
    :CPluginModule(plugin_name, VERSION_Lidar, AUTEUR_Lidar, INFO_Lidar)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CLidar::~CLidar()
{

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

    connect(&m_read_timer, SIGNAL(timeout()), this, SLOT(read_sick()));
    connect(&m_lidar, SIGNAL(connected()), this, SLOT(lidar_connected()));
    connect(&m_lidar, SIGNAL(disconnected()), this, SLOT(lidar_disconnected()));
    //connect(&m_lidar, SIGNAL(newData(CLidarData)), this, SLOT(data_received(CLidarData)));
    connect(m_ihm.ui.lidar_sample_period, SIGNAL(valueChanged(int)), this, SLOT(on_changed_read_period(int)));
    connect(m_ihm.ui.zoom_distance, SIGNAL(valueChanged(int)), this, SLOT(on_changed_zoom_distance(int)));

    connect(m_ihm.ui.tim5xx_openport, SIGNAL(clicked(bool)), this, SLOT(open_sick()));
    connect(m_ihm.ui.tim5xx_closeport, SIGNAL(clicked(bool)), &m_lidar, SLOT(close()));

    // Logger
    connect(m_ihm.ui.PB_logger_select_file, SIGNAL(clicked(bool)), this, SLOT(logger_select_file()));
    connect(m_ihm.ui.PB_start_logger, SIGNAL(clicked(bool)), this, SLOT(logger_start()));
    connect(m_ihm.ui.PB_stop_logger, SIGNAL(clicked(bool)), this, SLOT(logger_stop()));

    // Simulateur / rejoueur de trace
    connect(m_ihm.ui.PB_choixTrace, SIGNAL(clicked()), this, SLOT(on_PB_choixTrace_clicked()));


    int sick_autoreconnect = m_application->m_eeprom->read(getName(), "sick_autoreconnect", false).toBool();
    m_ihm.ui.tim5xx_enable_autoreconnect->setChecked(sick_autoreconnect);
    open_sick();

    logger_stop();

    init_polar_qcustomplot();
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
    }
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
}

// _____________________________________________________________________
/*!
 * \brief CLidar::read_sick
 */
void CLidar::read_sick()
{
    CLidarData data;
    bool status = m_lidar.poll_one_telegram(&data);
    if (status) {
        if (m_ihm.ui.cb_active_graph->isChecked()) refresh_graph(data);
        if (m_logger_active) log_data(data);
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
void CLidar::on_changed_read_period(int period)
{
    m_read_timer.setInterval(period);
}

void CLidar::on_changed_zoom_distance(int zoom_mm)
{
    m_angular_axis->radialAxis()->setRange(0, zoom_mm);
}

// _____________________________________________________________
void CLidar::init_polar_qcustomplot()
{
     //Test avec nouvelle version QCustomPlot Graphe polaire
      // Warning: Polar plots are a still a tech preview
      m_ihm.ui.customPlot->plotLayout()->clear();
      m_angular_axis = new QCPPolarAxisAngular(m_ihm.ui.customPlot);
      m_ihm.ui.customPlot->plotLayout()->addElement(0, 0, m_angular_axis);
      /* This is how we could set the angular axis to show pi symbols instead of degree numbers:
      QSharedPointer<QCPAxisTickerPi> ticker(new QCPAxisTickerPi);
      ticker->setPiValue(180);
      ticker->setTickCount(8);
      polarAxis->setTicker(ticker);
      */

      m_ihm.ui.customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
      m_angular_axis->setRangeDrag(false);
      m_angular_axis->setTickLabelMode(QCPPolarAxisAngular::lmUpright);

      m_angular_axis->radialAxis()->setTickLabelMode(QCPPolarAxisRadial::lmUpright);
      m_angular_axis->radialAxis()->setTickLabelRotation(0);
      m_angular_axis->radialAxis()->setAngle(-135);

      m_angular_axis->grid()->setAngularPen(QPen(QColor(200, 200, 200), 0, Qt::SolidLine));
      m_angular_axis->grid()->setSubGridType(QCPPolarGrid::gtAll);

      m_angular_axis->setRange(-180, 180);
      m_angular_axis->setAngle(-135);
      m_angular_axis->setRangeReversed(true);
      m_angular_axis->radialAxis()->setRange(0, 3500);  // échelle de valeur

      m_polar_graph = new QCPPolarGraph(m_angular_axis, m_angular_axis->radialAxis());
      m_polar_graph->setScatterStyle(QCPScatterStyle::ssDisc);
      m_polar_graph->setLineStyle(QCPPolarGraph::lsNone); // Pas de ligne tracée entre les points
}

// _____________________________________________________________________
void CLidar::init_linear_qcustomplot()
{

}

// _____________________________________________________________________
void CLidar::refresh_graph(CLidarData &data)
{
    //qDebug() << "Réception data lidar" << data.m_measures_count;
    m_polar_graph->data().data()->clear();
    for (int i=0; i<data.m_measures_count; i++) {
        if (data.m_dist_measures[i] < m_ihm.ui.filtrage_distance->value()) m_polar_graph->addData(i, data.m_dist_measures[i]);
    }
    m_ihm.ui.customPlot->replot();
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
    m_ihm.ui.logger_sample_period->setEnabled(true);
    m_ihm.ui.logger_pathfilename->setEnabled(true);
    m_ihm.ui.PB_logger_select_file->setEnabled(true);
    m_ihm.ui.logger_sample_period_label->setEnabled(true);
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
void CLidar::log_data(CLidarData &data)
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



// ================================================================================
//                      SIMULATEUR / REJOUEUR DE TRACE CSV
// ================================================================================

// _____________________________________________________________________
/*!
* Choix du fichier de trace à rejouer
*
*/
void CLidar::on_PB_choixTrace_clicked(void)
{
  QString fileName = QFileDialog::getOpenFileName(NULL,
      tr("Signal"), "./", tr("CSV Files (*.csv);;Signal Files (*.trc)"));

  QFileInfo file_info(fileName);
  QString trace_name = file_info.baseName();
  m_ihm.ui.trace_name->setText(trace_name);
}
