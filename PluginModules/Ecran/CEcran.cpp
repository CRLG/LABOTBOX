/*! \file CEcran.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include "CEcran.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CTrameFactory.h"



/*! \addtogroup Module_CEcran
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CEcran::CEcran(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_Ecran, AUTEUR_Ecran, INFO_Ecran)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CEcran::~CEcran()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CEcran::init(CApplication *application)
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
  //val = m_application->m_eeprom->read(getName(), "background_color", QVariant(DEFAULT_MODULE_COLOR));
  //setBackgroundColor(val.value<QColor>());
  initColor=getBackgroundColor();
  /*val = m_application->m_eeprom->read(getName(), "isMaximized", QVariant(0));
  m_isMaximized=val.toInt();*/

  // Restitue la couleur des boutons des 2 équipes
  QString color_str = m_application->m_eeprom->read(getName(), "pB_Couleur1_color", "").toString();
  if (!color_str.isEmpty()) {
    m_ihm.ui.pB_Couleur1->setStyleSheet(QStringLiteral("background-color: %1;").arg(color_str));
  }
  color_str = m_application->m_eeprom->read(getName(), "pB_Couleur2_color", "").toString();
  if (!color_str.isEmpty()) {
    m_ihm.ui.pB_Couleur2->setStyleSheet(QStringLiteral("background-color: %1;").arg(color_str));
  }
  // Restitue le texte sur les boutons des 2 équipes
  QString button_txt = m_application->m_eeprom->read(getName(), "pB_Couleur1_text", "").toString();
  if (!button_txt.isEmpty()) {
    m_ihm.ui.pB_Couleur1->setText(button_txt);
  }
  button_txt = m_application->m_eeprom->read(getName(), "pB_Couleur2_text", "").toString();
  if (!button_txt.isEmpty()) {
      m_ihm.ui.pB_Couleur2->setText(button_txt);
  }

  // S'assure que les données existent dans le DataManager
  m_application->m_data_center->write("CouleurEquipe",  -1);
  m_application->m_data_center->write("NumStrategie",  -1);
  m_application->m_data_center->write("ModeFonctionnement",  -1);
  m_application->m_data_center->write("Telemetre4",  -1);
  m_application->m_data_center->write("Telemetre3",  -1);
  m_application->m_data_center->write("Telemetre2",  -1);
  m_application->m_data_center->write("Telemetre1",  -1);
  m_application->m_data_center->write("Vbat", -1);
  m_application->m_data_center->write("TempsMatch", -1);
  m_application->m_data_center->write("Score", 0);

  m_ihm.ui.tps_dix->setNumber(0);
  m_ihm.ui.tps_unit->setNumber(0);
  m_ihm.ui.tabWidget->setCurrentIndex(0);

  connect(m_ihm.ui.pB_Couleur1,SIGNAL(clicked(bool)),this,SLOT(onClicColorButton()));
  connect(m_ihm.ui.pB_Couleur2,SIGNAL(clicked(bool)),this,SLOT(onClicColorButton()));
  connect(m_ihm.ui.combo_ChoixStrategie,SIGNAL(activated(int)),this,SLOT(onStrategyChoice_changed(int)));
  connect(m_ihm.ui.RPI_Reboot,SIGNAL(clicked(bool)),this,SLOT(onRPI_Reboot()));
  connect(m_ihm.ui.RPI_Shutdown,SIGNAL(clicked(bool)),this,SLOT(onRPI_Shutdown()));

  //pour le mode visu on se connecte aux changements du datamanager

  connect(m_application->m_data_center->getData("CouleurEquipe"), SIGNAL(valueChanged(QVariant)), this, SLOT(CouleurEquipe_changed(QVariant)));
  connect(m_application->m_data_center->getData("NumStrategie"), SIGNAL(valueChanged(QVariant)), this, SLOT(NumStrategie_changed(QVariant)));
  connect(m_application->m_data_center->getData("ModeFonctionnement"), SIGNAL(valueUpdated(QVariant)), this, SLOT(ModeFonctionnement_changed(QVariant)));
  connect(m_application->m_data_center->getData("Vbat"), SIGNAL(valueChanged(QVariant)), this, SLOT(Vbat_changed(QVariant)));
  connect(m_application->m_data_center->getData("TempsMatch"), SIGNAL(valueChanged(QVariant)), this, SLOT(TpsMatch_changed(QVariant)));
  connect(m_application->m_data_center->getData("Telemetre1"), SIGNAL(valueChanged(QVariant)), this, SLOT(Telemetre1_changed(QVariant)));
  connect(m_application->m_data_center->getData("Telemetre2"), SIGNAL(valueChanged(QVariant)), this, SLOT(Telemetre2_changed(QVariant)));
  connect(m_application->m_data_center->getData("Telemetre3"), SIGNAL(valueChanged(QVariant)), this, SLOT(Telemetre3_changed(QVariant)));
  connect(m_application->m_data_center->getData("Telemetre4"), SIGNAL(valueChanged(QVariant)), this, SLOT(Telemetre4_changed(QVariant)));
  connect(m_application->m_data_center->getData("Score"), SIGNAL(valueChanged(QVariant)), this, SLOT(Score_changed(QVariant)));
  connect(m_application->m_data_center->getData("ObstacleDetecte"), SIGNAL(valueChanged(QVariant)), this, SLOT(ObstacleDetecte_changed(QVariant)));
  connect(m_application->m_data_center->getData("DiagBlocage"), SIGNAL(valueChanged(QVariant)), this, SLOT(DiagBlocage_changed(QVariant)));
  connect(m_application->m_data_center->getData("ConvergenceAsserv"), SIGNAL(valueChanged(QVariant)), this, SLOT(ConvergenceAsserv_changed(QVariant)));


//  int modeMbed=m_application->m_data_center->getData("ModeFonctionnement")->read().toInt();
//  if(modeMbed==1)
//      setBackgroundColor(Qt::red);

  m_ihm.ui.qLed_green->setVisible(false);
  m_ihm.ui.qLed_orange->setVisible(false);

  initStrategies();

  val = m_application->m_eeprom->read(getName(), "logger_active", QVariant(false));
  if (val.toBool()) initDataLogger();
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CEcran::close(void)
{
  // Mémorise en EEPROM l'état de la fenêtre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
}

// _____________________________________________________________________
/*!
*  Initialise les données à logger
*
*/
void CEcran::initDataLogger()
{
    QVector <CData *> data_list;
    data_list.append(m_application->m_data_center->getData("TempsMatch"));
    data_list.append(m_application->m_data_center->getData("ObstacleDetecte"));
    data_list.append(m_application->m_data_center->getData("DiagBlocage"));
    data_list.append(m_application->m_data_center->getData("ConvergenceAsserv"));
    data_list.append(m_application->m_data_center->getData("x_pos"));
    data_list.append(m_application->m_data_center->getData("y_pos"));
    data_list.append(m_application->m_data_center->getData("teta_pos"));

    m_data_logger.setDataList(data_list);
    m_data_logger.setRefreshPeriod(200);
    QString pathfilename = getLogFilename();
    m_data_logger.start(pathfilename);
    m_application->m_print_view->print_debug(this, "Start logger : " + pathfilename);
}

// _____________________________________________________________________
QString CEcran::getLogFilename()
{
    QString pathfilename;

    for (int i=1; i<10000; i++) {
        pathfilename =  m_application->m_pathname_log_file +
                        "/" +
                        "ecran_log_match_" +
                        QString::number(i) +
                        ".csv";
        QFileInfo fileinfo(pathfilename);
        if (!fileinfo.exists()) break;
    }
    return pathfilename;
}


// _____________________________________________________________________
/*!
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CEcran::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();
  QString object_name = m_ihm.childAt(pos)->objectName();
  // cas particulier pour les 2 boutons de choix de la couleur de l'équipe
  if (object_name == "pB_Couleur1" || object_name == "pB_Couleur2") {
      menu->addAction("Select team color", this, SLOT(onSelectTeamColor()));
      menu->addAction("Set team name", this, SLOT(onSelectTeamName()));
  }
  // couleur de fond de la fenêtre
  else {
      menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  }
  menu->exec(m_ihm.mapToGlobal(pos));
}

/*!
 * \brief Sélectionne la couleur de bouton d'une équipe
 */
void CEcran::onSelectTeamColor()
{
    QPoint pos = m_ihm.mapFromGlobal( QCursor::pos() );
    QColorDialog colordlg;
    QColor color = colordlg.getColor();
    if (color.isValid()) {
        QWidget *wdgt = m_ihm.childAt(pos);  // récupère le widget pointé par la souris
        if (!wdgt) return;
        QString button_name = wdgt->objectName();
        wdgt->setStyleSheet(QStringLiteral("background-color: %1;").arg(color.name()));  // change la couleur de l'objet pointé
        // sauvegarde en EEPROM la couleur
        m_application->m_eeprom->write(getName(), QString("%1_color").arg(button_name), color.name());
    }
}

/*!
 * \brief Sélectionne lae texte sur le bouton d'une équipe
 */
void CEcran::onSelectTeamName()
{
    QPoint pos = m_ihm.mapFromGlobal( QCursor::pos() );  // récupère immédiatement la position du curseur avant qu'elle ne change
    bool ok;
    QString text = QInputDialog::getText(&m_ihm, "",
                                         tr("Nom de l'équipe"), QLineEdit::Normal,
                                         "", &ok);
    if (ok && !text.isEmpty()) {
        QWidget *wdgt = m_ihm.childAt(pos);
        if (!wdgt) return;
        QString button_name = wdgt->objectName();
        QPushButton *pb = (QPushButton*)wdgt;
        if (pb) pb->setText(text);
        // sauvegarde en EEPROM le nom du bouton
        m_application->m_eeprom->write(getName(), QString("%1_text").arg(button_name), text);
    }
}

void CEcran::onClicColorButton()
{
    QObject* pB_ColorButton=sender();
    if(pB_ColorButton->objectName().contains("Couleur2"))
    {
        m_application->m_data_center->write("ECRAN_ETAT_ECRAN_TxSync", true);
        m_application->m_data_center->write("valeur_etat_ecran", 1);
        m_application->m_data_center->write("commande_etat_ecran", LBB_CMDE_CHOIX_EQUIPE);
        m_application->m_data_center->write("ECRAN_ETAT_ECRAN_TxSync", false);

    }
    if(pB_ColorButton->objectName().contains("Couleur1"))
    {
        m_application->m_data_center->write("ECRAN_ETAT_ECRAN_TxSync", true);
        m_application->m_data_center->write("valeur_etat_ecran", 0);
        m_application->m_data_center->write("commande_etat_ecran", LBB_CMDE_CHOIX_EQUIPE);
        m_application->m_data_center->write("ECRAN_ETAT_ECRAN_TxSync", false);
    }

}

void CEcran::CouleurEquipe_changed(QVariant val)
{
    int colorTeam=val.toInt();
    if(colorTeam==1)
    {
        m_ihm.ui.qLed_green->setVisible(false);
        m_ihm.ui.qLed_orange->setVisible(true);
    }

    if(colorTeam==0)
    {
        m_ihm.ui.qLed_green->setVisible(true);
        m_ihm.ui.qLed_orange->setVisible(false);
    }
}

void CEcran::ModeFonctionnement_changed(QVariant val)
{
    int value=val.toInt();
    if(value==0)//mode autonome
    {
        m_ihm.ui.lbl_ModeFct->setText("Autonome");
        setBackgroundColor(initColor);
    }
    else if (value==1)
    {
        m_ihm.ui.lbl_ModeFct->setText("LaBotBox");
        setBackgroundColor(QColor(255,0,0,255));
    }
    else if (value==2)
    {
        m_ihm.ui.lbl_ModeFct->setText("Defaillant");
        setBackgroundColor(QColor(255,0,0,255));
    }
    else if (value==2)
    {
        m_ihm.ui.lbl_ModeFct->setText("Inopérant");
        setBackgroundColor(QColor(255,0,0,255));
    }
}

void CEcran::Vbat_changed(QVariant val){ m_ihm.ui.VBatt->setValue(val.toDouble());}
void CEcran::Telemetre1_changed(QVariant val){ m_ihm.ui.sb_AVD->setValue(val.toInt());}
void CEcran::Telemetre2_changed(QVariant val){ m_ihm.ui.sb_ARD->setValue(val.toInt());}
void CEcran::Telemetre3_changed(QVariant val){ m_ihm.ui.sb_AVG->setValue(val.toInt());}
void CEcran::Telemetre4_changed(QVariant val){ m_ihm.ui.sb_ARG->setValue(val.toInt());}

void CEcran::TpsMatch_changed(QVariant val)
{
    const unsigned char DUREE_MATCH = 100;

    int TpsMatch=val.toInt();
    if((TpsMatch>0)&&(TpsMatch<=1))
        m_ihm.ui.tabWidget->setCurrentIndex(1);
    if((TpsMatch>=(DUREE_MATCH-10)&&(TpsMatch<=(DUREE_MATCH-9))))
        m_ihm.ui.tabWidget->setCurrentIndex(2);
    if(TpsMatch>=(DUREE_MATCH-1))
        m_ihm.ui.tabWidget->setCurrentIndex(3);

    if(m_ihm.ui.tabWidget->currentIndex()==1)
    {
       m_ihm.ui.sb_x->setValue(m_application->m_data_center->getData("x_pos")->read().toDouble());
       m_ihm.ui.sb_y->setValue(m_application->m_data_center->getData("y_pos")->read().toDouble());
       m_ihm.ui.sb_teta->setValue(m_application->m_data_center->getData("teta_pos")->read().toDouble());
    }
    if(m_ihm.ui.tabWidget->currentIndex()==2)
    {
        int dizaine=qFloor(TpsMatch/10);
        int unite=qAbs(TpsMatch-10*dizaine);
        m_ihm.ui.tps_dix->setNumber(dizaine);
        m_ihm.ui.tps_unit->setNumber(unite);
    }
}

void CEcran::Score_changed(QVariant val)
{
    int Score = val.toInt();
    qDebug() << Score;
    int centaine=qFloor(Score/100);
    int dizaine=qFloor((Score-centaine*100)/10);
    int unite=qAbs(Score-10*dizaine-100*centaine);
    m_ihm.ui.score_cent->setNumber(centaine);
    m_ihm.ui.score_dix->setNumber(dizaine);
    m_ihm.ui.score_unit->setNumber(unite);
}

void CEcran::ObstacleDetecte_changed(QVariant val)
{
    m_ihm.ui.qLed_obst->setValue(val.toBool());
    m_ihm.ui.qLed_Obstacle->setValue(val.toBool());
}

void CEcran::DiagBlocage_changed(QVariant val)
{
    m_ihm.ui.qLed_bloc->setValue(val.toBool());
}

void CEcran::ConvergenceAsserv_changed(QVariant val)
{
    m_ihm.ui.qLed_conv->setValue(val.toBool());
}

void CEcran::onRPI_Shutdown()
{
    int ret = QMessageBox::warning(&m_ihm, tr("Warning"),
                                   tr("Power Off\n"
                                      "Are you sure ?"),
                                   QMessageBox::Ok | QMessageBox::Cancel);
    if (ret == QMessageBox::Ok) {
        QProcess *myProcess = new QProcess();
        myProcess->start("shutdown --poweroff now");
    }
}

void CEcran::onRPI_Reboot()
{
    int ret = QMessageBox::warning(&m_ihm, tr("Warning"),
                                   tr("Reboot\n"
                                      "Are you sure ?"),
                                   QMessageBox::Ok | QMessageBox::Cancel);
    if (ret == QMessageBox::Ok) {
        QProcess *myProcess = new QProcess();
        myProcess->start("shutdown --reboot now");
    }
}

// =======================================================
//                    STRATEGY
// =======================================================
void CEcran::initStrategies()
{
    QStringList list;
    // Lettre ici tous les numéros de stratégie possible
    // Seuls les numéros déclarés dans la liste ci-dessous seront proposés dans la liste déroulante
    list << strategyNumToString(0)
         << strategyNumToString(1)
         << strategyNumToString(2)
         << strategyNumToString(3)
         << strategyNumToString(4)
         << strategyNumToString(5)
         << strategyNumToString(6)
         << strategyNumToString(7)
         << strategyNumToString(8);
    m_ihm.ui.combo_ChoixStrategie->addItems(list);
}

QString CEcran::strategyNumToString(unsigned char num)
{
    // Correspondance entre un numéro et un nom de stratégie
    // Possibilité de mettre des noms plus parlants si besoin
    switch(num) {
        case 0 : return "Par défaut";
        case 1 : return "Homolo 1";
        case 2 : return "Homolo 2";
        case 3 : return "Strategie 1";
        case 4 : return "Strategie 2";
        case 5 : return "Strategie 3";
        case 6 : return "Strategie 4";
        case 7 : return "Strategie 5";
        case 8 : return "Strategie 6";
        default : return "!! UNKNOWN STRATEGY: " + QString::number(num);
    }
}

void CEcran::onStrategyChoice_changed(int val)
{
    m_application->m_data_center->write("ECRAN_ETAT_ECRAN_TxSync", true);
    m_application->m_data_center->write("valeur_etat_ecran", val);
    m_application->m_data_center->write("commande_etat_ecran", LBB_CMDE_CHOIX_NUMERO_STRATEGIE);
    m_application->m_data_center->write("ECRAN_ETAT_ECRAN_TxSync", false);
    checkStrategyMatch();
}

void CEcran::NumStrategie_changed(QVariant val)
{
   m_ihm.ui.lbl_RetourStrategie->setText(strategyNumToString(val.toInt()));
   checkStrategyMatch();
}

void CEcran::checkStrategyMatch()
{
    CData *data = m_application->m_data_center->getData("NumStrategie");
    if (!data) return;
    if (data->read().toInt() == m_ihm.ui.combo_ChoixStrategie->currentIndex()) {
        m_ihm.ui.lbl_RetourStrategie->setStyleSheet(QStringLiteral("color: rgb(78, 154, 6);")); // vert
    }
    else  {
        m_ihm.ui.lbl_RetourStrategie->setStyleSheet(QStringLiteral("color: rgb(239, 41, 41);")); // rouge
    }
}
