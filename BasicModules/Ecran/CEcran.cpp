/*! \file CEcran.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CEcran.h"
#include "CLaBotBox.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CTrameFactory.h"



/*! \addtogroup Module_Test2
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CEcran::CEcran(const char *plugin_name)
    :CBasicModule(plugin_name, VERSION_Ecran, AUTEUR_Ecran, INFO_Ecran)
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
void CEcran::init(CLaBotBox *application)
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


  m_application->m_data_center->write("Vbat", 0);
  m_application->m_data_center->write("TempsMatch", 0);

  m_ihm.ui.tps_dix->setNumber(0);
  m_ihm.ui.tps_unit->setNumber(0);
  m_ihm.ui.tabWidget->setCurrentIndex(0);

  connect(m_ihm.ui.pB_Green,SIGNAL(clicked(bool)),this,SLOT(onClicColorButton()));
  connect(m_ihm.ui.pB_Orange,SIGNAL(clicked(bool)),this,SLOT(onClicColorButton()));

  //pour le mode visu on se connecte aux changements du datamanager
  connect(m_application->m_data_center,SIGNAL(valueChanged(CData*)),this,SLOT(color_Changed(CData*)));
  connect(m_application->m_data_center->getData("Vbat"), SIGNAL(valueChanged(QVariant)), this, SLOT(Vbat_changed(QVariant)));
  connect(m_application->m_data_center->getData("TempsMatch"), SIGNAL(valueChanged(QVariant)), this, SLOT(TpsMatch_changed(QVariant)));
  connect(m_application->m_data_center->getData("Telemetre1"), SIGNAL(valueChanged(QVariant)), this, SLOT(Telemetre1_changed(QVariant)));
  connect(m_application->m_data_center->getData("Telemetre2"), SIGNAL(valueChanged(QVariant)), this, SLOT(Telemetre2_changed(QVariant)));
  connect(m_application->m_data_center->getData("Telemetre3"), SIGNAL(valueChanged(QVariant)), this, SLOT(Telemetre3_changed(QVariant)));
  connect(m_application->m_data_center->getData("Telemetre4"), SIGNAL(valueChanged(QVariant)), this, SLOT(Telemetre4_changed(QVariant)));


//  int modeMbed=m_application->m_data_center->getData("ModeFonctionnement")->read().toInt();
//  if(modeMbed==1)
//      setBackgroundColor(Qt::red);

  m_ihm.ui.qLed_blue->setVisible(false);
  m_ihm.ui.qLed_yellow->setVisible(false);
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
*  Création des menus sur clic droit sur la fenêtre du module
*
*/
void CEcran::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

void CEcran::onClicColorButton()
{
    QObject* pB_ColorButton=sender();
    if(pB_ColorButton->objectName().contains("Yellow"))
    {
        m_application->m_data_center->write("ECRAN_ETAT_ECRAN_TxSync", true);
        m_application->m_data_center->write("valeur_etat_ecran", 1);
        m_application->m_data_center->write("commande_etat_ecran", LBB_CMDE_CHOIX_EQUIPE);
        m_application->m_data_center->write("ECRAN_ETAT_ECRAN_TxSync", false);

    }
    if(pB_ColorButton->objectName().contains("Blue"))
    {
        m_application->m_data_center->write("ECRAN_ETAT_ECRAN_TxSync", true);
        m_application->m_data_center->write("valeur_etat_ecran", 0);
        m_application->m_data_center->write("commande_etat_ecran", LBB_CMDE_CHOIX_EQUIPE);
        m_application->m_data_center->write("ECRAN_ETAT_ECRAN_TxSync", false);
    }

}

void CEcran::color_Changed(CData *data)
{


    QString dataName=data->getName();
    if(dataName.compare("CouleurEquipe")==0)
    {
        qDebug() << data->read();
        int colorTeam=data->read().toInt();

        if(colorTeam==1)
        {
            m_ihm.ui.qLed_blue->setVisible(false);
            m_ihm.ui.qLed_yellow->setVisible(true);
        }

        if(colorTeam==0)
        {    m_ihm.ui.qLed_blue->setVisible(true);
            m_ihm.ui.qLed_yellow->setVisible(false);
        }
    }

    if(dataName.compare("Telemetre1")==0)
        m_ihm.ui.lcd_telemetre_1->display(data->read().toDouble());
    if(dataName.compare("Telemetre2")==0)
        m_ihm.ui.lcd_telemetre_2->display(data->read().toDouble());
    if(dataName.compare("Telemetre3")==0)
        m_ihm.ui.lcd_telemetre_3->display(data->read().toDouble());
    if(dataName.compare("Telemetre4")==0)
        m_ihm.ui.lcd_telemetre_4->display(data->read().toDouble());


    if(dataName.compare("ModeFonctionnement")==0){
        int value=data->read().toInt();
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
}

void CEcran::Vbat_changed(QVariant val){ m_ihm.ui.VBatt->setValue(val.toDouble());}
void CEcran::Telemetre1_changed(QVariant val){ m_ihm.ui.lcd_telemetre_1->display(val.toInt());}
void CEcran::Telemetre2_changed(QVariant val){ m_ihm.ui.lcd_telemetre_2->display(val.toInt());}
void CEcran::Telemetre3_changed(QVariant val){ m_ihm.ui.lcd_telemetre_3->display(val.toInt());}
void CEcran::Telemetre4_changed(QVariant val){ m_ihm.ui.lcd_telemetre_4->display(val.toInt());}
void CEcran::TpsMatch_changed(QVariant val)
{
    //m_ihm.ui.VBatt->setValue(val.toDouble());

    int TpsMatch=val.toInt();
    if((TpsMatch>0)&&(TpsMatch<=1))
        m_ihm.ui.tabWidget->setCurrentIndex(1);
    if((TpsMatch>79)&&(TpsMatch<=80))
        m_ihm.ui.tabWidget->setCurrentIndex(2);
    if(TpsMatch>99)
        m_ihm.ui.tabWidget->setCurrentIndex(3);

    if(m_ihm.ui.tabWidget->currentIndex()==2)
    {
        int dizaine=qFloor(TpsMatch/10);
        int unite=qAbs(TpsMatch-10*dizaine);
        m_ihm.ui.tps_dix->setNumber(dizaine);
        m_ihm.ui.tps_unit->setNumber(unite);
    }

    if(m_ihm.ui.tabWidget->currentIndex()==3)
    {
        int Score=m_application->m_data_center->getData("Score")->read().toInt();
        int centaine=qFloor(Score/100);
        int dizaine=qFloor(Score-centaine*100);
        int unite=qAbs(Score-10*dizaine-100*centaine);
        m_ihm.ui.score_cent->setNumber(dizaine);
        m_ihm.ui.score_dix->setNumber(dizaine);
        m_ihm.ui.score_unit->setNumber(unite);
    }
}
