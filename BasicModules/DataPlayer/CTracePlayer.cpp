/*! \file CTracePlayer.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include "CTracePlayer.h"
#include "CDataManager.h"
#include "csvParser.h"

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
  m_trace_filename.clear();
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
bool CTracePlayer::setTraceFilename(QString trace_filename, tFileFormat fileformat)
{
  QFileInfo fileInfo(trace_filename);
  if (fileInfo.exists() == false) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Fichier introuvable" + trace_filename);
    msgBox.exec();
    return false; // pas la peine d'aller plus loin
  }

  stopPlayer();
  clearSteps();

  if (fileformat == C_AUTOTECT_FORMAT) fileformat = getFormatFromFile(trace_filename);

  bool status=false;
  // Aiguillage en fonction du format d'entrée attendu
  switch(fileformat) {
    case C_TRACE_FORMAT :               status = setInputFile_TraceFormat(trace_filename);               break;
    case C_CSV_DURATION_TIME_FORMAT :   status = setInputFile_csvTimeFormat(trace_filename, false/*time_is_timestamp=false*/);      break;
    case C_CSV_ABSOLUTE_TIME_FORMAT :   status = setInputFile_csvTimeFormat(trace_filename, true/*time_is_timestamp=true*/);       break;
    default :                           status = false; break;
  }

  if (status) m_trace_filename = trace_filename; // ne conserve le nom du fichier d'entrée que si le format est valide

  // le player est prêt à jouer s'il contient quelquechose à jouer
  if (m_steps.size() != 0) {
    changePlayerState(C_PLAYER_STOP);
  }
  return status;
}


// _____________________________________________________________________
/*!
*  Format d'entrée de type "Trace"
* Ce format est celui enregistré par le module DataView
*
*
*  Le step peut être modifié depuis
*/
bool CTracePlayer::setInputFile_TraceFormat(QString trace_filename)
{
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
              return false; // pas la peine d'aller plus loin le fichier n'est pas conforme
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

    return true;
}


// _____________________________________________________________________
/*!
 * \brief Interprète le fichier d'entrée au format "csv duration" et crée une image en mémoire
 * 4 variantes sont possibles
 *    - Le temps en 1ère colonne exprime la durée du step ("Duration")
 *          * en [sec] ou
 *          * en [msec]
 *    - Le temps en 1ère colonne exprime un temps ("Timestamp")
 *          * en [sec] ou
 *          * en [msec]
 * Le format du fichier est :
 * <Time> [<unité>];Nom data1;Nom data2;Nom data ...
 * duree;valeur data1; valeur data2; valeur ...
 * duree;valeur data1; valeur data2; valeur ...
 * ...
 * <Time> exprime le type de temps "Duration" ou "Timestamp"
 * [<unité>] peut être indiqué [msec] ou [sec]
 * Sans indication contraire, les informations sont en [msec]
 *
 * La première ligne contient le nom des données
 * \param trace_filename le nom du fichier d'entrée (avec chemin complet)
 */
bool CTracePlayer::setInputFile_csvTimeFormat(QString trace_filename, bool time_is_timestamp)
{
    csvData data;
    csvParser parser;
    QStringList error_msg_lst, warn_msg_lst;

    parser.enableEmptyCells(false);
    parser.setSeparator(";");
    parser.setMinimumNumberOfExpectedColums(2);
    bool status = parser.parse(trace_filename, data, &error_msg_lst, &warn_msg_lst);
    if (!status) {
        if (error_msg_lst.size() > 0) {
            QString global_err_msg;
            foreach (QString msg, error_msg_lst) global_err_msg += "\n" + msg; // regroupe tous les messages en une seule chaine à afficher
            QMessageBox::critical(0, QFileInfo(trace_filename).fileName(), global_err_msg);
        }
        return false; // pas la peine d'aller plus loin, le fichier d'entrée n'est pas valide
    }

    // Il y a des warnings dans le parsing du fichier d'entrée
    if (warn_msg_lst.size() > 0) {
        QString global_warn_msg;
        foreach (QString msg, warn_msg_lst) global_warn_msg += "\n" + msg; // regroupe tous les messages en une seule chaine à afficher
        QMessageBox::warning(0, QFileInfo(trace_filename).fileName(), global_warn_msg);
    }

    bool time_in_sec = false;   // par défaut, les durées sont des entiers en [msec], sauf mention contraire dans nom de la colonne s'il est indiqué [sec]
    if (data.m_header.at(0).simplified().contains("[sec]")) time_in_sec = true;

    for (int line=0; line<data.m_datas.size(); line++) {
        tStep *pstep;
        pstep = new tStep();
        QStringList data_line = data.m_datas.at(line);

        // Calcule la durée du step (plusieurs cas de figure différent)
        // 1. la colonne de temps exprime la durée du step : la durée du step est directement la valeur de la cellule (en tenant compte de l'unité [sec] ou [msec])
        if (time_is_timestamp == false) {
            if (time_in_sec)    pstep->duration = time_second_to_msec( data_line.at(0).toDouble() );
            else                pstep->duration = data_line.at(0).toInt();
        }
        // 2. la colonne de temps exprime un timestamp : la durée du step est la différence avec le timestamp du step suivant
        else {
            bool line_is_last_dataline = (line == (data.m_datas.size() - 1));
            if (line_is_last_dataline) pstep->duration = 1000; // 1sec par défaut pour le dernier step
            else {
                if (time_in_sec) {
                    double current_timestamp = data_line.at(0).toDouble();
                    double next_timestamp = data.m_datas.at(line+1).at(0).toDouble();
                    pstep->duration = time_second_to_msec( next_timestamp - current_timestamp );
                }
                else {
                    int current_timestamp = data_line.at(0).toInt();
                    int next_timestamp = data.m_datas.at(line+1).at(0).toInt();
                    pstep->duration = next_timestamp - current_timestamp;
                }
            }
        }

        for (int column=1; column<data_line.size(); column++) { // parcours toutes les données
            QString varname = data.m_header.at(column).trimmed(); // nom de la colonne = nom de la data
            QVariant varvalue = data_line.at(column);
            pstep->variables_values.insert(varname, varvalue);
        }
        m_steps.append(pstep);
    }

    return true;
}

// _____________________________________________________________________
/*!
 * \brief Analyse le fichier d'entrée et en détermine le format
 * \param trace_filename le nom du fichier d'entrée
 * \return le format du fichier détecté ou UNKNOWN_FILEFORMAT si le format n'est pas reconnu
 */
CTracePlayer::tFileFormat CTracePlayer::getFormatFromFile(QString trace_filename)
{
    // Analyse le fichier (fichier au format .csv avec séparateur ";"
    QFile data(trace_filename);
    if (data.open(QFile::ReadOnly)) {
        QTextStream in(&data);
        QString line;
        line = in.readLine().toLower();
        QStringList split_line = line.split(";");
        for (int i=0; i<split_line.size(); i++) split_line[i] = split_line[i].trimmed(); // nettoyage des champs
        // Test n°1
        if (split_line.size() == 3) {
            if (split_line[0].contains("temps") &&
                (split_line[1].contains("nom") || split_line[1].contains("variable")) &&
                split_line[2].contains("valeur")) {
                return C_TRACE_FORMAT;
            }
        }
        // Test n°2
        if (split_line.size() >= 2) {
            if (split_line[0].contains("timestamp")) {
                return C_CSV_ABSOLUTE_TIME_FORMAT;
            }
        }
        // Test n°3
        if (split_line.size() >= 2) {
            if (split_line[0].contains("duration")) {
                return C_CSV_DURATION_TIME_FORMAT;
            }
        }
        data.close();
    }
    return C_UNKNOWN_FILEFORMAT;
}

// _____________________________________________________________________
/*!
 * \brief Convertit un temps de seconde en milliseconde
 * \param time_sec le temps en secondes
 * \return le temps convertit en milliseconde
 */
long int CTracePlayer::time_second_to_msec(double time_sec)
{
    return (long int)(floor((time_sec*1000.)+0.5)); // [sec] -> [msec]
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






