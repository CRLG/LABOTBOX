/*! \file CTracePlayer.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include "CTracePlayer.h"
#include "CDataManager.h"
#include <math.h>



/*! \addtogroup Data
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CTracePlayer::CTracePlayer(CDataManager *data_manager, QString player_name/*=""*/)
    :   m_player_name(player_name),
        m_data_manager(data_manager),
        m_trace_filename(""),
        m_common_step_duration(C_STEP_DURATION_IN_FILE),
        m_step_index(-1),
        m_cycle_number(1),
        m_cycle_count(0),
        m_player_state(C_PLAYER_IDLE_NO_DATA)
{

}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CTracePlayer::~CTracePlayer()
{
  clearSteps();
}


// _____________________________________________________________________
/*!
*  Nettoie la trace courante
*
*/
void CTracePlayer::clearSteps(void)
{
  for (int i=0; i<m_steps.size(); i++) {
        delete m_steps[i];
  }
  m_steps.clear();
  changePlayerState(C_PLAYER_IDLE_NO_DATA);
}

// _____________________________________________________________________
/*!
*  Choix du fichier contenant les échantillons
*  Analyse le fichier dont la syntaxe est : 
*    timestamp;nom_variable;valeur_variable
*  Un step c'est un pas de temps dans lequel plusieurs
*   variables peuvent être mise à jour en même temps
*   si elles ont le même timestamp
*  
*/
void CTracePlayer::setTraceFilename(QString trace_filename)
{
  QFileInfo fileInfo(trace_filename);
  if (fileInfo.exists() == false) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Fichier introuvable" + trace_filename);
    msgBox.exec();
    return; // pas la peine d'aller plus loin
  }

  m_trace_filename = trace_filename;
  stopPlayer();
  clearSteps();

  // Analyse le fichier (fichier au format .csv avec séparateur ";"
  QFile data(trace_filename);
  if (data.open(QFile::ReadOnly)) {
      QTextStream in(&data);
      QString line;
      in.readLine(); // ignore la première ligne du fichier qui explique le contneu du fichier
      long int old_timestamp = -1;
      while ((line = in.readLine()) != NULL) {
        QStringList split_line = line.split(";");
        if (split_line.size() != 3) { // format de fichier invalide
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setText("Ligne invalide: " + line);
            msgBox.exec();
            return; // pas la peine d'aller plus loin le fichier n'est pas conforme
        } // if le fichier n'est pas conforme

        double timestamp_sec = split_line.at(0).toDouble();
        long int timestamp  = (long int)(floor((timestamp_sec*1000)+0.5));  // dans le fichier, c'est en [sec], dans le code, on travail en [msec]
        QString var_name    = split_line.at(1);
        QVariant var_value  = split_line.at(2);
        tStep *pstep = NULL;

        if (timestamp != old_timestamp) { // cette donnée est sur un pas de temps différent du précédent
            pstep = new tStep();
            pstep->duration = 1000; // 1sec : met une valeur par défaut pour ne pas laisser le champ avec une valeur inconnue (valable pour le dernier pas de temps du fichier principalement)
            pstep->timestamp_sec = timestamp_sec;
            m_steps.append(pstep);
            
            // calcule la durée du pas de temps précédent en fonction du timestamp courant
            if (m_steps.size() != 1) { // si c'est le tout premier pas de temps, on ne peut pas encore calculer sa durée car il n'existe pas de pas de temps précédent
                long int duration = timestamp - old_timestamp; // traite le cas d'une erreure dans le fichier où le timestamp du pas suivant est inférieur au timestamp du pas précédent (le temps doit avancer)
                if (duration >= 0) {
                    m_steps.at(m_steps.size()-2)->duration = duration;
                }
            }
            old_timestamp = timestamp;
        }
        else {
            pstep = m_steps.at(m_steps.size() - 1);
        }
        // ici pstep pointe sur le step à mettre à jour
        pstep->variables_values.insert(var_name, var_value);
      } // pour toutes les lignes du fichier
  }
  // le player est prêt à jouer s'il contient quelquechose à jouer
  if (m_steps.size() != 0) {
    changePlayerState(C_PLAYER_STOP);
  }
}



// _____________________________________________________________________
/*!
*  Exécution du thread de génération des steps
*
*  Le step peut être modifié depuis 
*/
void CTracePlayer::run(void)
{
 changePlayerState(C_PLAYER_RUN);

 while(1) {  // boucle sur le nombre de périodes à réaliser
     if (m_cycle_number != C_PERIODIC_SIGNAL) {
       if (m_cycle_count >= m_cycle_number) {
           break;
       }
     }
     while(1) { // boucle sur chaque step
       m_step_index++;
       if (m_step_index >= m_steps.size()) { // sortie de boucle
            m_step_index = -1;
           break; 
       } 

       playStep(m_step_index);
       // la durée du step est soit la durée du fichier, soit une durée configurée par l'utilisateur
       // pour pouvoir rejouer une trace plus ou moins vite que la durée réelle
       int step_duration;
       if (m_common_step_duration == C_STEP_DURATION_IN_FILE) {
        step_duration = m_steps.at(m_step_index)->duration;
       }
       else {
        step_duration = m_common_step_duration;
       }
       msleep(step_duration);
     }
     m_cycle_count++;
 }
 changePlayerState(C_PLAYER_STOP);
}


// _____________________________________________________________________
/*!
*  Gère les états du player
*  
*/
void CTracePlayer::changePlayerState(tPlayerState new_state, bool enable_emit/*=true*/)
{
  // pas de changement par rapport au derner état -> rien à faire
  if (m_player_state == new_state) { return; }

  switch(new_state) {
    // __________________________
    case C_PLAYER_IDLE_NO_DATA:
       m_step_index = -1;
       m_cycle_count = 0;
    break;
    // __________________________
    case C_PLAYER_STOP:
       m_step_index = -1;
       m_cycle_count = 0;
       if (enable_emit) { emit playerFinished(m_player_name); }
    break;
    // __________________________
    case C_PLAYER_RUN:
        if (enable_emit) { emit playerStarted(m_player_name); }
    break;
    // __________________________
    case C_PLAYER_PAUSE:
        if (enable_emit) { emit playerPaused(m_player_name); }
    break;
    // __________________________
    case C_PLAYER_PAUSE_FIRST_STEP_INDEX:
       if (enable_emit) { emit playerPaused(m_player_name); }
    break;
    // __________________________
    case C_PLAYER_PAUSE_LAST_STEP_INDEX:
       if (enable_emit) { emit playerPaused(m_player_name); }
    break;
    // __________________________
    default:
        // ne rien faire : situation innatendue
    break;
  }
  m_player_state = new_state;
  if (enable_emit) { emit stateChanged(m_player_name); }
}

// _____________________________________________________________________
/*!
*  Joue un step donnée
*  Positionne l'ensemble des variables du step aux valeurs
*/
void CTracePlayer::playStep(int step_index)
{
 // protection contre les valeurs incohérentes d'index
 if ( (step_index < 0) || (step_index >= m_steps.size()) ) { 
     return; 
 } 

 tStep *pstep = m_steps.at(step_index);
 for (tVariableValue::iterator i=pstep->variables_values.begin(); i!=pstep->variables_values.end(); i++) {
    m_data_manager->write(i.key(), i.value());
    emit oneStepFinished(m_player_name);
 }
}

// _____________________________________________________________________
/*!
*  Commence (ou poursuit) une trace
*
*/
void CTracePlayer::startPlayer(void)
{
 if ( (m_player_state==C_PLAYER_STOP) || 
      (m_player_state==C_PLAYER_PAUSE) ||
      (m_player_state==C_PLAYER_PAUSE_FIRST_STEP_INDEX) ||
      (m_player_state==C_PLAYER_PAUSE_LAST_STEP_INDEX)) {
    start();
    // pas de changement du player state car il est fait en entrant dans le thread
 }
}


// _____________________________________________________________________
/*!
*  
*
*/
void CTracePlayer::stopPlayer(void)
{
 if ( (m_player_state==C_PLAYER_RUN) || 
      (m_player_state==C_PLAYER_PAUSE) ||
      (m_player_state==C_PLAYER_PAUSE_FIRST_STEP_INDEX) ||
      (m_player_state==C_PLAYER_PAUSE_LAST_STEP_INDEX)) { 
    terminate();
    changePlayerState(C_PLAYER_STOP);
 }
}


// _____________________________________________________________________
/*!
*  
*
*/
void CTracePlayer::pausePlayer(void)
{
 if (m_player_state==C_PLAYER_RUN) { 
     terminate();
 }
 
 // Laisse tous les compteurs m_step_index et m_cycle_count à leur valeur pour pouvoir reprendre 
 changePlayerState(C_PLAYER_PAUSE);
}

// _____________________________________________________________________
/*!
*  Joue le pas suivant (commande manuelle)
*
*/
void CTracePlayer::nextStep(void)
{
 pausePlayer(); // si le thread était en cours
 // calcul le prochain pas à jouer (le step suivant)
 if (m_step_index < (m_steps.size()-1)) { // arrivé à la fin, pas de rebouclage
    m_step_index++;
    playStep(m_step_index);
 }
 if (m_step_index >= (m_steps.size()-1)) {
    changePlayerState(C_PLAYER_PAUSE_LAST_STEP_INDEX);
 }
 else if (m_step_index <= 0) {
    changePlayerState(C_PLAYER_PAUSE_FIRST_STEP_INDEX);
 }
}
// _____________________________________________________________________
/*!
*  Joue le pas précédent (commande manuelle)
*
*/
void CTracePlayer::previousStep(void)
{
 pausePlayer(); // si le thread était en cours
 // calcul le prochain pas à jouer (le step précédent)
 if (m_step_index > 0) { // arrivé à la fin, pas de rebouclage
    m_step_index--;
    playStep(m_step_index);
 }
 if (m_step_index <= 0) {
    changePlayerState(C_PLAYER_PAUSE_FIRST_STEP_INDEX);
 }

}


// _____________________________________________________________________
/*!
*  
*
*/
void CTracePlayer::activeTrace(bool state)
{
  if (state) { startPlayer(); }
  else       { stopPlayer(); }
}


// _____________________________________________________________________
/*!
*  Point d'entrée pour repositionner l'index sur un step particulier
*   et exécuter ce step
*
*/
void CTracePlayer::selectAndPlayStep(int step_index)
{
 // protection contre les valeurs incohérentes d'index
 if ( (step_index < 0) || (step_index >= m_steps.size()) ) { 
     return; 
 } 
 m_step_index = step_index;
 playStep(step_index);
}






