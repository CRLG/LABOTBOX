/*! \file CDataView.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>

#include "CDataView.h"
#include "CLaBotBox.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"



/*! \addtogroup Module_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CDataView::CDataView(const char *plugin_name)
    :CBasicModule(plugin_name, VERSION_DataView, AUTEUR_DataView, INFO_DataView)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CDataView::~CDataView()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CDataView::init(CLaBotBox *application)
{
  CModule::init(application);
  setGUI(&m_ihm); // indique � la classe de base l'IHM

  // G�re les actions sur clic droit sur le panel graphique du module
  m_ihm.setContextMenuPolicy(Qt::CustomContextMenu);
  connect(&m_ihm, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRightClicGUI(QPoint)));

  // Restore la taille de la fen�tre
  QVariant val;
  val = m_application->m_eeprom->read(getName(), "geometry", QRect(50, 50, 150, 150));
  m_ihm.setGeometry(val.toRect());
  // Restore le fait que la fen�tre est visible ou non
  val = m_application->m_eeprom->read(getName(), "visible", QVariant(true));
  if (val.toBool()) { m_ihm.show(); }
  else              { m_ihm.hide(); }
  // Restore le niveau d'affichage
  val = m_application->m_eeprom->read(getName(), "niveau_trace", QVariant(MSG_TOUS));
  setNiveauTrace(val.toUInt());
  // Restore la couleur de fond
  val = m_application->m_eeprom->read(getName(), "background_color", QVariant(DEFAULT_MODULE_COLOR));
  setBackgroundColor(val.value<QColor>());

  // le bouton refresh met � jour la liste des variables disponibles
  connect(m_ihm.ui.PB_refresh_liste, SIGNAL(clicked()), this, SLOT(refreshListeVariables()));
  connect(m_ihm.ui.active_trace, SIGNAL(clicked(bool)), this, SLOT(activeInspector(bool)));
  connect(m_ihm.ui.pb_up, SIGNAL(clicked()), this, SLOT(upVariable()));
  connect(m_ihm.ui.pb_down, SIGNAL(clicked()), this, SLOT(downVariable()));
  connect(m_ihm.ui.pb_effacer_liste, SIGNAL(clicked()), this, SLOT(effacerListeVariablesValues()));

  connect(&m_timer_lecture_variables, SIGNAL(timeout()), this, SLOT(refreshValeursVariables()));

  connect(m_ihm.ui.valueToWrite, SIGNAL(returnPressed()), this, SLOT(editingFinishedWriteValueInstantane()));
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CDataView::close(void)
{
  // M�morise en EEPROM l'�tat de la fen�tre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));

  // m�morise en EEPROM la liste des variables sous surveillance
  QStringList liste_variables_observees;
  for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
    if (m_ihm.ui.liste_variables->item(i)->checkState() == Qt::Checked) {
        liste_variables_observees.append(m_ihm.ui.liste_variables->item(i)->text());
    }
  } // for toutes les variables sous surveillances
  m_application->m_eeprom->write(getName(), "liste_variables_observees", QVariant(liste_variables_observees));
}

// _____________________________________________________________________
/*!
*  Cr�ation des menus sur clic droit sur la fen�tre du module
*
*/
void CDataView::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}


// _____________________________________________________________________
/*!
*  Point d'entr�e lorsqu'une variable a �t� modifi�e
*
*/
void CDataView::variableChanged(QVariant val)
{
 CData *data = qobject_cast<CData*>(sender()); // r�cup�re l'objet �metteur du signal (pour pouvoir r�cup�rer son nom)
 if (data) {
    m_application->m_print_view->print_debug(this, "Variable modifiee: " + data->getName() + "=" + val.toString() + "(" +  data->read().toString() + ")");
    addTraceVariable(data->getName(), data->read().toString(), data->getTime());
 }
 else {
    m_application->m_print_view->print_error(this, QString(__FUNCTION__) + ": Erreur sur la r�cup�ration de l'objet a l'origine du signal");
 }
}

// _____________________________________________________________________
/*!
*  Point d'entr�e lorsque la fen�tre est appel�e
*
*/
void CDataView::setVisible(void)
{
  CModule::setVisible();
  refreshListeVariables();
}


// _____________________________________________________________________
/*!
*  Met � jour la liste des variables dans la fen�tre de gauche
*
*/
void CDataView::refreshListeVariables(void)
{
  QStringList var_list;
  m_application->m_data_center->getListeVariablesName(var_list);

  m_ihm.ui.liste_variables->clear();
  m_ihm.ui.liste_variables->addItems(var_list);
  for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
      m_ihm.ui.liste_variables->item(i)->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled);
      m_ihm.ui.liste_variables->item(i)->setCheckState(Qt::Unchecked);
  }
}


// _____________________________________________________________________
/*!
*  Point  d'entr�e lorsque la checkbox d'activation est actionn�e
*
*/
void CDataView::activeInspector(bool state)
{
  if (state == true) { // activation du suivi des variables
    m_start_time = QDateTime::currentMSecsSinceEpoch();
    m_ihm.ui.table_variables_valeurs->clearContents();
    m_ihm.ui.table_variables_valeurs->setRowCount(0);
    if (m_ihm.ui.choix_mode_inspection->currentText()=="Temporel") {
        activeInspectorTemporel();
    }
    else {
        activeInspectorInstantane();
    }
  }
  else { // d�sactivation du suivi des variables
    if (m_ihm.ui.choix_mode_inspection->currentText()=="Temporel") {
        finInspectorTemporel();
    }
    else {
        finInspectorInstantane();
    }
  }

  // v�rouille les objets pendant l'enregistrement
  m_ihm.ui.choix_mode_inspection->setEnabled(!state);
  m_ihm.ui.liste_variables->setEnabled(!state);
  m_ihm.ui.liste_variables->setEnabled(!state);
  m_ihm.ui.PB_refresh_liste->setEnabled(!state);
}


// _____________________________________________________________________
/*!
*  Efface la liste des variables
*
*/
void CDataView::effacerListeVariablesValues(void)
{
 m_ihm.ui.table_variables_valeurs->clearContents();
 m_ihm.ui.table_variables_valeurs->setRowCount(0);
}

// ===========================================================================
//              GESTION DU MODE "TEMPOREL"
// ===========================================================================


// _____________________________________________________________________
/*!
*  Actions � effecter lors de l'activation de l'enregistrement des data en mode "Temporel"
*
*  Cr�e une connexion entre toutes les variables � surveiller et l'affichage de la valeur
*/
void CDataView::activeInspectorTemporel(void)
{
  m_ihm.ui.pb_up->setEnabled(false);
  m_ihm.ui.pb_down->setEnabled(false);

  // r�alise la connexion entre chaque variable coch�e et le slot d'affichage
  connectDiscconnectVariablesTemporel(1);
}


// _____________________________________________________________________
/*!
*  Actions � effecter lors de l'activation de l'enregistrement des data en mode "Temporel"
*
*  Cr�e une connexion entre toutes les variables � surveiller et l'affichage de la valeur
*/
void CDataView::finInspectorTemporel(void)
{
  // r�alise la deconnexion entre chaque variable coch�e et le slot d'affichage
  connectDiscconnectVariablesTemporel(0);
}



// _____________________________________________________________________
/*!
*  Actions � effecter lors de l'activation de l'enregistrement des data en mode "Temporel"
*
*  Cr�e une connexion entre toutes les variables � surveiller et l'affichage de la valeur
*  choix : 0=deconnexion / 1=connexion
*/
void CDataView::connectDiscconnectVariablesTemporel(bool choix)
{
  // Recopie toutes les variables � surveiller
  for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
    if (m_ihm.ui.liste_variables->item(i)->checkState() == Qt::Checked) {
        CData *_data = m_application->m_data_center->getData(m_ihm.ui.liste_variables->item(i)->text());
        if (choix) {
            connect(_data,
                  SIGNAL(valueChanged(QVariant)),
                  this,
                  SLOT(variableChanged(QVariant))
                  );
        }
        else {
            disconnect(_data,
                  SIGNAL(valueChanged(QVariant)),
                  this,
                  SLOT(variableChanged(QVariant))
                  );
        }
    } // if la variable est � surveiller
  }
}

// _____________________________________________________________________
/*!
*  Normalise le temps par rapport � l'instant du d�but de la trace
*   name = ms_epoch : temps � normaliser (nombre de msec depuis 1970)
*   Renvoie une valeur en secondes
*/
double CDataView::normaliseTemps(qint64 ms_epoch)
{
    return (ms_epoch-m_start_time)/1000.0;
}


// _____________________________________________________________________
/*!
*  Ajoute une ligne � la liste des valeurs de variables
*   name = nom de la variable
*   value = valeur de la variable
*
*/
void CDataView::addTraceVariable(QString name, QString value, quint64 time)
{
 unsigned int index = m_ihm.ui.table_variables_valeurs->rowCount();
 m_ihm.ui.table_variables_valeurs->insertRow(index);
 m_ihm.ui.table_variables_valeurs->setItem(index, Cihm_DataView::COL_TEMPS, new QTableWidgetItem());
 m_ihm.ui.table_variables_valeurs->setItem(index, Cihm_DataView::COL_NOM, new QTableWidgetItem());
 m_ihm.ui.table_variables_valeurs->setItem(index, Cihm_DataView::COL_VALEUR, new QTableWidgetItem());
 m_ihm.ui.table_variables_valeurs->item(index, Cihm_DataView::COL_TEMPS)->setText(QString::number(normaliseTemps(time)));
 m_ihm.ui.table_variables_valeurs->item(index, Cihm_DataView::COL_NOM)->setText(name);
 m_ihm.ui.table_variables_valeurs->item(index, Cihm_DataView::COL_VALEUR)->setText(value);

 m_ihm.ui.table_variables_valeurs->item(index, Cihm_DataView::COL_TEMPS)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
 m_ihm.ui.table_variables_valeurs->item(index, Cihm_DataView::COL_NOM)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
 m_ihm.ui.table_variables_valeurs->item(index, Cihm_DataView::COL_VALEUR)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
}


// ===========================================================================
//              GESTION DU MODE "INSTANTANE"
// ===========================================================================

// _____________________________________________________________________
/*!
*  Actions � effecter lors de l'activation de l'enregistrement des data en mode "Instantane"
*
*/
void CDataView::activeInspectorInstantane(void)
{
  m_ihm.ui.pb_effacer_liste->setEnabled(false);
  m_ihm.ui.pb_up->setEnabled(true);
  m_ihm.ui.pb_down->setEnabled(true);
  m_ihm.ui.valueToWrite->setEnabled(true);
  m_ihm.ui.valueToWrite_label->setEnabled(true);

  // Recopie toutes les variables � surveiller
  for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
    if (m_ihm.ui.liste_variables->item(i)->checkState() == Qt::Checked) {
        addTraceVariable(m_ihm.ui.liste_variables->item(i)->text());
    }
  }

  // Lance le timer qui lit p�riodiquement les variables
  m_timer_lecture_variables.start(PERIODE_ECHANTILLONNAGE_VARIABLES); // refresh toutes les 100msec

}


// _____________________________________________________________________
/*!
*  Actions � effecter lors de l'activation de l'enregistrement des data en mode "Instantane"
*
*/
void CDataView::finInspectorInstantane(void)
{
  // Arr�te le timer de mise � jour des variables si l'enregistrement est stopp�
  m_timer_lecture_variables.stop();

  m_ihm.ui.pb_effacer_liste->setEnabled(true);

  m_ihm.ui.valueToWrite->setEnabled(false);
  m_ihm.ui.valueToWrite_label->setEnabled(false);
  
 }

// _____________________________________________________________________
/*!
*  Met � jour la valeur de chaque variable contenu dans la liste des variables � surveiller
*
*/
void CDataView::refreshValeursVariables(void)
{
  for (int i=0; i<m_ihm.ui.table_variables_valeurs->rowCount(); i++) {
    QString var_name = m_ihm.ui.table_variables_valeurs->item(i, Cihm_DataView::COL_NOM)->text();
    CData *data = m_application->m_data_center->getData(var_name);
    if (data)
    {
        m_ihm.ui.table_variables_valeurs->item(i, Cihm_DataView::COL_VALEUR)->setText(data->read().toString());
        double datatime = normaliseTemps(data->getTime());
        m_ihm.ui.table_variables_valeurs->item(i, Cihm_DataView::COL_TEMPS)->setText(QString::number(datatime));
    }
    else
    {
        m_application->m_print_view->print_error(this, QString(__FUNCTION__) + ": Erreur sur la r�cup�ration de la data " + var_name);
    }
  }
}

// _____________________________________________________________________
/*!
*  Remonte la variable dans la liste
*
*/
void CDataView::upVariable()
{
  // Identifie quelle variable est concern�e
  int index = m_ihm.ui.table_variables_valeurs->currentRow();
  m_application->m_print_view->print_debug(this, "index a deplacer = " + QString::number(index));
  if (index <= 0) { return; } // valeur invalide ou premier �l�ment de la liste

  // Arr�te le timer le temps de la manipulation
  bool timer_active = false;
  if (m_timer_lecture_variables.isActive()) {
    timer_active = true;
    m_timer_lecture_variables.stop();
  }

  // D�place la variable
  QString var_name = m_ihm.ui.table_variables_valeurs->item(index, Cihm_DataView::COL_NOM)->text();
  m_ihm.ui.table_variables_valeurs->removeRow(index);
  int new_row = index - 1;
  m_ihm.ui.table_variables_valeurs->insertRow(new_row);
  m_ihm.ui.table_variables_valeurs->setItem(new_row, Cihm_DataView::COL_NOM, new QTableWidgetItem());
  m_ihm.ui.table_variables_valeurs->setItem(new_row, Cihm_DataView::COL_VALEUR, new QTableWidgetItem());
  m_ihm.ui.table_variables_valeurs->setItem(new_row, Cihm_DataView::COL_TEMPS, new QTableWidgetItem());
  m_ihm.ui.table_variables_valeurs->item(new_row, Cihm_DataView::COL_NOM)->setText(var_name);
  m_ihm.ui.table_variables_valeurs->selectRow(new_row);

 // Rallume le timer
 if (timer_active) {
    m_timer_lecture_variables.start(PERIODE_ECHANTILLONNAGE_VARIABLES);
 }
}
// _____________________________________________________________________
/*!
*  Redescend la variable dans la liste
*
*/
void CDataView::downVariable()
{
  // Identifie quelle variable est concern�e
  int index = m_ihm.ui.table_variables_valeurs->currentRow();
  m_application->m_print_view->print_debug(this, "index a deplacer = " + QString::number(index));
  if (index < 0) { return; } // valeur invalide
  if (index >= (m_ihm.ui.table_variables_valeurs->rowCount()-1)) { return; } // valeur invalide ou dernier �l�ment de la liste

  // Arr�te le timer, le temps de la manipulation
  bool timer_active = false;
  if (m_timer_lecture_variables.isActive()) {
    timer_active = true;
    m_timer_lecture_variables.stop();
  }

  // D�place la variable
  QString var_name = m_ihm.ui.table_variables_valeurs->item(index, Cihm_DataView::COL_NOM)->text();
  m_ihm.ui.table_variables_valeurs->removeRow(index);
  int new_row = index + 1;
  m_ihm.ui.table_variables_valeurs->insertRow(new_row);
  m_ihm.ui.table_variables_valeurs->setItem(new_row, Cihm_DataView::COL_NOM, new QTableWidgetItem());
  m_ihm.ui.table_variables_valeurs->setItem(new_row, Cihm_DataView::COL_VALEUR, new QTableWidgetItem());
  m_ihm.ui.table_variables_valeurs->setItem(new_row, Cihm_DataView::COL_TEMPS, new QTableWidgetItem());
  m_ihm.ui.table_variables_valeurs->item(new_row, Cihm_DataView::COL_NOM)->setText(var_name);
  m_ihm.ui.table_variables_valeurs->selectRow(new_row);

 // Rallume le timer
 if (timer_active) {
    m_timer_lecture_variables.start(PERIODE_ECHANTILLONNAGE_VARIABLES);
 }
}


// _____________________________________________________________________
/*!
*  Slot appel� lorsque l'utilisateur termine la saisie d'une valeur � �crire
*  Chaque �l�ment s�lectionn� dans la liste se voit affect� de la nouvelle valeur
*/
void CDataView::editingFinishedWriteValueInstantane(void)
{
 QVariant valueToWrite = m_ihm.ui.valueToWrite->text();
 unsigned long nbre_variables_ecrites = 0;

  for (int i=0; i<m_ihm.ui.table_variables_valeurs->rowCount(); i++) {
      if (m_ihm.ui.table_variables_valeurs->item(i, Cihm_DataView::COL_VALEUR)->isSelected()) {
            QString var_name = m_ihm.ui.table_variables_valeurs->item(i, Cihm_DataView::COL_NOM)->text();
            m_application->m_data_center->write(var_name, valueToWrite);
            nbre_variables_ecrites++;
    } // if l'�l�ment est s�lectionn�
  }

  // si aucune variable n'a �t� �crite, pr�ciser � l'utilisateur pourquoi
  if (nbre_variables_ecrites == 0) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("S�lectionner une ou plusieurs donn�es � �crire");
    msgBox.exec();
  }
}
