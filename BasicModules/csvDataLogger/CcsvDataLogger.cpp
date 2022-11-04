/*! \file CcsvDataLogger.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QFileDialog>
#include "CcsvDataLogger.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CDataListOpenSave.h"

/*! \addtogroup Module_csvDataLogger
   *
   *  @{
   */

// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CcsvDataLogger::CcsvDataLogger(const char *plugin_name)
    :CBasicModule(plugin_name, VERSION_csvDataLogger, AUTEUR_csvDataLogger, INFO_csvDataLogger)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CcsvDataLogger::~CcsvDataLogger()
{
    stop_logger();
}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CcsvDataLogger::init(CApplication *application)
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

    //Restore les dernières variables loggées (créé la data dans le DataManager + l'ajoute sur l'IHM + coche la case)
    val = m_application->m_eeprom->read(getName(), "datalist_logged", "");
    QStringList datalist_memo = val.toStringList();
    addVariablesObserver(datalist_memo);

    // Restore la période d'échantillonnage et le type d'évènements provoquant des entrées dans le fichiers de sortie
    val = m_application->m_eeprom->read(getName(), "log_period", 1000);
    m_ihm.ui.log_period->setValue(val.toInt());

    val = m_application->m_eeprom->read(getName(), "enable_log_periodic", true);
    m_ihm.ui.cb_periodic_log->setChecked(val.toBool());

    val = m_application->m_eeprom->read(getName(), "enable_log_on_value_changed", false);
    m_ihm.ui.cb_log_on_value_changed->setChecked(val.toBool());

    val = m_application->m_eeprom->read(getName(), "enable_log_on_value_updated", false);
    m_ihm.ui.cb_log_on_value_updated->setChecked(val.toBool());

    onLoggerStatusChanged(CDataLogger::LOGGER_OFF);

    // le bouton refresh met à jour la liste des variables disponibles
    connect(m_ihm.ui.PB_refresh_liste, SIGNAL(clicked()), this, SLOT(refreshListeVariables()));
    connect(m_application->m_data_center, SIGNAL(dataCreated(CData*)), this, SLOT(refreshListeVariables()));

    connect(m_ihm.ui.filtre_noms_variables, SIGNAL(textChanged(QString)), this, SLOT(onDataFilterChanged(QString)));
    connect(m_ihm.ui.pb_open_variables_liste, SIGNAL(clicked(bool)), this, SLOT(loadListeVariablesObservees()));
    connect(m_ihm.ui.pb_save_variables_liste, SIGNAL(clicked(bool)), this, SLOT(saveListeVariablesObservees()));
    connect(m_ihm.ui.pb_decoche_tout, SIGNAL(clicked(bool)), this, SLOT(decocheToutesVariables()));
    connect(m_ihm.ui.pb_affiche_cochees, SIGNAL(clicked(bool)), this, SLOT(afficheVariablesCocheesUniquement()));

    connect(m_ihm.ui.pb_select_file, SIGNAL(clicked(bool)), this, SLOT(onSelectFile()));
    connect(m_ihm.ui.pb_start, SIGNAL(clicked(bool)), this, SLOT(start_logger()));
    connect(m_ihm.ui.pb_stop, SIGNAL(clicked(bool)), this, SLOT(stop_logger()));
    connect(m_ihm.ui.pb_pause, SIGNAL(clicked(bool)), this, SLOT(pause_logger()));
    connect(m_ihm.ui.pb_log_once_manualy, SIGNAL(clicked(bool)), this, SLOT(log_once()));

    connect(&m_data_logger, SIGNAL(status_changed(int)), this, SLOT(onLoggerStatusChanged(int)));
    connect(&m_data_logger, SIGNAL(logger_started(QString)), this, SLOT(onLoggerStarted(QString)));
    connect(&m_data_logger, SIGNAL(logger_finished(QString)), this, SLOT(onLoggerFinished(QString)));

    //connect(&m_logger_timer, SIGNAL(timeout()), this, SLOT(onLoggerTick()));
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CcsvDataLogger::close(void)
{
    // Mémorise en EEPROM l'état de la fenêtre
    m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
    m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
    m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
    m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
    m_application->m_eeprom->write(getName(), "datalist_logged", QVariant(getListeVariablesObservees()));
    m_application->m_eeprom->write(getName(), "log_period", QVariant(m_ihm.ui.log_period->value()));
    m_application->m_eeprom->write(getName(), "enable_log_periodic", QVariant(m_ihm.ui.cb_periodic_log->isChecked()));
    m_application->m_eeprom->write(getName(), "enable_log_on_value_changed", QVariant(m_ihm.ui.cb_log_on_value_changed->isChecked()));
    m_application->m_eeprom->write(getName(), "enable_log_on_value_updated", QVariant(m_ihm.ui.cb_log_on_value_updated->isChecked()));
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CcsvDataLogger::onRightClicGUI(QPoint pos)
{
    QMenu *menu = new QMenu();

    menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
    menu->exec(m_ihm.mapToGlobal(pos));
}


// _____________________________________________________________________
/*!
*  Met à jour la liste des variables dans la fenêtre de gauche
*
*/
void CcsvDataLogger::refreshListeVariables(void)
{
    if (!m_ihm.ui.liste_variables->isEnabled()) return;

    // Mémorise d'abord la liste des variables qui étaient à observer (cochées)
    QStringList liste_variables_observees;
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        if (m_ihm.ui.liste_variables->item(i)->checkState() == Qt::Checked) {
            liste_variables_observees.append(m_ihm.ui.liste_variables->item(i)->text());
        }
    } // for toutes les variables sous surveillances

    QStringList var_list;
    m_application->m_data_center->getListeVariablesName(var_list);

    m_ihm.ui.liste_variables->clear();
    m_ihm.ui.liste_variables->addItems(var_list);
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        QString data_name = m_ihm.ui.liste_variables->item(i)->text();
        m_ihm.ui.liste_variables->item(i)->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
        // Restitue le fait que la case était cochée ou décochée avant la mise à jour de liste
        m_ihm.ui.liste_variables->item(i)->setCheckState(liste_variables_observees.contains(data_name) ? Qt::Checked : Qt::Unchecked);
    }
}


// _____________________________________________________________________
/*!
*  Ajout des varibales à observer
*/
void CcsvDataLogger::addVariablesObserver(QStringList liste_variables)
{
    for (int i=0; i<liste_variables.count(); i++) {
        QString dataname = liste_variables.at(i);
        if (dataname.simplified() != "") {
            bool force_creation = true;
            m_application->m_data_center->getData(dataname, force_creation); //crée la donnée si elle n'existe pas. Si elle existe, conserve sa valeur
            m_ihm.ui.liste_variables->addItem(dataname);
        }
    }
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        if (liste_variables.contains(m_ihm.ui.liste_variables->item(i)->text())) {
            m_ihm.ui.liste_variables->item(i)->setCheckState(Qt::Checked);
        }
    }
    refreshListeVariables();
}

// _____________________________________________________________________
/*!
*  Décoche toutes les variables
*/
void CcsvDataLogger::decocheToutesVariables()
{
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        m_ihm.ui.liste_variables->item(i)->setCheckState(Qt::Unchecked);
    }
}

// _____________________________________________________________________
/*!
 * \brief Affiche uniquement les variables cochées
 */
void CcsvDataLogger::afficheVariablesCocheesUniquement()
{
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        bool checked = m_ihm.ui.liste_variables->item(i)->checkState() == Qt::Checked;
        m_ihm.ui.liste_variables->item(i)->setHidden(!checked);
    }
}

// _____________________________________________________________________
/*!
 * \brief Démarre l'enregistrement des données
 */
void CcsvDataLogger::start_logger()
{
    if (m_data_logger.isLogging()) stop_logger();

    m_output_pathfilename = m_ihm.ui.output_pathfilename->text();
    if (m_output_pathfilename.isEmpty()) return;

     QVector<CData *> logged_data_list;
    // Construt a liste des données
    foreach(QString dataname, getListeVariablesObservees()) {
        CData *data = m_application->m_data_center->getData(dataname);
        if (data) logged_data_list.append(data);
    }
    m_data_logger.setDataList(logged_data_list);

    // activer les options suivant les consignes utilisateur
    m_data_logger.setRefreshPeriod(m_ihm.ui.log_period->value());
    m_data_logger.activeLogPeriodically(m_ihm.ui.cb_periodic_log->isChecked());
    m_data_logger.activeLogOnDataChanged(m_ihm.ui.cb_log_on_value_changed->isChecked());
    m_data_logger.activeLogOnDataUpdated(m_ihm.ui.cb_log_on_value_updated->isChecked());

    m_data_logger.start(m_output_pathfilename);
}

// _____________________________________________________________________
/*!
 * \brief Arrête l'enregistrement des données
 */
void CcsvDataLogger::stop_logger()
{
    if (m_data_logger.isLogging()) m_data_logger.stop();
}

// _____________________________________________________________________
/*!
 * \brief Met en pause l'enregistrement des données
 */
void CcsvDataLogger::pause_logger()
{
    if (m_data_logger.isLogging()) m_data_logger.pause();
}

// _____________________________________________________________________
void CcsvDataLogger::log_once()
{
    if (m_data_logger.isLogging()) m_data_logger.log_once();
}

// _____________________________________________________________________
/*!
*  Charge une liste de variables à observer
*/
void CcsvDataLogger::loadListeVariablesObservees(void)
{
    QString fileName = QFileDialog::getOpenFileName(&m_ihm, tr("Open File"), ".");
    if (fileName.isEmpty()) return;

    QStringList liste_variables;
    CDataListOpenSave::open(fileName, liste_variables);

    addVariablesObserver(liste_variables);
}

// _____________________________________________________________________
/*!
*  Sauvegarde la liste des variables observées
*/
void CcsvDataLogger::saveListeVariablesObservees(void)
{
    QString fileName = QFileDialog::getSaveFileName(&m_ihm, tr("Save File"), ".");
    if (fileName.isEmpty()) return;

    QStringList liste_variables = getListeVariablesObservees();
    CDataListOpenSave::save(fileName, liste_variables);
}

// _____________________________________________________________________
/*!
*  Récupère la liste des variables à observer
*/
QStringList CcsvDataLogger::getListeVariablesObservees()
{
    QStringList liste_variables_observees;
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        if (m_ihm.ui.liste_variables->item(i)->checkState() == Qt::Checked) {
            liste_variables_observees.append(m_ihm.ui.liste_variables->item(i)->text());
        }
    }
    return liste_variables_observees;
}

// _____________________________________________________________________
/*!
*  Filtre les variables sur le nom
*/
void CcsvDataLogger::onDataFilterChanged(QString filter_name)
{
    QStringList items_to_match;
    items_to_match = filter_name.toLower().simplified().split(" ");

    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        m_ihm.ui.liste_variables->item(i)->setHidden(true);
        bool found = true;
        foreach (QString item_to_match, items_to_match) {
            if (!m_ihm.ui.liste_variables->item(i)->text().toLower().contains(item_to_match)) {
                found = false; // si tous les mots clés du filtre ne sont pas présents dans le nom du script, le script est rejeté
            }
        }
        m_ihm.ui.liste_variables->item(i)->setHidden(!found);
    }
}

// _____________________________________________________________________
/*!
 * \brief Changement de statut du logger
 * \param status le nouveau statut
 */
void CcsvDataLogger::onLoggerStatusChanged(int status)
{
    switch(status) {
    case CDataLogger::LOGGER_OFF :
        m_ihm.ui.pb_start->setEnabled(true);
        m_ihm.ui.pb_stop->setEnabled(false);
        m_ihm.ui.pb_pause->setEnabled(false);
        m_ihm.ui.pb_log_once_manualy->setEnabled(false);
        m_ihm.ui.cb_periodic_log->setEnabled(true);
        m_ihm.ui.cb_log_on_value_changed->setEnabled(true);
        m_ihm.ui.cb_log_on_value_updated->setEnabled(true);
        m_ihm.ui.log_period->setEnabled(true);
        m_ihm.ui.pb_select_file->setEnabled(true);
        m_ihm.ui.output_pathfilename->setEnabled(true);
        m_ihm.ui.liste_variables->setEnabled(true);
        m_ihm.ui.pb_decoche_tout->setEnabled(true);
        break;

    case CDataLogger::LOGGER_READY_TO_START :
        m_ihm.ui.pb_start->setEnabled(true);
        m_ihm.ui.pb_stop->setEnabled(false);
        m_ihm.ui.pb_pause->setEnabled(false);
        break;

    case CDataLogger::LOGGER_ON :
    case CDataLogger::LOGGER_CONTINUE :
        m_ihm.ui.pb_start->setEnabled(false);
        m_ihm.ui.pb_stop->setEnabled(true);
        m_ihm.ui.pb_pause->setEnabled(true);
        m_ihm.ui.pb_log_once_manualy->setEnabled(true);
        m_ihm.ui.cb_periodic_log->setEnabled(false);
        m_ihm.ui.cb_log_on_value_changed->setEnabled(false);
        m_ihm.ui.cb_log_on_value_updated->setEnabled(false);
        m_ihm.ui.log_period->setEnabled(false);
        m_ihm.ui.pb_select_file->setEnabled(false);
        m_ihm.ui.output_pathfilename->setEnabled(false);
        m_ihm.ui.liste_variables->setEnabled(false);
        m_ihm.ui.pb_decoche_tout->setEnabled(false);
        break;

    case CDataLogger::LOGGER_PAUSE :
        m_ihm.ui.pb_pause->setEnabled(false);
        m_ihm.ui.pb_start->setEnabled(false);
        break;

    default :
        break;
    }
}

// _____________________________________________________________________
void CcsvDataLogger::onLoggerStarted(QString filename)
{
    QString filename_no_path = QFileInfo(filename).fileName();
    m_ihm.ui.statusbar->showMessage(QString("Logger start : %1").arg(filename_no_path), 2000);
}

// _____________________________________________________________________
void CcsvDataLogger::onLoggerFinished(QString filename)
{
    QString filename_no_path = QFileInfo(filename).fileName();
    m_ihm.ui.statusbar->showMessage(QString("Logger stop : %1").arg(filename_no_path), 2000);
}

// _____________________________________________________________________
void CcsvDataLogger::onSelectFile()
{
    QString default_pathfilename = getDefaultPathfilename();
    QString pathfilename = QFileDialog::getSaveFileName(NULL, tr("Fichier"),
                                                    QString(default_pathfilename), tr("csv Files (*.csv)"));

    if(!pathfilename.isEmpty() && !pathfilename.isNull()){  //Différencie clic sur OK et Cancel
        if (pathfilename.endsWith(".csv") == false){  // l'extension .csv est ajoutée si elle n'est pas déjà indiquée
            pathfilename = pathfilename + ".csv";
        }
        m_ihm.ui.output_pathfilename->setText(pathfilename);
    }
}

// _____________________________________________________________________
QString CcsvDataLogger::getDefaultPathfilename(){
    QString default_pathfilename;
    QDateTime date_time = QDateTime::currentDateTime();
    default_pathfilename = \
            m_application->m_pathname_log_file +
            "/" +
            QString(getName()).simplified().replace(" ", "") +
            "_" +
            date_time.toString("yyyy_MM_dd_hh'h'mm'm'ss's'") +
            ".csv";
    return default_pathfilename;
}
