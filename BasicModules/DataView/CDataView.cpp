/*! \file CDataView.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>

#include "CDataView.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CToolBox.h"
#include "CDataListOpenSave.h"



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
void CDataView::init(CApplication *application)
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

  // le bouton refresh met à jour la liste des variables disponibles
  connect(m_ihm.ui.PB_refresh_liste, SIGNAL(clicked()), this, SLOT(refreshListeVariables()));
  connect(m_application->m_data_center, SIGNAL(dataCreated(CData*)), this, SLOT(refreshListeVariables()));
  connect(m_ihm.ui.active_trace, SIGNAL(clicked(bool)), this, SLOT(activeInspector(bool)));
  connect(m_ihm.ui.pb_up, SIGNAL(clicked()), this, SLOT(upVariable()));
  connect(m_ihm.ui.pb_down, SIGNAL(clicked()), this, SLOT(downVariable()));
  connect(m_ihm.ui.pb_effacer_liste, SIGNAL(clicked()), this, SLOT(effacerListeVariablesValues()));

  connect(&m_timer_lecture_variables, SIGNAL(timeout()), this, SLOT(refreshValeursVariables()));

  connect(m_ihm.ui.valueToWrite, SIGNAL(returnPressed()), this, SLOT(editingFinishedWriteValueInstantane()));
  connect(m_ihm.ui.pb_enregistrer_trace, SIGNAL(clicked()), this, SLOT(saveToFile()));
  connect(m_ihm.ui.filtre_noms_variables, SIGNAL(textChanged(QString)), this, SLOT(onDataFilterChanged(QString)));
  connect(m_ihm.ui.pb_open_variables_liste, SIGNAL(clicked(bool)), this, SLOT(loadListeVariablesObservees()));
  connect(m_ihm.ui.pb_save_variables_liste, SIGNAL(clicked(bool)), this, SLOT(saveListeVariablesObservees()));
  connect(m_ihm.ui.pb_decoche_tout, SIGNAL(clicked(bool)), this, SLOT(decocheToutesVariables()));
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CDataView::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));

  // mémorise en EEPROM la liste des variables sous surveillance (cochées sur l'IHM)
  m_application->m_eeprom->write(getName(), "datalist_logged", QVariant(getListeVariablesObservees()));
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
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
*  Point d'entrée lorsqu'une variable a été modifiée
*
*/
void CDataView::variableChangedUpdated(QVariant val, quint64 update_time)
{
 CData *data = qobject_cast<CData*>(sender()); // récupère l'objet émetteur du signal (pour pouvoir récupérer son nom)
 if (data) {
    m_application->m_print_view->print_debug(this, "Variable modifiee: " + data->getName() + "=" + val.toString() + "(" +  data->read().toString() + ")");
    addTraceVariable(data->getName(), val.toString(), update_time);
 }
 else {
    m_application->m_print_view->print_error(this, QString(__FUNCTION__) + ": Erreur sur la récupération de l'objet a l'origine du signal");
 }
}

// _____________________________________________________________________
/*!
*  Point d'entrée lorsque la fenêtre est appelée
*
*/
void CDataView::setVisible(void)
{
  CModule::setVisible();
  refreshListeVariables();
}


// _____________________________________________________________________
/*!
*  Met à jour la liste des variables dans la fenêtre de gauche
*
*/
void CDataView::refreshListeVariables(void)
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
void CDataView::addVariablesObserver(QStringList liste_variables)
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
*  Récupère la liste des variables à observer
*/
QStringList CDataView::getListeVariablesObservees()
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
*  Décoche toutes les variables
*/
void CDataView::decocheToutesVariables()
{
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        if (!m_ihm.ui.liste_variables->item(i)->isHidden()) {
            m_ihm.ui.liste_variables->item(i)->setCheckState(Qt::Unchecked);
        }
    }
}

// _____________________________________________________________________
/*!
*  Charge une liste de variables à observer
*/
void CDataView::loadListeVariablesObservees(void)
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
void CDataView::saveListeVariablesObservees(void)
{
    QString fileName = QFileDialog::getSaveFileName(&m_ihm, tr("Save File"), ".");
    if (fileName.isEmpty()) return;

    QStringList liste_variables = getListeVariablesObservees();
    CDataListOpenSave::save(fileName, liste_variables);
}


// _____________________________________________________________________
/*!
*  Point  d'entrée lorsque la checkbox d'activation est actionnée
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
  else { // désactivation du suivi des variables
    if (m_ihm.ui.choix_mode_inspection->currentText()=="Temporel") {
        finInspectorTemporel();
    }
    else {
        finInspectorInstantane();
    }
  }

  // vérouille les objets pendant l'enregistrement
  m_ihm.ui.choix_mode_inspection->setEnabled(!state);
  m_ihm.ui.liste_variables->setEnabled(!state);
  m_ihm.ui.liste_variables->setEnabled(!state);
  m_ihm.ui.PB_refresh_liste->setEnabled(!state);
  m_ihm.ui.affiche_diff_seulement->setEnabled(!state);
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
*  Actions à effecter lors de l'activation de l'enregistrement des data en mode "Temporel"
*
*  Crée une connexion entre toutes les variables à surveiller et l'affichage de la valeur
*/
void CDataView::activeInspectorTemporel(void)
{
  m_ihm.ui.pb_up->setEnabled(false);
  m_ihm.ui.pb_down->setEnabled(false);

  // réalise la connexion entre chaque variable cochée et le slot d'affichage
  connectDiscconnectVariablesTemporel(1, m_ihm.ui.affiche_diff_seulement->isChecked());
}


// _____________________________________________________________________
/*!
*  Actions à effecter lors de l'activation de l'enregistrement des data en mode "Temporel"
*
*  Crée une connexion entre toutes les variables à surveiller et l'affichage de la valeur
*/
void CDataView::finInspectorTemporel(void)
{
  // réalise la deconnexion entre chaque variable cochée et le slot d'affichage
  connectDiscconnectVariablesTemporel(0, m_ihm.ui.affiche_diff_seulement->isChecked());
}



// _____________________________________________________________________
/*!
*  Actions à effecter lors de l'activation de l'enregistrement des data en mode "Temporel"
*
*  Crée une connexion entre toutes les variables à surveiller et l'affichage de la valeur
*  choix : 0=deconnexion / 1=connexion
*  only_diff : n'affiche que si la variable a changé de valeur
*/
void CDataView::connectDiscconnectVariablesTemporel(bool choix, bool only_diff)
{
  // Recopie toutes les variables à surveiller
  for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
    if (m_ihm.ui.liste_variables->item(i)->checkState() == Qt::Checked) {
        CData *_data = m_application->m_data_center->getData(m_ihm.ui.liste_variables->item(i)->text());
        if (choix) {
            if (only_diff) {  // exploite un signal différent en fonction du fait de filtrer sur les différences
                connect(_data,
                      SIGNAL(valueChanged(QVariant, quint64)),
                      this,
                      SLOT(variableChangedUpdated(QVariant, quint64))
                      );
            }
            else {
                connect(_data,
                      SIGNAL(valueUpdated(QVariant, quint64)),
                      this,
                      SLOT(variableChangedUpdated(QVariant, quint64))
                      );
            }
        }
        else {
            if (only_diff) {  // exploite un signal différent en fonction du fait de filtrer sur les différences
                disconnect(_data,
                      SIGNAL(valueChanged(QVariant, quint64)),
                      this,
                      SLOT(variableChangedUpdated(QVariant, quint64))
                      );
            }
            else {
                disconnect(_data,
                      SIGNAL(valueUpdated(QVariant, quint64)),
                      this,
                      SLOT(variableChangedUpdated(QVariant, quint64))
                      );
            }
        }
    } // if la variable est à surveiller
  }
}

// _____________________________________________________________________
/*!
*  Normalise le temps par rapport à l'instant du début de la trace
*   name = ms_epoch : temps à normaliser (nombre de msec depuis 1970)
*   Renvoie une valeur en secondes
*/
double CDataView::normaliseTemps(qint64 ms_epoch)
{
    return (ms_epoch-m_start_time)/1000.0;
}


// _____________________________________________________________________
/*!
*  Ajoute une ligne à la liste des valeurs de variables
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
*  Actions à effecter lors de l'activation de l'enregistrement des data en mode "Instantane"
*
*/
void CDataView::activeInspectorInstantane(void)
{
  m_ihm.ui.pb_effacer_liste->setEnabled(false);
  m_ihm.ui.pb_up->setEnabled(true);
  m_ihm.ui.pb_down->setEnabled(true);
  m_ihm.ui.valueToWrite->setEnabled(true);
  m_ihm.ui.valueToWrite_label->setEnabled(true);

  // Recopie toutes les variables à surveiller
  for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
    if (m_ihm.ui.liste_variables->item(i)->checkState() == Qt::Checked) {
        addTraceVariable(m_ihm.ui.liste_variables->item(i)->text());
    }
  }

  // Lance le timer qui lit périodiquement les variables
  m_timer_lecture_variables.start(PERIODE_ECHANTILLONNAGE_VARIABLES); // refresh toutes les 100msec

}


// _____________________________________________________________________
/*!
*  Actions à effecter lors de l'activation de l'enregistrement des data en mode "Instantane"
*
*/
void CDataView::finInspectorInstantane(void)
{
  // Arrête le timer de mise à jour des variables si l'enregistrement est stoppé
  m_timer_lecture_variables.stop();

  m_ihm.ui.pb_effacer_liste->setEnabled(true);

  m_ihm.ui.valueToWrite->setEnabled(false);
  m_ihm.ui.valueToWrite_label->setEnabled(false);
  
 }

// _____________________________________________________________________
/*!
*  Met à jour la valeur de chaque variable contenu dans la liste des variables à surveiller
*
*/
void CDataView::refreshValeursVariables(void)
{
  for (int i=0; i<m_ihm.ui.table_variables_valeurs->rowCount(); i++) {
    QString var_name = m_ihm.ui.table_variables_valeurs->item(i, Cihm_DataView::COL_NOM)->text();
    CData *data = m_application->m_data_center->getData(var_name);
    if (data)
    {
        QString actual_val = m_ihm.ui.table_variables_valeurs->item(i, Cihm_DataView::COL_VALEUR)->text();
        QString new_val = data->read().toString();
        if ( ( m_ihm.ui.affiche_diff_seulement->isChecked() && (actual_val != new_val) )
            || !m_ihm.ui.affiche_diff_seulement->isChecked() ) {
            m_ihm.ui.table_variables_valeurs->item(i, Cihm_DataView::COL_VALEUR)->setText(data->read().toString());
            double datatime = normaliseTemps(data->getTime());
            m_ihm.ui.table_variables_valeurs->item(i, Cihm_DataView::COL_TEMPS)->setText(QString::number(datatime));
        }
    }
    else
    {
        m_application->m_print_view->print_error(this, QString(__FUNCTION__) + ": Erreur sur la récupération de la data " + var_name);
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
  // Identifie quelle variable est concernée
  int index = m_ihm.ui.table_variables_valeurs->currentRow();
  m_application->m_print_view->print_debug(this, "index a deplacer = " + QString::number(index));
  if (index <= 0) { return; } // valeur invalide ou premier élément de la liste

  // Arrête le timer le temps de la manipulation
  bool timer_active = false;
  if (m_timer_lecture_variables.isActive()) {
    timer_active = true;
    m_timer_lecture_variables.stop();
  }

  // Déplace la variable
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
  // Identifie quelle variable est concernée
  int index = m_ihm.ui.table_variables_valeurs->currentRow();
  m_application->m_print_view->print_debug(this, "index a deplacer = " + QString::number(index));
  if (index < 0) { return; } // valeur invalide
  if (index >= (m_ihm.ui.table_variables_valeurs->rowCount()-1)) { return; } // valeur invalide ou dernier élément de la liste

  // Arrête le timer, le temps de la manipulation
  bool timer_active = false;
  if (m_timer_lecture_variables.isActive()) {
    timer_active = true;
    m_timer_lecture_variables.stop();
  }

  // Déplace la variable
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
*  Slot appelé lorsque l'utilisateur termine la saisie d'une valeur à écrire
*  Chaque élément sélectionné dans la liste se voit affecté de la nouvelle valeur
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
    } // if l'élément est sélectionné
  }

  // si aucune variable n'a été écrite, préciser à l'utilisateur pourquoi
  if (nbre_variables_ecrites == 0) {
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText("Sélectionner une ou plusieurs données à écrire");
    msgBox.exec();
  }
}


// _____________________________________________________________________
/*!
*  Enregistre les données dans un fichier texte au format CSV
*/
void CDataView::saveToFile()
{
    // Construit le chemin du fichier de sortie :
    //    CheminEnregistrement/<NomModule>_Terminal_<DateHeure>.txt
    QString pathfilename;
    pathfilename =    m_application->m_pathname_log_file +
                        "/" +
                        QString(getName()).replace(" ", "") + // Supprime les espaces dans le nom généré
                        CToolBox::getDateTime() +
                        ".trc";
    // Ouvre et construit le fichier
    QFile file(pathfilename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        m_application->m_print_view->print_error(this, "Impossible d'ouvrir en écriture le fichier " + pathfilename);
        return;
    }
    // Ecrit le fichier
    QTextStream out(&file);
    // 1ere ligne du fichier = nom des colonnes
    for (int col=0; col<m_ihm.ui.table_variables_valeurs->columnCount(); col++)
    {
        out << m_ihm.ui.table_variables_valeurs->horizontalHeaderItem(col)->text();
        if (col != (m_ihm.ui.table_variables_valeurs->columnCount()-1))
            out << ";";
    }
    out << "\n";

    for (int row=0; row<m_ihm.ui.table_variables_valeurs->rowCount(); row++)
    {
        for (int col=0; col<m_ihm.ui.table_variables_valeurs->columnCount(); col++)
        {
            out << m_ihm.ui.table_variables_valeurs->item(row, col)->text();
            if (col != (m_ihm.ui.table_variables_valeurs->columnCount()-1))
                out << ";";
        }
        out << "\n";
    }
    file.close();
}


// _____________________________________________________________________
/*!
*  Filtre les variables sur le nom
*/
void CDataView::onDataFilterChanged(QString filter_name)
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
