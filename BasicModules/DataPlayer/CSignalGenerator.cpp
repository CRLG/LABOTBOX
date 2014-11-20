/*! \file CSignalGenerator.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include "CSignalGenerator.h"
#include "CDataManager.h"



/*! \addtogroup Data
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CSignalGenerator::CSignalGenerator(CDataManager *data_manager, QString var_name)
    :   m_data_manager(data_manager),
        m_var_name(var_name),
        m_signal_filename(""),
        m_common_samples_duration(100),
        m_cycle_number(1),
        m_cycle_count(0),
        m_generator_in_progress(false)
{

}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CSignalGenerator::~CSignalGenerator()
{
  m_samples.clear();
}


// _____________________________________________________________________
/*!
*  Choix du fichier contenant les �chantillons
*  Analys le fichier
*  2 types de formats de fichiers autoris�s :
*   - Fichier � une seule colonne repr�sentant la valeur des �chantillons
*   - Fichier � 2 colonnes :
*       > La 1�re colonne repr�sente la valeur de l'�chantillon
*       > La 2�me colonne repr�sente la dur�e de l'�chantillon en [msec]
*         Valeur particuli�re (-1) pour indiquer que c'est la dur�e commune qui doit �tre utilis�e
*/
void CSignalGenerator::setSignalFilename(QString signal_filename)
{
  QFileInfo fileInfo(signal_filename);
  if (fileInfo.exists() == false) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Fichier introuvable" + signal_filename);
    msgBox.exec();
    return; // pas la peine d'aller plus loin
  }

  bool generator_is_running = m_generator_in_progress;  // m�morisation de l'�tat cournat pour le restituer � la fin
  m_signal_filename = signal_filename;
  stop_generator();
  m_samples.clear();


  // Analyse le fichier (fichier au format .csv avec s�parateur ";"
  QFile data(signal_filename);
  if (data.open(QFile::ReadOnly)) {
      QTextStream in(&data);
      QString line;
      in.readLine(); // ignore la premi�re ligne du fichier qui explique le contneu du fichier
      while ((line = in.readLine()) != NULL) {
        QStringList split_line = line.split(";");
        tSample sample = { "", C_COMMON_DURATION };
        sample.value = split_line.at(0);
        if (split_line.size() == 2) {  // fichier � 2 colonnes
            sample.duration = split_line.at(1).toULong();
        }
        m_samples.append(sample);
      }
  }

 // si le g�n�rateur tournait, le relance
 if (generator_is_running) {
     start_generator();
 }

}



// _____________________________________________________________________
/*!
*  Ex�cution du threadd de g�n�ration du signal
*
*/
void CSignalGenerator::run(void)
{
 m_generator_in_progress = true;
 emit signalStarted(m_var_name);
 m_cycle_count = 0;
 while(1) {
     if (m_cycle_number != C_PERIODIC_SIGNAL) {
       if (m_cycle_count >= m_cycle_number) {
           break;
       }
     }
     for (int i=0; i<m_samples.size(); i++) {
       m_data_manager->write(m_var_name, m_samples[i].value);
       msleep((m_samples[i].duration!=C_COMMON_DURATION)?m_samples[i].duration:m_common_samples_duration);
     }
     m_cycle_count++;
 }
 emit signalFinished(m_var_name);
 m_generator_in_progress = false;
}



void CSignalGenerator::start_generator(void)
{
 if (m_generator_in_progress == true) { return; }
 start();
}

void CSignalGenerator::stop_generator(void)
{
 if (m_generator_in_progress == false) { return; }
 terminate();
 m_generator_in_progress = false;
 emit signalFinished(m_var_name);
}

void CSignalGenerator::activeInhibeGenerator(bool state)
{
  if (state) { start_generator(); }
  else       { stop_generator(); }
}




