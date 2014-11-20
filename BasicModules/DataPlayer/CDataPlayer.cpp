/*! \file CDataPlayer.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>
#include <QInputDialog>
#include "CDataPlayer.h"
#include "CLaBotBox.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CSignalGenerator.h"


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
 for (tListeVariables::iterator i=m_liste_generateurs.begin(); i!=m_liste_generateurs.end(); i++) {
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
void CDataPlayer::init(CLaBotBox *application)
{
  CModule::init(application);
  setGUI(&m_ihm); // indique à la classe de base l'IHM

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

  val = m_application->m_eeprom->read(getName(), "default_signal_path", QVariant("./"));
  m_default_signal_path = val.toString();

  connect(m_ihm.ui.PB_refresh_liste, SIGNAL(clicked()), this, SLOT(on_PB_refresh_liste_clicked()));
  connect(m_ihm.ui.PB_choixSignal, SIGNAL(clicked()), this, SLOT(on_PB_choixSignal_clicked()));
  connect(m_ihm.ui.PB_StartGeneration, SIGNAL(clicked()), this, SLOT(on_PB_StartGeneration_clicked()));
  connect(m_ihm.ui.PB_StopGeneration, SIGNAL(clicked()), this, SLOT(on_PB_StopGeneration_clicked()));
  connect(m_ihm.ui.dureeEchantillon, SIGNAL(editingFinished()), this, SLOT(on_dureeEchantillon_editingFinished()));
  connect(m_ihm.ui.nombreCycles, SIGNAL(editingFinished()), this, SLOT(on_nombreCycles_editingFinished()));

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

  // Crée un player par défaut pour qu'il y en ai au moins 1 de base
  newPlayer("Player1");

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
  m_application->m_eeprom->write(getName(), "default_signal_path", QVariant(m_default_signal_path));
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
*  Met en cohérence l'IHM  avec l'état de la liste interne
*
*/
void CDataPlayer::miseEnCoherenceListeVariables(void)
{
 // Balaye toutes les variables affichées dans la liste
 for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
    QString ihm_varname = m_ihm.ui.liste_variables->item(i)->text();
    tListeVariables::const_iterator it = m_liste_generateurs.find(ihm_varname);
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
void CDataPlayer::on_PB_refresh_liste_clicked(void)
{
  refreshListeVariables();
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
 m_application->m_print_view->print_info(this, QString(__FUNCTION__));

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
 tListeVariables::const_iterator it = m_liste_generateurs.find(variable_name);
 CSignalGenerator *generator = NULL;
 if (it != m_liste_generateurs.end()) {
     generator = m_liste_generateurs.find(variable_name).value();
 }

 if (item->checkState()) {  // L'utilisateur vient de cocher la case
  if (generator == NULL) {  // aucun générateur n'est associé à cette variable
    generator = new CSignalGenerator(m_application->m_data_center,
                                     variable_name);
    m_liste_generateurs.insert(variable_name, generator);
    connect(generator, SIGNAL(signalStarted(QString)), this, SLOT(refreshGeneratorProperties()));
    connect(generator, SIGNAL(signalFinished(QString)), this, SLOT(refreshGeneratorProperties()));
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
* Récupère un pointeur sur le générateur associé à la variable sélectionnée
*
*/
CSignalGenerator *CDataPlayer::selectedVariableToSignalGenerator(void)
{
  QString varname = "";
  CSignalGenerator *generator = NULL;

  // recherche la variable sélectionnée
  for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
      if (m_ihm.ui.liste_variables->item(i)->isSelected()) {
          varname = m_ihm.ui.liste_variables->item(i)->text();
          break;
      }
  }

  tListeVariables::iterator it = m_liste_generateurs.find(varname);
  if (it != m_liste_generateurs.end()) {
        generator = it.value();
  }
  return(generator);
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

      m_ihm.ui.sliderStepNumTrace->setValue(0);
  }
  else {
    //m_ihm.ui.sliderStepNumTrace->setStyleSheet(QString::fromUtf8("#sliderStepNumTrace {\n \n \n \n }"));
    m_ihm.ui.sliderStepNumTrace->setVisible(true);
    m_ihm.ui.sliderStepNumTrace->setValue(player->getCurrentStepIndex());
  }

  // grise ou dégrise les boutons en fonction de l'état du player
  tPlayerState player_state = player->getState();
  switch (player_state) {
    // ________________________________
    case C_PLAYER_IDLE_NO_DATA : 
        m_ihm.ui.PB_StepBackwardTrace->setEnabled(false);
        m_ihm.ui.PB_StepForwardTrace->setEnabled(false);
        m_ihm.ui.PB_StartTrace->setEnabled(false);
        m_ihm.ui.PB_StopTrace->setEnabled(false);
        m_ihm.ui.sliderStepNumTrace->setEnabled(false);
    break;
    // ________________________________
    case C_PLAYER_STOP : 
        m_ihm.ui.PB_StepBackwardTrace->setEnabled(false);
        m_ihm.ui.PB_StepForwardTrace->setEnabled(true);
        m_ihm.ui.PB_StartTrace->setEnabled(true);
        m_ihm.ui.PB_StartTrace->setIcon(QIcon(":/icons/player_play.png"));
        m_ihm.ui.PB_StopTrace->setEnabled(false);
        m_ihm.ui.sliderStepNumTrace->setEnabled(false);
    break;
    // ________________________________
    case C_PLAYER_RUN : 
        m_ihm.ui.PB_StepBackwardTrace->setEnabled(false);
        m_ihm.ui.PB_StepForwardTrace->setEnabled(false);
        m_ihm.ui.PB_StartTrace->setEnabled(true);
        m_ihm.ui.PB_StartTrace->setIcon(QIcon(":/icons/player_pause.png"));
        m_ihm.ui.PB_StopTrace->setEnabled(true);
        m_ihm.ui.sliderStepNumTrace->setEnabled(false);
    break;
    // ________________________________
    case C_PLAYER_PAUSE : 
        m_ihm.ui.PB_StepBackwardTrace->setEnabled(true);
        m_ihm.ui.PB_StepForwardTrace->setEnabled(true);
        m_ihm.ui.PB_StartTrace->setEnabled(true);
        m_ihm.ui.PB_StartTrace->setIcon(QIcon(":/icons/player_play.png"));
        m_ihm.ui.PB_StopTrace->setEnabled(true);
        m_ihm.ui.sliderStepNumTrace->setEnabled(true);
    break;
    // ________________________________
    case C_PLAYER_PAUSE_FIRST_STEP_INDEX : 
        m_ihm.ui.PB_StepBackwardTrace->setEnabled(false);
        m_ihm.ui.PB_StepForwardTrace->setEnabled(true);
        m_ihm.ui.PB_StartTrace->setEnabled(true);
        m_ihm.ui.PB_StartTrace->setIcon(QIcon(":/icons/player_play.png"));
        m_ihm.ui.PB_StopTrace->setEnabled(true);
        m_ihm.ui.sliderStepNumTrace->setEnabled(true);
    break;
    // ________________________________
    case C_PLAYER_PAUSE_LAST_STEP_INDEX : 
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
      tr("Signal"), m_default_signal_path, tr("Signal Files (*.trc)"));

  QFileInfo file_info(fileName);
  QString trace_name = file_info.baseName();
  m_ihm.ui.trace_name->setText(trace_name);

  m_default_signal_path = file_info.path();

  // Met à jour le fichier dans le player
  selectedPlayerNameToPlayer()->setTraceFilename(fileName);

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

 if (player->getState() == C_PLAYER_RUN) {
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
