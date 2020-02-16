/*! \file CDataGraph.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QMessageBox>
#include "CDataGraph.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include <QTest>
#include <QFile>
#include <QDate>
#include <QTime>
#include <QtMath>
#include <QtXml/QDomDocument>
#include <QFileDialog>
#include <QMessageBox>



/*! \addtogroup Module_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CDataGraph::CDataGraph(const char *plugin_name)
    :CBasicModule(plugin_name, VERSION_DataGraph, AUTEUR_DataGraph, INFO_DataGraph)
{
    //Initialisation des paramètres propres à  l'affichage graphique
    dureeObservee=20;
    graphColor=-1;
    graphEnabled=true;
    storedRange=new QCPRange(-100,100);
    storedRangeAxe1=new QCPRange(-350,350);
    storedRangeAxe2=new QCPRange(-4,4);
    t_predefinedColorName<<"noir"<<"rouge"<<"vert"<<"bleu"<<"cyan"<<"magenta"<<"jaune"<<"gris";
    cursorEnabled=false;
    nearDistanceCursor1=0;
    nearDistanceCursor2=0;
    isCursor1Selected=false;
    isCursor2Selected=false;
    fineDistanceCursorSelection=7;
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CDataGraph::~CDataGraph()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CDataGraph::init(CApplication *application)
{
    CModule::init(application);
    setGUI(&m_ihm); // indique à  la classe de base l'IHM
    setNiveauTrace(MSG_TOUS);

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

    //labels pour le tableau de données
    QStringList QS_Labels;
    QS_Labels << "Variable" << "Valeur" << "Couleur" << "Curseur 1" << "Curseur 2";
    m_ihm.ui.table_variables_valeurs->setHorizontalHeaderLabels(QS_Labels);
    m_ihm.ui.table_variables_valeurs->horizontalHeader()->setVisible(true);
    m_ihm.ui.table_variables_valeurs->horizontalHeaderItem(3)->setTextColor(Qt::green);
    m_ihm.ui.table_variables_valeurs->horizontalHeaderItem(4)->setTextColor(Qt::red);

    // Restaure la zone minimale autour du curseur pour le sélectionner
    val = m_application->m_eeprom->read(getName(), "fineDistanceCursorSelection", QVariant(true));
    fineDistanceCursorSelection=val.toInt();

    //Restore les dernières variables loggées (créé la data dans le DataManager + l'ajoute sur l'IHM + coche la case)
    val = m_application->m_eeprom->read(getName(), "datalist_logged", "");
    QStringList datalist_memo = val.toStringList();
    for (int i=0; i<datalist_memo.count(); i++) {
        QString dataname = datalist_memo.at(i);
        if (dataname.simplified() != "") {
            m_application->m_data_center->write(dataname, "");  //crée la donnée si elle n'existe pas
            m_ihm.ui.liste_variables->addItem(datalist_memo.at(i));
        }
    }
    for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
        if (datalist_memo.contains(m_ihm.ui.liste_variables->item(i)->text())) {
            m_ihm.ui.liste_variables->item(i)->setCheckState(Qt::Checked);
        }
    }
    refreshListeVariables();

    //On cache les contrà´les agissant sur les courbes (réactivés lors de la pause des courbes)
    m_ihm.ui.label_deltaT->setVisible(false);
    m_ihm.ui.pb_resetZoom->setVisible(true);

    //de même on ne permet pas l'activation des curseurs
    m_ihm.ui.pb_cusor->setEnabled(false);
    m_ihm.ui.pb_cusor->setChecked(false);

    //ni de la pause
    m_ihm.ui.pb_resume->setEnabled(false);
    m_ihm.ui.pb_resume->setChecked(false);

    // le bouton refresh met à  jour la liste des variables disponibles
    connect(m_ihm.ui.PB_refresh_liste, SIGNAL(clicked()), this, SLOT(refreshListeVariables()));
    connect(m_application->m_data_center, SIGNAL(dataCreated(CData*)), this, SLOT(refreshListeVariables()));
    connect(m_ihm.ui.pb_play,SIGNAL(toggled(bool)),this,SLOT(activeGraph(bool)));
    connect(m_ihm.ui.pb_up, SIGNAL(clicked()), this, SLOT(upVariable()));
    connect(m_ihm.ui.pb_down, SIGNAL(clicked()), this, SLOT(downVariable()));
    connect(m_ihm.ui.liste_variables,SIGNAL(itemChanged(QListWidgetItem*)),this,SLOT(authorizePlay(QListWidgetItem*)));

    connect(&m_timer_lecture_variables, SIGNAL(timeout()), this, SLOT(refreshValeursVariables()));

    //récupération du pointeur du graphique et création des curseurs
    customPlot=m_ihm.findChild<QCustomPlot*>("customPlot");
    cursor1 = new QCPItemLine(customPlot);
    cursor1->setPen(QPen(Qt::green));
    cursor2 = new QCPItemLine(customPlot);
    cursor2->setPen(QPen(Qt::red));

    // à  inclure pour supprimer l'antialiasing pour de meilleures perfos
    /*
    customPlot->setNotAntialiasedElements(QCP::aeAll);
    QFont font;
    font.setStyleStrategy(QFont::NoAntialias);
    customPlot->xAxis->setTickLabelFont(font);
    customPlot->yAxis->setTickLabelFont(font);
    customPlot->legend->setFont(font);
    */
    //formattage des axes
    customPlot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    customPlot->xAxis->setDateTimeFormat("hh:mm:ss");
    customPlot->xAxis->setAutoTickStep(false);
    customPlot->xAxis->setTickStep(2);
    customPlot->axisRect()->setupFullAxesBox();
    //range du premier axe
    customPlot->yAxis->setRange(-300,300);
    val = m_application->m_eeprom->read(getName(), "initHighRange", QVariant());
    customPlot->yAxis->setRange(-fabs(val.toDouble()),fabs(val.toDouble()));
    //range du deuxième axe
    customPlot->yAxis2->setRange(-3.5,3.5);
    val = m_application->m_eeprom->read(getName(), "initLowRange", QVariant());
    customPlot->yAxis2->setRange(-fabs(val.toDouble()),fabs(val.toDouble()));
    customPlot->yAxis2->setAutoTickLabels(true);
    customPlot->yAxis2->setTickLabels(true);
    customPlot->yAxis2->setVisible(true);

    //uniformisation des ranges entre les axes dessus-dessous et gauche-droite
    connect(customPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), customPlot->xAxis2, SLOT(setRange(QCPRange)));

    //gestion des ranges pour les différents zoom
    storedRange->lower=customPlot->yAxis->range().lower;
    storedRange->upper=customPlot->yAxis->range().upper;
    storedRangeAxe1->lower=customPlot->yAxis->range().lower;
    storedRangeAxe1->upper=customPlot->yAxis->range().upper;
    storedRangeAxe2->lower=customPlot->yAxis2->range().lower;
    storedRangeAxe2->upper=customPlot->yAxis2->range().upper;
    //par sécurité on normalise les ranges
    storedRange->normalize();storedRangeAxe1->normalize();storedRangeAxe2->normalize();
    QString str;
    m_ihm.ui.inputRangeAxe1->setText(str.setNum(storedRangeAxe1->upper));
    m_ihm.ui.inputRangeAxe2->setText(str.setNum(storedRangeAxe2->upper));

    //Interaction avec la souris: défilement de la courbe, zoom, sélection de lignes
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectItems);
    // zone minimale autour du curseur pour le sélectionner
    customPlot->setSelectionTolerance(fineDistanceCursorSelection);
    //pour l'init on désactive temporairement le drag and drop des courbes (réactive à  la mise en pause des courbes)
    customPlot->axisRect()->setRangeDrag(0);
    //pour l'init on désactive le zoom (réactive à  la mise en pause des courbes)
    customPlot->axisRect()->setRangeZoom(0);

    //mise à  jour des courbes
    customPlot->replot();    

    backupEnabled=false;

    //gestion des interactions des courbes
    connect(m_ihm.ui.pb_resume,SIGNAL(toggled(bool)),this,SLOT(resumeGraph(bool))); //pause des courbes
    connect(m_ihm.ui.table_variables_valeurs,SIGNAL(itemDoubleClicked(QTableWidgetItem*)),this, SLOT(itemRescaleGraph(QTableWidgetItem*))); //redimensionnement des axes
    connect(customPlot,SIGNAL(mousePress(QMouseEvent*)),this,SLOT(mousePress(QMouseEvent*))); //gestion des curseurs
    connect(customPlot,SIGNAL(mouseMove(QMouseEvent*)),this,SLOT(mouseMove(QMouseEvent*)));
    connect(customPlot,SIGNAL(mouseRelease(QMouseEvent*)),this,SLOT(mouseRelease(QMouseEvent*)));
    connect(m_ihm.ui.pb_resetZoom, SIGNAL(clicked()),this,SLOT(resetZoom())); //resetZoom (no comment)
    connect(m_ihm.ui.pb_cusor, SIGNAL(toggled(bool)),this, SLOT(enableCursor(bool)));
    connect(m_ihm.ui.pb_Range,SIGNAL(clicked()),this,SLOT(userRescaleGraph()));
    connect(m_ihm.ui.sliderZoom,SIGNAL(valueChanged(int)),this,SLOT(userZoomGraph(int)));
	//manage record data
    //connect(m_ihm.ui.pb_record,SIGNAL(toggled(bool)),this,SLOT(Slot_save(bool))); //pause des courbes
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CDataGraph::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));

  // mémorise en EEPROM la liste des variables sous surveillance (cochées sur l'IHM)
  QStringList liste_variables_observees;
  for (int i=0; i<m_ihm.ui.liste_variables->count(); i++) {
      if (m_ihm.ui.liste_variables->item(i)->checkState() == Qt::Checked) {
          liste_variables_observees.append(m_ihm.ui.liste_variables->item(i)->text());
      }
  }
  m_application->m_eeprom->write(getName(), "datalist_logged", QVariant(liste_variables_observees));
}

// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CDataGraph::onRightClicGUI(QPoint pos)
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
void CDataGraph::setVisible(void)
{
  CModule::setVisible();
  refreshListeVariables();
}


// _____________________________________________________________________
/*!
*  Met à  jour la liste des variables dans la fenêtre de gauche
*
*/
void CDataGraph::refreshListeVariables(void)
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
*  Point  d'entrée lorsque la checkbox d'activation est actionnée
*
*/
void CDataGraph::activeGraph(bool state)
{
    //qDebug() << "toogled";
  if (state) { // activation du suivi des variables
    m_ihm.ui.table_variables_valeurs->clearContents();
    m_ihm.ui.table_variables_valeurs->setRowCount(0);
    playGraph();
    m_ihm.ui.pb_play->setIcon(QIcon(":/icons/player_stop.png"));

    //on autorise la pause
    m_ihm.ui.pb_resume->setEnabled(true);
    m_ihm.ui.pb_resume->setChecked(false);

  }
  else {
      stopGraph(); // désactivation du suivi des variables
      m_ihm.ui.pb_play->setIcon(QIcon(":/icons/player_play.png"));

      //on ne permet plus la pause la pause
      m_ihm.ui.pb_resume->setEnabled(false);
      m_ihm.ui.pb_resume->setChecked(false);
  }

  // vérouille les objets pendant l'enregistrement
  //m_ihm.ui.choix_mode_inspection->setEnabled(!state);
  m_ihm.ui.liste_variables->setEnabled(!state);
  //m_ihm.ui.liste_variables->setEnabled(!state);
  m_ihm.ui.PB_refresh_liste->setEnabled(!state);
}


// _____________________________________________________________________
/*!
*  Efface la liste des variables
*
*/
void CDataGraph::effacerListeVariablesValues(void)
{
 m_ihm.ui.table_variables_valeurs->clearContents();
 m_ihm.ui.table_variables_valeurs->setRowCount(0);
}

// _____________________________________________________________________
/*!
*  Ajoute une ligne à  la liste des valeurs de variables
*   name = nom de la variable
*   value = valeur de la variable
*
*/
void CDataGraph::addTraceVariable(QString name, QString value)
{
 QColor randomColor=variousColor();

 unsigned int index = m_ihm.ui.table_variables_valeurs->rowCount();
 m_ihm.ui.table_variables_valeurs->insertRow(index);
 m_ihm.ui.table_variables_valeurs->setItem(index, 0, new QTableWidgetItem());
 m_ihm.ui.table_variables_valeurs->setItem(index, 1, new QTableWidgetItem());
 m_ihm.ui.table_variables_valeurs->item(index, 0)->setText(name);
 m_ihm.ui.table_variables_valeurs->item(index, 1)->setText(value);

 m_ihm.ui.table_variables_valeurs->item(index, 0)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
 m_ihm.ui.table_variables_valeurs->item(index, 1)->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

 //cellule de choix de couleur de la courbe
 QComboBox *choixCouleur=widgetCouleur(index);
 choixCouleur->setCurrentIndex(graphColor);
 connect(choixCouleur,SIGNAL(currentIndexChanged(int)),this,SLOT(changeGraphCouleur(int)));
 m_ihm.ui.table_variables_valeurs->setCellWidget(index,2,choixCouleur);

 //cellule qui indique la valeur du curseur associé au bouton gauche
 m_ihm.ui.table_variables_valeurs->setItem(index, 3, new QTableWidgetItem());
 m_ihm.ui.table_variables_valeurs->setItem(index, 4, new QTableWidgetItem());

 //on crée une courbe et on l'associe à  la variable
 int indexGraph=customPlot->graphCount();

 QVariant property_value=m_application->m_data_center->getDataProperty(name,"Range");
 int range_property=property_value.toInt();

 QVariant val = m_application->m_eeprom->read(getName(), "initLowRange", QVariant());
 if (range_property>val.toDouble())
    customPlot->addGraph(customPlot->xAxis,customPlot->yAxis); //ligne
 else
    customPlot->addGraph(customPlot->xAxis,customPlot->yAxis2); //ligne
 customPlot->graph(indexGraph)->setName(name);
 m_liste_graph[name]=indexGraph;
 customPlot->graph(indexGraph)->setPen(QPen(randomColor));

 if (range_property>val.toDouble())
    customPlot->addGraph(customPlot->xAxis,customPlot->yAxis); //ligne
 else
    customPlot->addGraph(customPlot->xAxis,customPlot->yAxis2); //ligne
 customPlot->graph(indexGraph+1)->setPen(QPen(randomColor));
 customPlot->graph(indexGraph+1)->setLineStyle(QCPGraph::lsNone);
 customPlot->graph(indexGraph+1)->setScatterStyle(QCPScatterStyle::ssDisc);
}

// _____________________________________________________________________
/*!
 * \brief CDataGraph::variousColor retourne une couleur, à  chaque appel on procède à  une permutation circulaire dans
 * le tableau des couleurs prédéfinies.
 * \return
 */
QColor CDataGraph::variousColor()
{
    graphColor++;
    if (graphColor>7) graphColor=0;

    return t_predefinedColor[graphColor];
}

// _____________________________________________________________________
/*!
 * \brief CDataGraph::widgetCouleur création d'un widget qcomboBox de choix de couleur
 * \param indexRow on associe le comboBox à  une ligne dans le tableau des variables
 * \return
 */
QComboBox *CDataGraph::widgetCouleur(int indexRow)
{
    QComboBox *choixCouleur=new QComboBox();

    //création d'un petit carré noir (qu'on remplira de couleur bien entendu :-) )
    QImage img(16,16,QImage::Format_RGB32);
    QPainter p(&img);
    p.fillRect(img.rect(), Qt::black);
    QRect rect = img.rect().adjusted(1,1,-1,-1);

    //remplissage de la comboBox
    for(int h=0;h<8;h++){
        p.fillRect(rect,t_predefinedColor[h]);
        choixCouleur->addItem(t_predefinedColorName[h],QVariant(indexRow));
        choixCouleur->setItemData(h, QPixmap::fromImage(img),Qt::DecorationRole);
    }

    return choixCouleur;
}

// _____________________________________________________________________
/*!
*  Actions à  effecter lors de l'activation de l'enregistrement des data en mode "Instantane"
*
*/
void CDataGraph::playGraph(void)
{
  m_ihm.ui.pb_up->setEnabled(true);
  m_ihm.ui.pb_down->setEnabled(true);

  // Recopie toutes les variables à  surveiller
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
*  Actions à  effecter lors de l'activation de l'enregistrement des data en mode "Instantane"
*
*/
void CDataGraph::stopGraph(void)
{
  //repositionne le bouton pause
  m_ihm.ui.pb_resume->setChecked(false);

  // Arrête le timer de mise à  jour des variables si l'enregistrement est stoppé
  m_timer_lecture_variables.stop();

  //nettoie la liste des variables suivies
  effacerListeVariablesValues();

  //nettoie le graph
  m_liste_graph.clear();
  customPlot->clearGraphs();
  customPlot->replot();
 }

// _____________________________________________________________________
/*!
*  Met à  jour la valeur de chaque variable contenu dans la liste des variables à  surveiller
* Traces les courbes à  chaque pas de temps
*
*/
void CDataGraph::refreshValeursVariables(void)
{
    double key = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;
	if(backupEnabled)
	{
		sauvStream << key <<";";
	}

    for (int i=0; i<m_ihm.ui.table_variables_valeurs->rowCount(); i++)
	{
        QString var_name = m_ihm.ui.table_variables_valeurs->item(i, 0)->text();
        QVariant var_value = m_application->m_data_center->read(var_name);

        if(graphEnabled)
        {
            //identification de la courbe
            int indexGraph=m_liste_graph[var_name];
            //ajout des données à  la courbe "ligne"
            customPlot->graph(indexGraph)->addData(key, var_value.toDouble());
            //positionnement du point
            customPlot->graph(indexGraph+1)->clearData();
            customPlot->graph(indexGraph+1)->addData(key, var_value.toDouble());
            //if(backupEnabled)
				

            //suppression des données qui sont en dehors du range visible (positionné à  "dureeObservee" secondes):
            //avec un tampon de 30 sec
            if(key> (dureeObservee+30))
            customPlot->graph(indexGraph)->removeDataBefore(key-(dureeObservee+30));

            //adaptation du range de l'axe vertical pour correspondre au range des nouvelles données:
            //customPlot->graph(0)->rescaleValueAxis();
        }
		
        /*if(backupEnabled)
		{
			sauvStream << var_value <<";";
        }*/

        m_ihm.ui.table_variables_valeurs->item(i, 1)->setText(var_value.toString());
    }

    if(graphEnabled)
    {
        //défilement de l'axe horizontal (à  un écart constant de "dureeObservee" secondes):
        customPlot->xAxis->setRange(key+0.25, dureeObservee, Qt::AlignRight);
        customPlot->replot();
    }

}

// _____________________________________________________________________
/*!
*  Remonte la variable dans la liste
*
*/
void CDataGraph::upVariable()
{
  // Identifie quelle variable est concernée
  int index = m_ihm.ui.table_variables_valeurs->currentRow();
  if (index <= 0) { return; } // valeur invalide ou premier élément de la liste

  // Arrête le timer le temps de la manipulation
  bool timer_active = false;
  if (m_timer_lecture_variables.isActive()) {
    timer_active = true;
    m_timer_lecture_variables.stop();
  }

  // Déplace la variable
  QString var_name = m_ihm.ui.table_variables_valeurs->item(index, 0)->text();
  QString cursor1_value=m_ihm.ui.table_variables_valeurs->item(index, 3)->text();
  QString cursor2_value=m_ihm.ui.table_variables_valeurs->item(index, 4)->text();

  QComboBox* widgetCouleurChoisie=qobject_cast<QComboBox*>(m_ihm.ui.table_variables_valeurs->cellWidget(index, 2));
  int couleurChoisie=widgetCouleurChoisie->currentIndex();
  m_ihm.ui.table_variables_valeurs->removeRow(index);
  int new_row = index - 1;
  m_ihm.ui.table_variables_valeurs->insertRow(new_row);
  m_ihm.ui.table_variables_valeurs->setItem(new_row, 0, new QTableWidgetItem());
  m_ihm.ui.table_variables_valeurs->setItem(new_row, 1, new QTableWidgetItem());
  m_ihm.ui.table_variables_valeurs->item(new_row, 0)->setText(var_name);

  //gestion des cellules qui gèrent les courbes et l'affichage du curseur gauche
  widgetCouleurChoisie=widgetCouleur(new_row);
  widgetCouleurChoisie->setCurrentIndex(couleurChoisie);
  connect(widgetCouleurChoisie,SIGNAL(currentIndexChanged(int)),this,SLOT(changeGraphCouleur(int)));
  m_ihm.ui.table_variables_valeurs->setCellWidget(new_row,2,widgetCouleurChoisie);

  m_ihm.ui.table_variables_valeurs->setItem(new_row, 3, new QTableWidgetItem(cursor1_value));
  m_ihm.ui.table_variables_valeurs->setItem(new_row, 4, new QTableWidgetItem(cursor2_value));
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
void CDataGraph::downVariable()
{
  // Identifie quelle variable est concernée
  int index = m_ihm.ui.table_variables_valeurs->currentRow();
  if (index < 0) { return; } // valeur invalide
  if (index >= (m_ihm.ui.table_variables_valeurs->rowCount()-1)) { return; } // valeur invalide ou dernier élément de la liste

  // Arrête le timer, le temps de la manipulation
  bool timer_active = false;
  if (m_timer_lecture_variables.isActive()) {
    timer_active = true;
    m_timer_lecture_variables.stop();
  }

  // Déplace la variable
  QString var_name = m_ihm.ui.table_variables_valeurs->item(index, 0)->text();
  QString cursor1_value=m_ihm.ui.table_variables_valeurs->item(index, 3)->text();
  QString cursor2_value=m_ihm.ui.table_variables_valeurs->item(index, 4)->text();

  QComboBox* widgetCouleurChoisie=qobject_cast<QComboBox*>(m_ihm.ui.table_variables_valeurs->cellWidget(index, 2));
  int couleurChoisie=widgetCouleurChoisie->currentIndex();
  m_ihm.ui.table_variables_valeurs->removeRow(index);
  int new_row = index + 1;
  m_ihm.ui.table_variables_valeurs->insertRow(new_row);
  m_ihm.ui.table_variables_valeurs->setItem(new_row, 0, new QTableWidgetItem());
  m_ihm.ui.table_variables_valeurs->setItem(new_row, 1, new QTableWidgetItem());
  m_ihm.ui.table_variables_valeurs->item(new_row, 0)->setText(var_name);

  //gestion des cellules qui gèrent les courbes et l'affichage du curseur gauche
  widgetCouleurChoisie=widgetCouleur(new_row);
  widgetCouleurChoisie->setCurrentIndex(couleurChoisie);
  connect(widgetCouleurChoisie,SIGNAL(currentIndexChanged(int)),this,SLOT(changeGraphCouleur(int)));
  m_ihm.ui.table_variables_valeurs->setCellWidget(new_row,2,widgetCouleurChoisie);

  m_ihm.ui.table_variables_valeurs->setItem(new_row, 3, new QTableWidgetItem(cursor1_value));
  m_ihm.ui.table_variables_valeurs->setItem(new_row, 4, new QTableWidgetItem(cursor2_value));
  m_ihm.ui.table_variables_valeurs->selectRow(new_row);

 // Rallume le timer
 if (timer_active) {
    m_timer_lecture_variables.start(PERIODE_ECHANTILLONNAGE_VARIABLES);
 }
}

// _____________________________________________________________________
/*!
 * \brief CDataGraph::resumeInspector gèle le traà§age des courbes et rend disponible le zoom et les curseurs
 * \param varToggle
 */
void CDataGraph::resumeGraph(bool varToggle)
{
    if(varToggle)
    {
        //si on veut geler également la lecture de variables
        //m_timer_lecture_variables.stop();

        graphEnabled=false;//on gèle le traà§age

        //on permet le drag des courbes (uniquement le long de l'axe horizontal)
        customPlot->axisRect()->setRangeDrag(Qt::Horizontal);

        //on active le zoom
        customPlot->axisRect()->setRangeZoom(Qt::Vertical);
        storedRange->expand(customPlot->yAxis->range());
        m_ihm.ui.pb_resetZoom->setVisible(true);

        //on permet l'activation des curseurs
        m_ihm.ui.pb_cusor->setEnabled(true);
        m_ihm.ui.pb_cusor->setChecked(false);
    }
    else
    {
        //m_timer_lecture_variables.start();

        //on désactive temporairement le drag and drop des courbes
        customPlot->axisRect()->setRangeDrag(0);

        graphEnabled=true;//on réactive les courbes

        //on désactive le zoom en réhabilitant le range par défaut.
        customPlot->axisRect()->setRangeZoom(0);
        m_ihm.ui.pb_resetZoom->setVisible(false);
        QCPRange const restoredRange(storedRange->lower,storedRange->upper);
        customPlot->yAxis->setRange(restoredRange);

        //on ne permet plus l'activation des curseurs
        m_ihm.ui.pb_cusor->setEnabled(false);
        m_ihm.ui.pb_cusor->setChecked(false);
   }
}

// _____________________________________________________________________
/*!
 * \brief CDataGraph::changeGraphCouleur change la couleur d'une courbe sur selection de celle-ci pour une variable donnée
 * \param indexCouleur couleur choisie
 */
void CDataGraph::changeGraphCouleur(int indexCouleur)
{
    // Arrête le timer le temps de la manipulation
    bool timer_active = false;
    if (m_timer_lecture_variables.isActive()) {
      timer_active = true;
      m_timer_lecture_variables.stop();
    }

    // Identifie quelle variable est concernée (ligne stockée dans le combobox)
    QComboBox* choixCouleur=qobject_cast<QComboBox*>(sender());
    int index = choixCouleur->itemData(indexCouleur).toInt();
    QString var_name = m_ihm.ui.table_variables_valeurs->item(index, 0)->text();
    //on identifie le graph grace à  son nom
    int indexGraph=m_liste_graph[var_name];
    //on met la couleur choisie
    customPlot->graph(indexGraph)->setPen(QPen(t_predefinedColor[indexCouleur]));
    customPlot->graph(indexGraph+1)->setPen(QPen(t_predefinedColor[indexCouleur]));

    // Rallume le timer
    if (timer_active) {
       m_timer_lecture_variables.start(PERIODE_ECHANTILLONNAGE_VARIABLES);
    }
}

// _____________________________________________________________________
/*!
 * \brief CDataGraph::rescaleGraph redimensionnement des ordonnées en double cliquant sur une variable tracée.
 * \param item référence de l'objet stockant la valeur de la variable tracée.
 */
void CDataGraph::itemRescaleGraph(QTableWidgetItem *item)
{
    //valeur absolue de la valeur
    double var_absValue=fabs(m_ihm.ui.table_variables_valeurs->item(item->row(),1)->text().toDouble());

    QVariant property_value=m_application->m_data_center->getDataProperty(m_ihm.ui.table_variables_valeurs->item(item->row(),0)->text(),"Range");
    int range_property=property_value.toInt();
    QVariant val = m_application->m_eeprom->read(getName(), "initLowRange", QVariant());

    QString str;
    //Le nouveau range correspond à  120% du range [-valeur, +valeur]
    if (range_property>val.toDouble())
    {
        rescaleGraph(var_absValue*1.2,0);
        m_ihm.ui.inputRangeAxe1->setText(str.setNum(var_absValue*1.2));
    }
    else
    {
       rescaleGraph(0,var_absValue*1.2);
       m_ihm.ui.inputRangeAxe2->setText(str.setNum(var_absValue*1.2));
    }
}

void CDataGraph::userRescaleGraph()
{
    m_ihm.ui.sliderZoom->setValue(0);

    QString newStringMaxRange1=m_ihm.ui.inputRangeAxe1->text();
    double newMaxRange1=newStringMaxRange1.toDouble();
    QString newStringMaxRange2=m_ihm.ui.inputRangeAxe2->text();
    double newMaxRange2=newStringMaxRange2.toDouble();

    if(fabs(newMaxRange1)!=storedRangeAxe1->upper)
    {
        storedRangeAxe1->upper=fabs(newMaxRange1);
        storedRangeAxe1->lower=-fabs(newMaxRange1);
        rescaleGraph(newMaxRange1,0);
    }
    if(fabs(newMaxRange2)!=storedRangeAxe2->upper)
    {
        storedRangeAxe2->upper=fabs(newMaxRange2);
        storedRangeAxe2->lower=-fabs(newMaxRange2);
        rescaleGraph(0,newMaxRange2);
    }
}

void CDataGraph::userZoomGraph(int value)
{
    //value 0 do nothing
    double zoomedRange=0;
    //zoom out
    if (value>0)
    {
        zoomedRange=(value/20)*(storedRangeAxe1->upper);
        rescaleGraph(zoomedRange,0);
    }
    if (value<0)
    {
        zoomedRange=(storedRangeAxe1->upper)/(fabs(value));
        rescaleGraph(zoomedRange,0);
    }
}

void CDataGraph::rescaleGraph(double newMaxRange, double newMaxRange2)
{
    //Le nouveau range correspond à  120% du range [-valeur, +valeur] ou à  [-5, 5] si la valeur est nulle
    double upperRange=storedRangeAxe1->upper;
    double upperRange2=storedRangeAxe2->upper;

    //gestion axe1
    if(fabs(newMaxRange)!=0)
    {
        upperRange=(fabs(newMaxRange));
        double lowerRange=-upperRange;
        customPlot->yAxis->setRange(lowerRange,upperRange);
    }
    //gestion axe2
    if(fabs(newMaxRange2)!=0)
    {
        upperRange2=(fabs(newMaxRange2));
        double lowerRange2=-upperRange2;
        customPlot->yAxis2->setRange(lowerRange2,upperRange2);
    }

    customPlot->replot();
}

// _____________________________________________________________________
/*!
 * \brief CDataGraph::ManageCursor Dessine le curseur graphique (relativement au bouton gauche ou droit) sous le curseur de la souris.
 * Ecriture/mise à  jour des valeurs des variables tracées relativement à  la position du curseur gauche
 * \param cursor référence du curseur graphique
 * \param xPosition abscisse du curseur de la souris
 */
void CDataGraph::ManageCursor(QCPItemLine *cursor, double xPosition, int columTableWidget)
{

    //Positionnement du curseur à  l'abscisse du curseur de la souris
    cursor->start->setCoords( xPosition, (customPlot->yAxis->range().lower));
    cursor->end->setCoords( xPosition, (customPlot->yAxis->range().upper));


    //les deux curseurs sont présent on met à  jour le DeltaT
    double deltaT=fabs((cursor1->start->coords().x())-(cursor2->start->coords().x()));
    QString str;
    m_ihm.ui.label_deltaT->setText(str.number(deltaT,'f',3));


    QString var_valueFormated;
    double var_value;
    int indexGraph;
    QString var_name;

    for(int index=0;index<m_ihm.ui.table_variables_valeurs->rowCount();index++)
    {
        var_name = m_ihm.ui.table_variables_valeurs->item(index, 0)->text();
        //on identifie le graph grace à  son nom
        indexGraph=m_liste_graph[var_name];
        //on prend la valeur de la courbe
        var_value=customPlot->graph(indexGraph)->data()->lowerBound(xPosition).value().value;
        m_ihm.ui.table_variables_valeurs->item(index, columTableWidget)->setText(var_valueFormated.number(var_value,'f',2));
    }

}

// _____________________________________________________________________
/*!
 * \brief CDataGraph::ResetCursor On retire les curseurs du graphique. Les curseur sont rénitialisés aux valeurs par défaut.
 * Les valeurs des variables tracées relativement à  la position du curseur gauche sont effacées
 */
void CDataGraph::ResetCursor(void)
{
    //On retire les curseurs
    if((customPlot->hasItem(cursor1))) customPlot->removeItem(cursor1);
    if((customPlot->hasItem(cursor2))) customPlot->removeItem(cursor2);
    //on réinitialise les curseurs
    cursor1 = new QCPItemLine(customPlot);
    cursor1->setPen(QPen(Qt::green));
    cursor2 = new QCPItemLine(customPlot);
    cursor2->setPen(QPen(Qt::red));
    //On efface les valeurs relatives à  la position du curseur gauche
    for(int index=0;index<m_ihm.ui.table_variables_valeurs->rowCount();index++)
    {
        m_ihm.ui.table_variables_valeurs->item(index, 3)->setText("");
        m_ihm.ui.table_variables_valeurs->item(index, 4)->setText("");
    }
}

// _____________________________________________________________________
/*!
 * \brief CDataGraph::mousePress redirection du gestionnaire d'événement lors d'un click souris.
 * Si le curseur graphique est autorisé (gel de l'affichage), un curseur graphique est positionné sous le curseur de la souris.
 * On gère un curseur graphique par bouton (gauche et droit).
 * \param event
 */
void CDataGraph::mousePress(QMouseEvent* event)
{
    if(cursorEnabled)
    {
        double y=event->pos().y();
        double x=event->pos().x();
        QPointF const posMouse(x,y);

        nearDistanceCursor1=cursor1->selectTest(posMouse,true);
        nearDistanceCursor2=cursor2->selectTest(posMouse,true);

        if (nearDistanceCursor1 <= fineDistanceCursorSelection) isCursor1Selected=true;
        if (nearDistanceCursor2 <= fineDistanceCursorSelection) isCursor2Selected=true;
    }
}

void CDataGraph::mouseRelease(QMouseEvent* event)
{
    isCursor1Selected=false;
    isCursor2Selected=false;
}

void CDataGraph::mouseMove(QMouseEvent *event)
{
    if(cursorEnabled)
    {
        //on récupère le curseur de la souris
        double x=customPlot->xAxis->pixelToCoord(event->pos().x());

        //gestion du bouton gauche et droit
        if(isCursor1Selected) ManageCursor(cursor1, x,3); //positionnement du curseur gauche
        if(isCursor2Selected) ManageCursor(cursor2, x,4); //positionnement du curseur droit

        customPlot->replot();
    }
}

// _____________________________________________________________________
/*!
 * \brief CDataGraph::resetZoom restaure le Range stocké dans le membre storedRange
 */
void CDataGraph::resetZoom()
{
    QCPRange const restoredRange(storedRangeAxe1->lower,storedRangeAxe1->upper);
    m_ihm.ui.sliderZoom->setValue(0);
    customPlot->yAxis->setRange(restoredRange);
    customPlot->replot();
}

void CDataGraph::authorizePlay(QListWidgetItem* senderItem){

    if((senderItem->checkState()==Qt::Checked)&&(senderItem->text().compare("Nouvel élément")!=0)){
        //une variable au moins est sélectionnée une fois
        m_ihm.ui.pb_play->setEnabled(true);
    }
}

void CDataGraph::enableCursor(bool varToggle)
{
    cursorEnabled=varToggle;
    if (varToggle){

        customPlot->axisRect()->setRangeDrag(0); //désactive temporairement le drag and drop des courbes

        //On ajoute les curseurs au graphique
        customPlot->addItem(cursor1);
        customPlot->addItem(cursor2);
        //qDebug() << customPlot->itemCount() << "créés";

        //on les rend sélectionnables
        cursor1->setSelectable(true);
        cursor2->setSelectable(true);

        //Positionnement des curseurs
        cursor1->start->setCoords( (customPlot->xAxis->range().upper)-7, (customPlot->yAxis->range().lower));
        cursor1->end->setCoords( (customPlot->xAxis->range().upper)-7, (customPlot->yAxis->range().upper));

        cursor2->start->setCoords( (customPlot->xAxis->range().upper)-5, (customPlot->yAxis->range().lower));
        cursor2->end->setCoords( (customPlot->xAxis->range().upper)-5, (customPlot->yAxis->range().upper));

        ManageCursor(cursor1,(customPlot->xAxis->range().upper)-7,3);
        ManageCursor(cursor2,(customPlot->xAxis->range().upper)-5,4);

        customPlot->replot();

        //on rend le delta T visible
        m_ihm.ui.label_deltaT->setVisible(true);
        m_ihm.ui.label_deltaT->setText("2");
    }
    else{
        customPlot->axisRect()->setRangeDrag(Qt::Horizontal); //réactive drag de la courbe (uniquement le long de l'axe horizontal)

        //le delta T n'est plus visible
        m_ihm.ui.label_deltaT->setVisible(false);

        //on enlève les curseurs le cas échéant
        ResetCursor();

        customPlot->replot();
    }
}

void CDataGraph::Slot_save(bool toggleState)
{
	if(!toggleState)
	{
		// Init of the backup file
		QString ficName("dataGraph_");
		QString temps2=QTime::currentTime().toString("hh_mm");
		QString temps1 = QDate::currentDate().toString("yyyy_MM_dd_at_");
		ficName.append(temps1);
		ficName.append(temps2);
		ficName.append(".csv");
		//qDebug() << ficName;

		sauvFic=new QFile(ficName);
		// On ouvre notre fichier en lecture seule et on vérifie l'ouverture
		if (sauvFic->open(QIODevice::ReadWrite | QIODevice::Text))
		{
			// Création d'un objet QTextStream à partir de notre objet QFile
			sauvStream.setDevice(sauvFic);
			// On choisit le codec correspondant au jeu de caractère que l'on souhaite ; ici, UTF-8
			sauvStream.setCodec("UTF-8");
			//write header
			sauvStream << "time;";
			for (int i=0; i<m_ihm.ui.table_variables_valeurs->rowCount(); i++)		
				sauvStream << m_ihm.ui.table_variables_valeurs->item(i, 0)->text() << ";";
			sauvStream << endl;
			backupEnabled=true;
		}
	}
	else //we don't want anymore record data
	{
		if(sauvFic) //a file is already opened
         sauvFic->close();
		backupEnabled=false;
			 
	}
}
