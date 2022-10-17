/*! \file CDataPlayer.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include "CDataPlayer.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CSignalGenerator.h"
#include "csvParser.h"

/*! \addtogroup Module_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CDataPlayer::CDataPlayer(const char *plugin_name)
    :CBasicModule(plugin_name, VERSION_DataPlayer, AUTEUR_DataPlayer, INFO_DataPlayer)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CDataPlayer::~CDataPlayer()
{
 // Efface la liste des générateurs de signaux
 for (tListeGenerateurs::iterator i=m_liste_generateurs.begin(); i!=m_liste_generateurs.end(); i++) {
        delete i.value();
 }

 // Efface la liste des players
 for (tListePlayers::iterator i=m_liste_players.begin(); i!=m_liste_players.end(); i++) {
        delete i.value();
 }
}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CDataPlayer::init(CApplication *application)
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

  val = m_application->m_eeprom->read(getName(), "default_signal_path", QVariant("./"));
  m_default_signal_path = val.toString();

  connect(m_application->m_data_center, SIGNAL(dataCreated(CData*)), this, SLOT(refreshListeVariables()));
  connect(m_ihm.ui.PB_refresh_liste, SIGNAL(clicked()), this, SLOT(refreshListeVariables()));
  connect(m_ihm.ui.PB_choixSignal, SIGNAL(clicked()), this, SLOT(on_PB_choixSignal_clicked()));
  connect(m_ihm.ui.PB_StartGeneration, SIGNAL(clicked()), this, SLOT(on_PB_StartGeneration_clicked()));
  connect(m_ihm.ui.PB_StopGeneration, SIGNAL(clicked()), this, SLOT(on_PB_StopGeneration_clicked()));
  connect(m_ihm.ui.dureeEchantillon, SIGNAL(editingFinished()), this, SLOT(on_dureeEchantillon_editingFinished()));
  connect(m_ihm.ui.nombreCycles, SIGNAL(editingFinished()), this, SLOT(on_nombreCycles_editingFinished()));

  connect(m_ihm.ui.filtre_noms_variables, SIGNAL(textChanged(QString)), this, SLOT(onDataFilterChanged(QString)));
  connect(m_ihm.ui.pb_open_variables_liste, SIGNAL(clicked(bool)), this, SLOT(loadListeVariablesAvecGenerateur()));
  connect(m_ihm.ui.pb_save_variables_liste, SIGNAL(clicked(bool)), this, SLOT(saveListeVariablesAvecGenerateur()));
  connect(m_ihm.ui.pb_decoche_tout, SIGNAL(clicked(bool)), this, SLOT(decocheToutesVariables()));

  connect(m_ihm.ui.liste_variables, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(on_checkVariable(QListWidgetItem*)));
  connect(m_ihm.ui.liste_variables, SIGNAL(itemSelectionChanged()), this, SLOT(on_selectVariable()));

  // Joueur de trace
  connect(m_ihm.ui.PB_choixTrace, SIGNAL(clicked()), this, SLOT(on_PB_choixTrace_clicked()));
  connect(m_ihm.ui.PB_AjoutPlayer, SIGNAL(clicked()), this, SLOT(on_PB_AjoutPlayer_clicked()));
  connect(m_ihm.ui.PB_StepBackwardTrace, SIGNAL(clicked()), this, SLOT(on_PB_StepBackwardTrace_clicked()));
  connect(m_ihm.ui.PB_StartTrace, SIGNAL(clicked()), this, SLOT(on_PB_StartTrace_clicked()));
  connect(m_ihm.ui.PB_StopTrace, SIGNAL(clicked()), this, SLOT(on_PB_StopTrace_clicked()));
  connect(m_ihm.ui.PB_StepForwardTrace, SIGNAL(clicked()), this, SLOT(on_PB_StepForwardTrace_clicked()));
  connect(m_ihm.ui.playerNameListe, SIGNAL(currentIndexChanged(QString)), this, SLOT(on_playerNameListe_changed(QString)));
  connect(m_ihm.ui.dureeStepTrace, SIGNAL(editingFinished()), this, SLOT(on_dureeStepTrace_editingFinished()));
  connect(m_ihm.ui.nombreCyclesTrace, SIGNAL(editingFinished()), this, SLOT(on_nombreCyclesTrace_editingFinished()));

  connect(m_ihm.ui.PB_StartAllTraces_Synchro, SIGNAL(clicked()), this, SLOT(on_PB_StartAllTraces_Synchro_clicked()));
  connect(m_ihm.ui.PB_StopAllTraces_Synchro, SIGNAL(clicked()), this, SLOT(on_PB_StopAllTraces_Synchro_clicked()));
  connect(m_ihm.ui.sliderStepNumTrace, SIGNAL(sliderMoved(int)), this, SLOT(on_sliderStepNumTrace_moved(int)));

  m_ihm.ui.groupBox_ConfigGenerator->setEnabled(false);

  // Restitue la liste des variables associés à des générateurs
  loadListeVariablesAvecGenerateur(getName().simplified() + "generator_autosave.csv");

  // Crée un player par défaut pour qu'il y en ai au moins 1 de base
  newPlayer("Player1");

  QIcon icon = QIcon(":/icons/signal.png");
  m_ihm.ui.toolBar->addAction(icon, "Action!");

}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CDataPlayer::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
  m_application->m_eeprom->write(getName(), "default_signal_path", QVariant(m_default_signal_path));

  // Sauvegarde la liste des générateurs de fonctions sur les variables
  QString pathfilename = getName().simplified() + "generator_autosave.csv";
  saveListeVariablesAvecGenerateur(pathfilename);
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CDataPlayer::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

// _____________________________________________________________________
/*!
*  Point d'entrée lorsque la fenêtre est appelée
*
*/
void CDataPlayer::setVisible(void)
{
  CModule::setVisible();
  refreshListeVariables();
  refreshGeneratorProperties();
}




// ===============================================================
//              PAGE GENERATION DE SIGNAUX
// ===============================================================


// _____________________________________________________________________
/*!
*  Met à jour la liste des variables existantes
*
*/
void CDataPlayer::refreshListeVariables(void)
{
 QStringList var_list;

 disconnect(m_ihm.ui.liste_variables, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(on_checkVariable(QListWidgetItem*)));

 m_application->m_data_center->getListeVariablesName(var_list);

 m_ihm.ui.liste_variables->clear();
 m_ihm.ui.liste_variables->addItems(var_list);
 for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
    m_ihm.ui.liste_variables->item(i)->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsSelectable);
    m_ihm.ui.liste_variables->item(i)->setCheckState(Qt::Unchecked);
 }
 miseEnCoherenceListeVariables();
 connect(m_ihm.ui.liste_variables, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(on_checkVariable(QListWidgetItem*)));
}

// _____________________________________________________________________
/*!
*  Filtre les variables sur le nom
*/
void CDataPlayer::onDataFilterChanged(QString filter_name)
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
*  Décoche toutes les variables
*/
void CDataPlayer::decocheToutesVariables()
{
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        m_ihm.ui.liste_variables->item(i)->setCheckState(Qt::Unchecked);
    }
}

// _____________________________________________________________________
/*!
*  Charge une liste de variables à observer
*/
void CDataPlayer::loadListeVariablesAvecGenerateur(void)
{
    QString pathfileName = QFileDialog::getOpenFileName(&m_ihm, tr("Open File"), ".csv");
    if (pathfileName.isEmpty()) return;

    loadListeVariablesAvecGenerateur(pathfileName);
}

// _____________________________________________________________________
void CDataPlayer::loadListeVariablesAvecGenerateur(QString pathfilename)
{
    csvData csv_data;
    csvParser parser;
    QStringList error_msg, warn_msg;

    // Lit et analyse le fichier d'entrée
    parser.enableEmptyCells(false);
    parser.setNumberOfExpectedColums(4); // Nom de la data ; chemin du fichier générateur ; durée d'un échantillon ; nombre de cycles
    bool status = parser.parse(pathfilename, csv_data, &error_msg, &warn_msg);
    if (!status) {
        m_application->m_print_view->print_error(this, QString("Fichier csv non conforme: %1").arg(pathfilename));
        foreach(QString msg, error_msg) m_application->m_print_view->print_error(this, msg);
        return; // pas la peine d'aller plus loin, le fichier n'est pas conforme
    }
    if (!warn_msg.isEmpty()) foreach(QString msg, warn_msg) m_application->m_print_view->print_warning(this, msg);

    for (int line=0; line<csv_data.m_datas.size(); line++) {
        QStringList data_line = csv_data.m_datas.at(line);
        QString varname = data_line.at(0);
        QString generator_filename = data_line.at(1);
        int sample_duration = data_line.at(2).toInt();
        int cycle_count = data_line.at(3).toInt();

        // La data n'existe pas encore dans le DataManager -> la créé
        if (!m_application->m_data_center->isExist(varname)) m_application->m_data_center->createData(varname);
        if (!QFileInfo(generator_filename).exists()) { // vérifie si le fichier de signal existe bien
            m_application->m_print_view->print_warning(this, QString("Le fichier %1 associé à la data %2 n'existe pas").arg(generator_filename).arg(varname));
            continue;
        }
        // crée le générateur associé et lui affecte le fichier contenant le signal
        CSignalGenerator* generator = createGenerator(varname);
        if (generator) {
            generator->setSignalFilename(generator_filename);
            generator->setCommonSampleDuration(sample_duration);
            generator->setCycleNumber(cycle_count);
        }
    }
    // une fois les données et générateurs créés, met en cohérence l'IHM avec la liste des générateurs
    miseEnCoherenceListeVariables();
}

// _____________________________________________________________________
/*!
*  Sauvegarde la liste des variables observées
*/
void CDataPlayer::saveListeVariablesAvecGenerateur(void)
{
    if (m_liste_generateurs.size() == 0) { // la liste est vide, pas la peine de sauvegarder un fichier vide
        QMessageBox::information(getGUI(), tr(""), tr("La liste des générateurs est vide"));
        return;
    }

    QStringList list_variables;
    QStringList list_fichiers_generateurs;
    for (tListeGenerateurs::iterator it=m_liste_generateurs.begin(); it!=m_liste_generateurs.end(); it++) {
        list_variables << it.key();
        list_fichiers_generateurs << it.value()->getSignalFilename();
    }

    QString pathfilename = QFileDialog::getSaveFileName(&m_ihm, tr("Save File"), ".");
    if (pathfilename.isEmpty()) return;

    saveListeVariablesAvecGenerateur(pathfilename);
}

// _____________________________________________________________________
void CDataPlayer::saveListeVariablesAvecGenerateur(QString pathfilename)
{
    QFile file(pathfilename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_application->m_print_view->print_error(this, "Impossible d'ouvrir en écriture le fichier " + pathfilename);
        return;
    }
    // Ecrit le fichier de type csv
    QTextStream out(&file);
    const QString CSV_SEPARATOR = ";";
    out << "Name" << CSV_SEPARATOR
        << "Generator pathfilename" << CSV_SEPARATOR
        << "Sample duration [msec]" << CSV_SEPARATOR
        << "Cycle count";
    endl(out);
    for (tListeGenerateurs::iterator it=m_liste_generateurs.begin(); it!=m_liste_generateurs.end(); it++) {
        out << it.key() << CSV_SEPARATOR                            // Nom de la data
            << it.value()->getSignalFilename() << CSV_SEPARATOR
            << it.value()->getCommonSampleDuration() << CSV_SEPARATOR
            << it.value()->getCycleNumber();
        endl(out);
    }
    file.close();
}

// _____________________________________________________________________
/*!
*  Met en cohérence l'IHM  avec l'état de la liste interne
*
*/
void CDataPlayer::miseEnCoherenceListeVariables(void)
{
 // Balaye toutes les variables affichées dans la liste
 for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
    QString ihm_varname = m_ihm.ui.liste_variables->item(i)->text();
    tListeGenerateurs::const_iterator it = m_liste_generateurs.find(ihm_varname);
    if (it != m_liste_generateurs.end()) {
        if (it.value() != NULL) {
            m_ihm.ui.liste_variables->item(i)->setCheckState(Qt::Checked);
        }
    } // si la variable est dans la liste des variables possédant un générateur
 } // for toutes les variables listées sur l'IHM
}




void CDataPlayer::signalGenerationFinished(QString var_name)
{
  m_application->m_print_view->print_info(this, "Génération terminée pour signal: " + var_name);
}

// _____________________________________________________________________
/*!
*
*
*/
void CDataPlayer::on_PB_choixSignal_clicked(void)
{
  m_application->m_print_view->print_info(this, QString(__FUNCTION__));

  QString fileName = QFileDialog::getOpenFileName(NULL,
      tr("Signal"), m_default_signal_path, tr("Signal Files (*.sig)"));

  QFileInfo file_info(fileName);
  QString signal_name = file_info.baseName();
  m_ihm.ui.signal_name->setText(signal_name);

  m_default_signal_path = file_info.path();

  CSignalGenerator *generator = selectedVariableToSignalGenerator();
  if (generator != NULL) {
     generator->setSignalFilename(fileName);
  }
}

// _____________________________________________________________________
/*!
*
*
*/
void CDataPlayer::on_PB_StartGeneration_clicked(void)
{
  m_application->m_print_view->print_info(this, QString(__FUNCTION__));
  CSignalGenerator *generator = selectedVariableToSignalGenerator();
  if (generator!=NULL) {
    generator->start_generator();
  }
}

// _____________________________________________________________________
/*!
*
*
*/
void CDataPlayer::on_PB_StopGeneration_clicked(void)
{
 m_application->m_print_view->print_info(this, QString(__FUNCTION__));

 CSignalGenerator *generator = selectedVariableToSignalGenerator();
 if (generator!=NULL) {
   generator->stop_generator();
 }
}

// _____________________________________________________________________
/*!
*
*
*/
void CDataPlayer::on_dureeEchantillon_editingFinished(void)
{
 CSignalGenerator *generator = selectedVariableToSignalGenerator();
 if (generator != NULL) {
       generator->setCommonSampleDuration(m_ihm.ui.dureeEchantillon->value());
 }
}



// _____________________________________________________________________
/*!
*
*
*/
void CDataPlayer::on_nombreCycles_editingFinished(void)
{
 CSignalGenerator *generator = selectedVariableToSignalGenerator();
 if (generator != NULL) {
    generator->setCycleNumber(m_ihm.ui.nombreCycles->value());
 }
}



// _____________________________________________________________________
/*!
* Sélection d'une variable dans la liste des variables disponibles
* L'évènement est levé quand :
*   - L'utilisateur coche ou décoche la checkbox associée
*
*/
void CDataPlayer::on_selectVariable(void)
{
 CSignalGenerator *generator = selectedVariableToSignalGenerator();

 if (generator != NULL) {
    m_ihm.ui.groupBox_ConfigGenerator->setEnabled(true);
    refreshGeneratorProperties();
 }
 else {
    m_ihm.ui.groupBox_ConfigGenerator->setEnabled(false);
    //met des valeurs par défaut dans les champs
    m_ihm.ui.dureeEchantillon->setValue(0);
    m_ihm.ui.nombreCycles->setValue(0);
    m_ihm.ui.signal_name->setText("");
 }



}

// _____________________________________________________________________
/*!
* Sélection d'une variable dans la liste des variables disponibles
* L'évènement est levé quand :
*   - L'utilisateur coche ou décoche la checkbox associée
*
*/
void CDataPlayer::on_checkVariable(QListWidgetItem* item)
{
 m_application->m_print_view->print_info(this, QString(__FUNCTION__));

 QString variable_name = item->text();
 CSignalGenerator *generator = getGenerator(variable_name);

 if (item->checkState()) {  // L'utilisateur vient de cocher la case
  if (generator == NULL) {  // aucun générateur n'est associé à cette variable
    generator = createGenerator(variable_name);
  }
  // else : si un générateur était déjà affecté, n'en crée pas (situation qui ne devrait pas arriver)
 }
 else {  // l'utilisateur à décoché la case
    if (generator != NULL) {
        generator->stop_generator();
        delete generator;
        m_liste_generateurs.remove(variable_name);
    }
    // else : aucun générateur n'existe -> donc rien à supprimer
 }

 // Met en surbrillance la liste correspondance
 item->setSelected(true);
 on_selectVariable();
}

// _____________________________________________________________________
/*!
 * \brief Créé un générateur pour la variable
 * \param varname la variable pour laquelle créer le générateur
 * \return un pointeur sur le générateur créé ou le générateur existant
 */
CSignalGenerator *CDataPlayer::createGenerator(QString varname)
{
    // Pas de création si la data est déjà associée à un générateur
    CSignalGenerator *generator = getGenerator(varname);
    if (generator) return generator;

    generator = new CSignalGenerator(m_application->m_data_center,
                                     varname);
    m_liste_generateurs.insert(varname, generator);
    connect(generator, SIGNAL(signalStarted(QString)), this, SLOT(refreshGeneratorProperties()));
    connect(generator, SIGNAL(signalFinished(QString)), this, SLOT(refreshGeneratorProperties()));
    return generator;
}

// _____________________________________________________________________
/*!
 * \brief Récupère le générateur associé à une data
 * \param varname le nom de la data
 * \return pointeur sur le générateur ou NULL si aucun générateur n'est associé à la data
 */
CSignalGenerator *CDataPlayer::getGenerator(QString varname)
{
    tListeGenerateurs::const_iterator it = m_liste_generateurs.find(varname);
    CSignalGenerator *generator = NULL;
    if (it != m_liste_generateurs.end()) {
        generator = it.value();
    }
    return generator;
}

// _____________________________________________________________________
/*!
* Récupère un pointeur sur le générateur associé à la variable sélectionnée
*
*/
CSignalGenerator *CDataPlayer::selectedVariableToSignalGenerator(void)
{
  QString varname = "";

  // recherche la variable sélectionnée
  for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
      if (m_ihm.ui.liste_variables->item(i)->isSelected()) {
          varname = m_ihm.ui.liste_variables->item(i)->text();
          break;
      }
  }
  return getGenerator(varname);
}




// _____________________________________________________________________
/*!
* Met à jour les propriétés du générateur sur l'IHM
*
*/
void CDataPlayer::refreshGeneratorProperties(void)
{
 CSignalGenerator *generator = selectedVariableToSignalGenerator();
 if (generator != NULL) {
    m_ihm.ui.dureeEchantillon->setValue(generator->getCommonSampleDuration());
    m_ihm.ui.nombreCycles->setValue(generator->getCycleNumber());
    QString signal_filename = generator->getSignalFilename();
    QFileInfo file_info(signal_filename);
    QString signal_name = file_info.baseName();
    m_ihm.ui.signal_name->setText(signal_name);
    if (generator->isGeneratorActive()) {
        m_ihm.ui.PB_StartGeneration->setEnabled(false);
        m_ihm.ui.PB_StopGeneration->setEnabled(true);
    }
    else  {
        m_ihm.ui.PB_StartGeneration->setEnabled(true);
        m_ihm.ui.PB_StopGeneration->setEnabled(false);
    }
 }
}


// ===============================================================
//              PAGE DE REJOUEUR DE TRACE
// ===============================================================

// _____________________________________________________________________
/*!
* Renvoie un pointeur sur le player dont le nom est affiché dans la combobox
* Renvoie NULL si le player n'existe pas
*
*/
CTracePlayer* CDataPlayer::selectedPlayerNameToPlayer(void)
{
  CTracePlayer *pplayer=NULL;

  tListePlayers::iterator it = m_liste_players.find(m_ihm.ui.playerNameListe->currentText());
  if (it != m_liste_players.end()) { 
      pplayer = it.value(); 
  }
  return(pplayer);
}


// _____________________________________________________________________
/*!
* Met à jour l'interface du player
*
*/
void CDataPlayer::refreshIHMGenerateur(void)
{
  CTracePlayer *player = selectedPlayerNameToPlayer();
  if (player == NULL) { 
      m_application->m_print_view->print_error(this, QString(__FUNCTION__) + " : Pointeur <player> null");
      return; 
  }
  
  m_ihm.ui.nombreCyclesTrace->setValue(player->getCycleNumber());
  m_ihm.ui.dureeStepTrace->setValue(player->getCommonStepDuration());

  QFileInfo file_info(player->getTraceFilename());
  m_ihm.ui.trace_name->setText(file_info.baseName());

  // met à jour la valeur max du slider qui correspond au nombre de steps du player
  int max_step = player->getStepsSize()-1;
  if (max_step > 0) {
      m_ihm.ui.sliderStepNumTrace->setMaximum(max_step);
      m_ihm.ui.sliderStepNumTrace->setPageStep(max_step/10);
  }
  int current_step = player->getCurrentStepIndex();
  if (current_step<0) {
      //m_ihm.ui.sliderStepNumTrace->setStyleSheet(QString::fromUtf8("#sliderStepNumTrace {\n	background-color: rgb(195, 195, 195);\n \n \n }"));
      m_ihm.ui.sliderStepNumTrace->setVisible(false);
      m_ihm.ui.valStepNumTrace->setVisible(false);
      m_ihm.ui.lbl_valStepNumTrace->setVisible(false);

      m_ihm.ui.sliderStepNumTrace->setValue(0);
      m_ihm.ui.valStepNumTrace->setText("");
  }
  else {
    //m_ihm.ui.sliderStepNumTrace->setStyleSheet(QString::fromUtf8("#sliderStepNumTrace {\n \n \n \n }"));
    m_ihm.ui.sliderStepNumTrace->setVisible(true);
    m_ihm.ui.valStepNumTrace->setVisible(true);
    m_ihm.ui.lbl_valStepNumTrace->setVisible(true);
    m_ihm.ui.sliderStepNumTrace->setValue(current_step);
    m_ihm.ui.valStepNumTrace->setText(QString::number(current_step+1));
  }

  // grise ou dégrise les boutons en fonction de l'état du player
  CTracePlayer::tPlayerState player_state = player->getState();
  switch (player_state) {
    // ________________________________
    case CTracePlayer::C_PLAYER_IDLE_NO_DATA :
        m_ihm.ui.PB_StepBackwardTrace->setEnabled(false);
        m_ihm.ui.PB_StepForwardTrace->setEnabled(false);
        m_ihm.ui.PB_StartTrace->setEnabled(false);
        m_ihm.ui.PB_StopTrace->setEnabled(false);
        m_ihm.ui.sliderStepNumTrace->setEnabled(false);
    break;
    // ________________________________
    case CTracePlayer::C_PLAYER_STOP :
        m_ihm.ui.PB_StepBackwardTrace->setEnabled(false);
        m_ihm.ui.PB_StepForwardTrace->setEnabled(true);
        m_ihm.ui.PB_StartTrace->setEnabled(true);
        m_ihm.ui.PB_StartTrace->setIcon(QIcon(":/icons/player_play.png"));
        m_ihm.ui.PB_StopTrace->setEnabled(false);
        m_ihm.ui.sliderStepNumTrace->setEnabled(false);
    break;
    // ________________________________
    case CTracePlayer::C_PLAYER_RUN :
        m_ihm.ui.PB_StepBackwardTrace->setEnabled(false);
        m_ihm.ui.PB_StepForwardTrace->setEnabled(false);
        m_ihm.ui.PB_StartTrace->setEnabled(true);
        m_ihm.ui.PB_StartTrace->setIcon(QIcon(":/icons/player_pause.png"));
        m_ihm.ui.PB_StopTrace->setEnabled(true);
        m_ihm.ui.sliderStepNumTrace->setEnabled(false);
    break;
    // ________________________________
    case CTracePlayer::C_PLAYER_PAUSE :
        m_ihm.ui.PB_StepBackwardTrace->setEnabled(true);
        m_ihm.ui.PB_StepForwardTrace->setEnabled(true);
        m_ihm.ui.PB_StartTrace->setEnabled(true);
        m_ihm.ui.PB_StartTrace->setIcon(QIcon(":/icons/player_play.png"));
        m_ihm.ui.PB_StopTrace->setEnabled(true);
        m_ihm.ui.sliderStepNumTrace->setEnabled(true);
    break;
    // ________________________________
    case CTracePlayer::C_PLAYER_PAUSE_FIRST_STEP_INDEX :
        m_ihm.ui.PB_StepBackwardTrace->setEnabled(false);
        m_ihm.ui.PB_StepForwardTrace->setEnabled(true);
        m_ihm.ui.PB_StartTrace->setEnabled(true);
        m_ihm.ui.PB_StartTrace->setIcon(QIcon(":/icons/player_play.png"));
        m_ihm.ui.PB_StopTrace->setEnabled(true);
        m_ihm.ui.sliderStepNumTrace->setEnabled(true);
    break;
    // ________________________________
    case CTracePlayer::C_PLAYER_PAUSE_LAST_STEP_INDEX :
        m_ihm.ui.PB_StepBackwardTrace->setEnabled(true);
        m_ihm.ui.PB_StepForwardTrace->setEnabled(false);
        m_ihm.ui.PB_StartTrace->setEnabled(true);
        m_ihm.ui.PB_StartTrace->setIcon(QIcon(":/icons/player_play.png"));
        m_ihm.ui.PB_StopTrace->setEnabled(true);
        m_ihm.ui.sliderStepNumTrace->setEnabled(true);
    break;
    // ________________________________
    default : // situation qui ne devrait jamais arriver
        bool state = true;
        m_ihm.ui.PB_StepBackwardTrace->setEnabled(state);
        m_ihm.ui.PB_StepForwardTrace->setEnabled(state);
        m_ihm.ui.PB_StartTrace->setEnabled(state);
        m_ihm.ui.PB_StopTrace->setEnabled(state);
        m_ihm.ui.sliderStepNumTrace->setEnabled(state);
    break;
  } //swtich l'état du player
}

// _____________________________________________________________________
/*!
* Choix du fichier de trace à rejouer
*
*/
void CDataPlayer::on_PB_choixTrace_clicked(void)
{
  QString fileName = QFileDialog::getOpenFileName(NULL,
      tr("Signal"), m_default_signal_path, tr("CSV Files (*.csv);;Signal Files (*.trc)"));

  QFileInfo file_info(fileName);
  QString trace_name = file_info.baseName();
  m_ihm.ui.trace_name->setText(trace_name);

  m_default_signal_path = file_info.path();

  // Met à jour le fichier dans le player
  bool success = selectedPlayerNameToPlayer()->setTraceFilename(fileName);
  if (!success) {
      QString msg = QString("Fichier d'entrée invalide : %1").arg(fileName);
      m_application->m_print_view->print_error(this, msg);
      QMessageBox::warning(&m_ihm, "Analyse du fichier", msg);
  }

  // si la liste contient plus d'un player, autorise le start et le stop 
  // synchronisé sur tous les players en même temps
  if (m_liste_players.size() > 1) {
    m_ihm.ui.PB_StartAllTraces_Synchro->setEnabled(true);
    m_ihm.ui.PB_StopAllTraces_Synchro->setEnabled(true);
  }

  // Met à jour l'IHM
  refreshIHMGenerateur();
}


// _____________________________________________________________________
/*!
* Callback de changement de valeur
*
*/
void CDataPlayer::on_dureeStepTrace_editingFinished(void)
{
 selectedPlayerNameToPlayer()->setCommonStepDuration(m_ihm.ui.dureeStepTrace->value());
}


// _____________________________________________________________________
/*!
* Callback de changement de valeur
*
*/
void CDataPlayer::on_nombreCyclesTrace_editingFinished(void)
{
 selectedPlayerNameToPlayer()->setCycleNumber(m_ihm.ui.nombreCyclesTrace->value());
}



// _____________________________________________________________________
/*!
* L'utilisateur change le player
*
*/
void CDataPlayer::on_playerNameListe_changed(QString player_name)
{
 Q_UNUSED(player_name)
 refreshIHMGenerateur();
}

// _____________________________________________________________________
/*!
* 
*
*/
void CDataPlayer::on_PB_AjoutPlayer_clicked(void)
{
  bool ok;
  QString text = QInputDialog::getText(NULL, tr("Nom du player à créer"),
                                          tr(""), QLineEdit::Normal,
                                          "Player_n", &ok);    
  if (ok && !text.isEmpty()) {
    newPlayer(text);
  }
}

// _____________________________________________________________________
/*!
* Ajoute un player dont le nom est passé
*
*/
void CDataPlayer::newPlayer(QString player_name)
{
 if (m_liste_players.find(player_name) != m_liste_players.end() ) { return; }  // Ce nom de player existe déjà
 
 CTracePlayer *player = new CTracePlayer(m_application->m_data_center, player_name);
 m_liste_players[player_name] = player;
 int insert_index = 0;
 m_ihm.ui.playerNameListe->insertItem(insert_index, player_name);
 m_ihm.ui.playerNameListe->setCurrentIndex(insert_index);

 // connecte les signaux émis avec la mise à jour de l'IHM
 connect(player, SIGNAL(stateChanged(QString)), this, SLOT(on_TracePlayerStateChanged(QString)));
 connect(player, SIGNAL(oneStepFinished(QString)), this, SLOT(on_TracePlayerStateChanged(QString)));

}


// _____________________________________________________________________
/*!
* Capture des évènements émis par CTracePlayer
*
*/
void CDataPlayer::on_TracePlayerStateChanged(QString player_name)
{
 // si le player qui a émis l'évènement correspond au player affiché
 // l'IHM est mise à jour
 if (m_ihm.ui.playerNameListe->currentText() == player_name) {
  refreshIHMGenerateur();
 }
}


// _____________________________________________________________________
/*!
* Joue le pas précédent 
*
*/
void CDataPlayer::on_PB_StepBackwardTrace_clicked(void)
{
 selectedPlayerNameToPlayer()->previousStep();
}
// _____________________________________________________________________
/*!
* Joue le pas suivant
*
*/
void CDataPlayer::on_PB_StepForwardTrace_clicked(void)
{
 selectedPlayerNameToPlayer()->nextStep();
}
// _____________________________________________________________________
/*!
* Joue ou met en pause la trace 
*
*/
void CDataPlayer::on_PB_StartTrace_clicked(void)
{
 CTracePlayer *player = selectedPlayerNameToPlayer();
 if (player == NULL) { return; }

 if (player->getState() == CTracePlayer::C_PLAYER_RUN) {
    player->pausePlayer();
 }
 else {
    player->startPlayer();
 }
}
// _____________________________________________________________________
/*!
* Arrête de jouer la trace en cours 
*
*/
void CDataPlayer::on_PB_StopTrace_clicked(void)
{
  selectedPlayerNameToPlayer()->stopPlayer();
}



// _____________________________________________________________________
/*!
* Démarrage synchronisé de tous les modules 
*
*/
void CDataPlayer::on_PB_StartAllTraces_Synchro_clicked(void)
{
  for (tListePlayers::iterator it=m_liste_players.begin(); it!=m_liste_players.end(); it++) {
    it.value()->startPlayer();
  }
}

// _____________________________________________________________________
/*!
* Arrêt synchronisé de tous les modules 
*
*/
void CDataPlayer::on_PB_StopAllTraces_Synchro_clicked(void)
{
  for (tListePlayers::iterator it=m_liste_players.begin(); it!=m_liste_players.end(); it++) {
    it.value()->stopPlayer();
  }
}

// _____________________________________________________________________
/*!
* Callback d'action utilisateur sur le slider
*
*/
void CDataPlayer::on_sliderStepNumTrace_moved(int val)
{
 selectedPlayerNameToPlayer()->selectAndPlayStep(val);
}
