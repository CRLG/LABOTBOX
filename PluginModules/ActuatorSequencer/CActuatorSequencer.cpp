/*! \file CActuatorSequencer.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CActuatorSequencer.h"
#include "CApplication.h"
#include "CPrintView.h"
#include "CMainWindow.h"
#include "CEEPROM.h"
#include "CDataManager.h"
#include "CSimuBot.h"
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
CActuatorSequencer::CActuatorSequencer(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_ActuatorSequencer, AUTEUR_ActuatorSequencer, INFO_ActuatorSequencer)
{
    bPlay=false;
    bResume=false;
    bStop=false;
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CActuatorSequencer::~CActuatorSequencer()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CActuatorSequencer::init(CApplication *application)
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
  //restaure le répertoire par défaut pour les sauvegardes
  val = m_application->m_eeprom->read(getName(), "default_path", QVariant("."));
  defaultPath=val.toString();

  m_ihm.ui.tW_TabSequences->setCurrentIndex(0);
  QTableWidget * newSequence= new QTableWidget();
  QWidget * newTab=m_ihm.ui.tW_TabSequences->currentWidget();
  QString strName=m_ihm.ui.strategyName->text();
  creatTabSequence(newTab,newSequence,strName,true);

  //nettoyage des séquences quand on ferme l'application
  connect(m_ihm.ui.tW_TabSequences,SIGNAL(tabCloseRequested(int)),this,SLOT(Slot_Remove_Sequence(int)));

  //remplissage des combobox (utilisation du data.ini si cela est possible)
  updateComboBox();

  connect(m_ihm.ui.strategyName, SIGNAL(textEdited(QString)), this, SLOT(Slot_setStrategyName_tab(QString)));
  connect(m_ihm.ui.tW_TabSequences, SIGNAL(currentChanged(int)), this, SLOT(Slot_setStrategyName_text(int)));

  connect(m_ihm.ui.pB_Add_AX,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_SD20,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_Motor,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_Power,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_Wait,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_Asser,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_Event,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_Sensor,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_Node,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_Free_Transition,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_Free_Action,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Edit_Free_Action,SIGNAL(clicked(bool)),this,SLOT(Slot_editFreeItem()));
  connect(m_ihm.ui.pB_Load_Free_Action,SIGNAL(clicked(bool)),this,SLOT(Slot_loadFreeItem()));
  connect(m_ihm.ui.pB_Edit_Free_Transition,SIGNAL(clicked(bool)),this,SLOT(Slot_editFreeItem()));
  connect(m_ihm.ui.pB_Load_Free_Transition,SIGNAL(clicked(bool)),this,SLOT(Slot_loadFreeItem()));

  connect(m_ihm.ui.pB_Remove, SIGNAL(clicked(bool)),this,SLOT(removeSequenceItem()));
  connect(m_ihm.ui.pB_Up, SIGNAL(clicked(bool)),this,SLOT(Slot_up()));
  connect(m_ihm.ui.pB_Down, SIGNAL(clicked(bool)),this,SLOT(Slot_down()));
  connect(m_ihm.ui.pB_Play, SIGNAL(clicked(bool)),this,SLOT(Slot_Play()));
  connect(m_ihm.ui.pB_Resume, SIGNAL(clicked(bool)),this,SLOT(Slot_Resume()));
  connect(m_ihm.ui.pB_Stop, SIGNAL(clicked(bool)),this,SLOT(Slot_Stop()));
  connect(m_ihm.ui.pB_Play_Step, SIGNAL(clicked(bool)),this,SLOT(playStep()));

  connect(m_ihm.ui.pB_Save, SIGNAL(clicked(bool)),this,SLOT(Slot_Save()));
  connect(m_ihm.ui.pB_Load, SIGNAL(clicked(bool)),this,SLOT(Slot_Load()));
  connect(m_ihm.ui.pB_Clear, SIGNAL(clicked(bool)),this,SLOT(Slot_Clear()));
  connect(m_ihm.ui.pB_New, SIGNAL(clicked(bool)), this, SLOT(Slot_Add_Sequence()));
  connect(m_ihm.ui.pB_Delete, SIGNAL(clicked(bool)), this, SLOT(Slot_Delete()));
  connect(m_ihm.ui.pB_Clone, SIGNAL(clicked(bool)), this, SLOT(Slot_Clone()));
  connect(m_ihm.ui.pB_Generate, SIGNAL(clicked(bool)),this,SLOT(Slot_Generate_CPP()));
  connect(m_ihm.ui.pB_Strategies_combine,SIGNAL(clicked(bool)),this,SLOT(Slot_combineStrategies()));
  //connect(m_ihm.ui.pB_Strategy_up,SIGNAL(clicked(bool)),this,SLOT(Slot_moveStrategy()));
  //connect(m_ihm.ui.pB_Strategy_down,SIGNAL(clicked(bool)),this,SLOT(Slot_moveStrategy()));


  connect(m_ihm.ui.pB_Play_SD20, SIGNAL(clicked(bool)),this,SLOT(Slot_Play_only_SD20()));
  connect(m_ihm.ui.pB_Play_AX, SIGNAL(clicked(bool)),this,SLOT(Slot_Play_only_AX()));
  connect(m_ihm.ui.pB_Play_Motor, SIGNAL(clicked(bool)),this,SLOT(Slot_Play_only_motor()));
  connect(m_ihm.ui.pB_Stop_Motor, SIGNAL(clicked(bool)),this,SLOT(Slot_Stop_only_motors()));
  connect(m_ihm.ui.pB_Play_Power, SIGNAL(clicked(bool)),this,SLOT(Slot_Play_only_power()));
  connect(m_ihm.ui.pB_Stop_Power, SIGNAL(clicked(bool)),this,SLOT(Slot_Stop_only_power()));
  connect(m_ihm.ui.pB_Play_Asser, SIGNAL(clicked(bool)),this,SLOT(Slot_Play_only_asser()));
  connect(m_ihm.ui.pB_Stop_Asser, SIGNAL(clicked(bool)),this,SLOT(Slot_Stop_only_asser()));

  connect(m_ihm.ui.cB_AX,SIGNAL(currentIndexChanged(int)),this,SLOT(updateTooltip()));
  connect(m_ihm.ui.cB_Motor,SIGNAL(currentIndexChanged(int)),this,SLOT(updateTooltip()));
  connect(m_ihm.ui.cB_SD20,SIGNAL(currentIndexChanged(int)),this,SLOT(updateTooltip()));
  connect(m_ihm.ui.cB_Asser,SIGNAL(currentIndexChanged(int)),this,SLOT(updateTooltip()));
  connect(m_ihm.ui.cB_Power,SIGNAL(currentIndexChanged(int)),this,SLOT(updateTooltip()));
  m_ihm.ui.cB_AX->setCurrentIndex(0);
  m_ihm.ui.cB_Motor->setCurrentIndex(0);
  m_ihm.ui.cB_SD20->setCurrentIndex(0);
  m_ihm.ui.cB_Asser->setCurrentIndex(0);
  m_ihm.ui.cB_Asser->setCurrentIndex(0);

  connect(m_ihm.ui.tool_deg,SIGNAL(returnPressed()),this,SLOT(Slot_convert_to_rad()));
  connect(m_ihm.ui.tool_rad,SIGNAL(returnPressed()),this,SLOT(Slot_convert_to_deg()));

  connect(m_ihm.ui.cB_bRepeatSequence, SIGNAL(toggled(bool)),this,SLOT(setEnableRepetition(bool)));

  connect(m_ihm.ui.pB_get_XYTeta, SIGNAL(clicked(bool)),this,SLOT(Slot_get_XYTeta()));
  connect(m_application->m_SimuBot,SIGNAL(setSequence()),this, SLOT(Slot_SetPosFromSimu()));
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CActuatorSequencer::close(void)
{
  // M�morise en EEPROM l'�tat de la fen�tre
  m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
  m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
  m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));
  m_application->m_eeprom->write(getName(), "background_color", QVariant(getBackgroundColor()));
}

// _____________________________________________________________________
/*!
*  Cr�ation des menus sur clic droit sur la fen�tre du module
*
*/
void CActuatorSequencer::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

// _____________________________________________________________________
void CActuatorSequencer::updateComboBox(void)
{
 // Gestion des alias sur les noms et remplissage des combobox
    QString str_name;
    QString str_alias;
    int i;
//Motor
 for(i=1;i<7;i++){
     str_name=QString("cde_moteur_%1").arg(i);
     str_alias = m_application->m_data_center->getDataProperty(str_name, "Alias").toString();
     if (str_alias != "")
         m_ihm.ui.cB_Motor->addItem(str_alias,QVariant(i));
     else
         m_ihm.ui.cB_Motor->addItem(str_name.setNum(i),QVariant(i));
 }
 //SD20
 for(i=13;i<21;i++){
     str_name=QString("cde_servo_%1").arg(i);
     str_alias = m_application->m_data_center->getDataProperty(str_name, "Alias").toString();
     if (str_alias != "")
         m_ihm.ui.cB_SD20->addItem(str_alias,QVariant(i));
     else
         m_ihm.ui.cB_SD20->addItem(str_name.setNum(i),QVariant(i));
 }
 //AX
 for(int i=0;i<8;i++){
     str_name=QString("cde_ax_%1").arg(i);
     str_alias = m_application->m_data_center->getDataProperty(str_name, "Alias").toString();
     if (str_alias != "")
         m_ihm.ui.cB_AX->addItem(str_alias,QVariant(i));
     else
         m_ihm.ui.cB_AX->addItem(str_name.setNum(i),QVariant(i));
 }
 //PowerSwitch
  for(i=1;i<9;i++){
      str_name=QString("PowerSwitch_xt%1").arg(i);
      str_alias = m_application->m_data_center->getDataProperty(str_name, "Alias").toString();
      if (str_alias != "")
          m_ihm.ui.cB_Power->addItem(str_alias,QVariant(i));
      else
          m_ihm.ui.cB_Power->addItem(str_name.setNum(i),QVariant(i));
  }
m_ihm.ui.cB_AX_type_cde->addItem("Position",QVariant(cSERVO_AX_POSITION));
m_ihm.ui.cB_AX_type_cde->addItem("Vitesse",QVariant(cSERVO_AX_VITESSE));


 //ASSER
 m_ihm.ui.cB_Asser->addItem("XY",QVariant(0));
  m_ihm.ui.cB_Asser->addItem("XYTheta",QVariant(1));
 m_ihm.ui.cB_Asser->addItem("DistAng",QVariant(2));
 m_ihm.ui.cB_Asser->addItem("Init_asser",QVariant(3));
 m_ihm.ui.cB_Asser->addItem("Init_rack",QVariant(4));
 m_ihm.ui.cB_Asser->addItem("Rack",QVariant(5));
 m_ihm.ui.label_asserValue_1->setText("X");
 m_ihm.ui.label_asserValue_2->setText("Y");
 m_ihm.ui.label_asserValue_3->setText("Not Used");
 m_ihm.ui.lE_Asser_1->setValue(0);
 m_ihm.ui.lE_Asser_1->setMaximum(300);
 m_ihm.ui.lE_Asser_1->setMinimum(-300);
 m_ihm.ui.lE_Asser_2->setValue(0);
 m_ihm.ui.lE_Asser_3->setValue(0.0);
 m_ihm.ui.lE_Asser_1->setEnabled(true);
 m_ihm.ui.lE_Asser_2->setEnabled(true);
 m_ihm.ui.lE_Asser_3->setEnabled(false);

 //Event
 QStringList eventsAvalaible;
 eventsAvalaible << "convAsserv" << "convRack";
 m_ihm.ui.cB_Events->addItems(eventsAvalaible);

 //Sensor
 QStringList var_list;
 m_application->m_data_center->getListeVariablesName(var_list);
 var_list.sort();
 for (int i=0; i<var_list.count(); i++)
 {
     if (!var_list.at(i).isEmpty())
     {
         str_name=var_list.at(i);
         str_alias = m_application->m_data_center->getDataProperty(str_name, "Alias").toString();
         if (!str_alias .isEmpty())
             m_ihm.ui.cB_Sensors->addItem(str_alias,QVariant(i));
         else
             m_ihm.ui.cB_Sensors->addItem(str_name,QVariant(i));
     }
 }
 QStringList sensorConditions;
 sensorConditions << ">" << "=" << "<";
 for (int i=0; i<sensorConditions.count();i++)
     m_ihm.ui.cB_Conditions->addItem(sensorConditions.at(i),QVariant(i));
 //m_ihm.ui.cB_Conditions->addItems(sensorConditions);


}

void CActuatorSequencer::updateTooltip(void)
{
 // Gestion des alias sur les noms et remplissage des combobox
    QString str_name;
    QString str_tooltip;
    int i=0;

    QObject * wdgt=sender();

    //Motor
    if(m_ihm.ui.cB_Motor==wdgt)
    {
        i=m_ihm.ui.cB_Motor->currentIndex()+1;
        str_name=QString("cde_moteur_%1").arg(i);
        str_tooltip = m_application->m_data_center->getDataProperty(str_name, "Tooltip").toString();
        m_ihm.ui.tip_Motor->setText(str_tooltip);
        m_ihm.ui.tip_Motor->setCursorPosition(0);
    }

    //PowerSwitch
    if(m_ihm.ui.cB_Power==wdgt)
    {
        i=m_ihm.ui.cB_Power->currentIndex()+1;
        str_name=QString("PowerSwitch_xt%1").arg(i);
        str_tooltip = m_application->m_data_center->getDataProperty(str_name, "Tooltip").toString();
        m_ihm.ui.tip_Power->setText(str_tooltip);
        m_ihm.ui.tip_Power->setCursorPosition(0);
    }

     //SD20
    if(m_ihm.ui.cB_SD20==wdgt)
    {
        i=m_ihm.ui.cB_SD20->currentIndex()+13;
        str_name=QString("cde_servo_%1").arg(i);
        str_tooltip = m_application->m_data_center->getDataProperty(str_name, "Tooltip").toString();
        m_ihm.ui.tip_SD20->setText(str_tooltip);
        m_ihm.ui.tip_SD20->setCursorPosition(0);
    }

     //AX
    if(m_ihm.ui.cB_AX==wdgt)
    {
        i=m_ihm.ui.cB_AX->currentIndex();
        str_name=QString("cde_ax_%1").arg(i);
        str_tooltip = m_application->m_data_center->getDataProperty(str_name, "Tooltip").toString();
        m_ihm.ui.tip_AX->setText(str_tooltip);
        m_ihm.ui.tip_AX->setCursorPosition(0);
    }

    if(m_ihm.ui.cB_Asser==wdgt)
    {
        //Asser
        QString currentAsser=m_ihm.ui.cB_Asser->currentText();


        //ERASE OLD ASSER
            m_ihm.ui.label_asserValue_1->setText("Not Used");
            m_ihm.ui.label_asserValue_2->setText("Not Used");
            m_ihm.ui.label_asserValue_3->setText("Not Used");
            m_ihm.ui.lE_Asser_1->setValue(0);
            m_ihm.ui.lE_Asser_2->setValue(0);
            m_ihm.ui.lE_Asser_3->setValue(0.0);
            m_ihm.ui.lE_Asser_1->setEnabled(false);
            m_ihm.ui.lE_Asser_2->setEnabled(false);
            m_ihm.ui.lE_Asser_3->setEnabled(false);


        //"XY","DistAng","Rack"
        if(currentAsser=="XY")
        {
            //qDebug() << "asser text is" << currentAsser;
            m_ihm.ui.label_asserValue_1->setText("X");
            m_ihm.ui.label_asserValue_2->setText("Y");
            m_ihm.ui.label_asserValue_3->setText("Not Used");
            m_ihm.ui.lE_Asser_1->setValue(0);
            m_ihm.ui.lE_Asser_1->setMaximum(300);
            m_ihm.ui.lE_Asser_1->setMinimum(-300);
            m_ihm.ui.lE_Asser_2->setValue(0);
            m_ihm.ui.lE_Asser_3->setValue(0.0);
            m_ihm.ui.lE_Asser_1->setEnabled(true);
            m_ihm.ui.lE_Asser_2->setEnabled(true);
            m_ihm.ui.lE_Asser_3->setEnabled(false);
        }
        if(currentAsser=="XYTheta")
        {
            //qDebug() << "asser text is" << currentAsser;
            m_ihm.ui.label_asserValue_1->setText("X");
            m_ihm.ui.label_asserValue_2->setText("Y");
            m_ihm.ui.label_asserValue_3->setText("Theta");
            m_ihm.ui.lE_Asser_1->setValue(0);
            m_ihm.ui.lE_Asser_1->setMaximum(300);
            m_ihm.ui.lE_Asser_1->setMinimum(-300);
            m_ihm.ui.lE_Asser_2->setValue(0);
            m_ihm.ui.lE_Asser_3->setValue(0.0);
            m_ihm.ui.lE_Asser_1->setEnabled(true);
            m_ihm.ui.lE_Asser_2->setEnabled(true);
            m_ihm.ui.lE_Asser_3->setEnabled(true);
        }
        if(currentAsser=="DistAng")
        {
             //qDebug() << "asser text is" << currentAsser;
            m_ihm.ui.label_asserValue_1->setText("Distance");
            m_ihm.ui.label_asserValue_2->setText("Not Used");
            m_ihm.ui.label_asserValue_3->setText("Angle");
            m_ihm.ui.lE_Asser_1->setValue(0);
            m_ihm.ui.lE_Asser_1->setMaximum(300);
            m_ihm.ui.lE_Asser_1->setMinimum(-300);
            m_ihm.ui.lE_Asser_2->setValue(0);
            m_ihm.ui.lE_Asser_3->setValue(0.0);
            m_ihm.ui.lE_Asser_1->setEnabled(true);
            m_ihm.ui.lE_Asser_2->setEnabled(false);
            m_ihm.ui.lE_Asser_3->setEnabled(true);
        }
        if(currentAsser=="Init_asser")
        {
             //qDebug() << "asser text is" << currentAsser;
            m_ihm.ui.label_asserValue_1->setText("Position");
            m_ihm.ui.label_asserValue_2->setText("Not Used");
            m_ihm.ui.label_asserValue_3->setText("Not Used");
            m_ihm.ui.label_asserValue_1->setText("X");
            m_ihm.ui.label_asserValue_2->setText("Y");
            m_ihm.ui.label_asserValue_3->setText("Theta");
            m_ihm.ui.lE_Asser_1->setValue(0);
            m_ihm.ui.lE_Asser_1->setMaximum(300);
            m_ihm.ui.lE_Asser_1->setMinimum(-300);
            m_ihm.ui.lE_Asser_2->setValue(0);
            m_ihm.ui.lE_Asser_3->setValue(0.0);
            m_ihm.ui.lE_Asser_1->setEnabled(true);
            m_ihm.ui.lE_Asser_2->setEnabled(true);
            m_ihm.ui.lE_Asser_3->setEnabled(true);
        }
        if(currentAsser=="Init_rack")
        {
             //qDebug() << "asser text is" << currentAsser;
            m_ihm.ui.label_asserValue_1->setText("Not Used");
            m_ihm.ui.label_asserValue_2->setText("Not Used");
            m_ihm.ui.label_asserValue_3->setText("Not Used");
            m_ihm.ui.lE_Asser_1->setValue(0);
            m_ihm.ui.lE_Asser_2->setValue(0);
            m_ihm.ui.lE_Asser_3->setValue(0);
            m_ihm.ui.lE_Asser_1->setEnabled(false);
            m_ihm.ui.lE_Asser_2->setEnabled(false);
            m_ihm.ui.lE_Asser_3->setEnabled(false);
        }
        if(currentAsser=="Rack")
        {
             //qDebug() << "asser text is" << currentAsser;
            m_ihm.ui.label_asserValue_1->setText("Position");
            m_ihm.ui.label_asserValue_2->setText("Not Used");
            m_ihm.ui.label_asserValue_3->setText("Not Used");
            m_ihm.ui.lE_Asser_1->setValue(0);
            m_ihm.ui.lE_Asser_1->setMaximum(1500);
            m_ihm.ui.lE_Asser_1->setMinimum(-1500);
            m_ihm.ui.lE_Asser_2->setValue(0);
            m_ihm.ui.lE_Asser_3->setValue(0.0);
            m_ihm.ui.lE_Asser_1->setEnabled(true);
            m_ihm.ui.lE_Asser_2->setEnabled(false);
            m_ihm.ui.lE_Asser_3->setEnabled(false);
        }
    }
}

void CActuatorSequencer::addSequenceItem(void)
{
    int tabIndex=m_ihm.ui.tW_TabSequences->currentIndex();
    QTableWidget * currentSequence=listSequence.at(tabIndex);

    QObject * pB_Add=sender();

    QString type="NA";
    QString id="0";
    QString value="0";
    QString comments;
    QString name="";

    bool bFormat=false;

    if(pB_Add==m_ihm.ui.pB_Add_AX)
    {
        switch(m_ihm.ui.cB_AX_type_cde->currentData().toInt())
        {
        case cSERVO_AX_POSITION:
            type="AX-Position";
            break;
        case cSERVO_AX_VITESSE:
            type="AX-Speed";
            break;
        default:
            type="AX-Position";
            break;
        }


        QString id_int;
        id=id_int.setNum(m_ihm.ui.cB_AX->currentIndex());
        comments=m_ihm.ui.cB_AX->currentText();
        value.setNum(m_ihm.ui.sB_AX->value());
    }
    if(pB_Add==m_ihm.ui.pB_Add_Sensor)
    {
        id=m_ihm.ui.cB_Sensors->currentText();
        value.setNum(m_ihm.ui.sB_Sensor->value());
        switch(m_ihm.ui.cB_Conditions->currentData().toInt())
        {
        case 0:
            type="Sensor_sup";
            break;
        case 1:
            type="Sensor_egal";
            break;
        case 2:
            type="Sensor_inf";
            break;

        default:
            type="Sensor_egal";
            break;
        }
        comments=m_ihm.ui.cB_Sensors->currentText()+m_ihm.ui.cB_Conditions->currentText()+value;
        bFormat=true;

    }
    else if(pB_Add==m_ihm.ui.pB_Add_SD20)
    {
        type="SD20";
        QString id_int;
        id=id_int.setNum(m_ihm.ui.cB_SD20->currentIndex()+13);
        comments=m_ihm.ui.cB_SD20->currentText();
        value.setNum(m_ihm.ui.sB_SD20->value());
    }
    else if(pB_Add==m_ihm.ui.pB_Add_Motor)
    {
        type="Motor";
        QString id_int;
        id=id_int.setNum(m_ihm.ui.cB_Motor->currentIndex()+1);
        comments=m_ihm.ui.cB_Motor->currentText();
        value.setNum(m_ihm.ui.sB_Motor->value());
    }
    else if(pB_Add==m_ihm.ui.pB_Add_Power)
    {
        type="Power";
        QString id_int;
        id=id_int.setNum(m_ihm.ui.cB_Power->currentIndex()+1);
        comments=m_ihm.ui.cB_Power->currentText();
        value.setNum(m_ihm.ui.sB_Power->value());
    }
    else if(pB_Add==m_ihm.ui.pB_Add_Event)
    {
        type="Event";

        id=m_ihm.ui.cB_Events->currentText();
        //comments=m_ihm.ui.cB_Motor->currentText();
        value=m_ihm.ui.sB_TimeOut->text();
        bFormat=true;
    }
    else if(pB_Add==m_ihm.ui.pB_Add_Wait)
    {
        type="Wait";
        id="0";
        value.setNum(m_ihm.ui.sB_Wait->value());
        bFormat=true;
    }
    else if(pB_Add==m_ihm.ui.pB_Add_Asser)
    {
        type="Asser";
        id=m_ihm.ui.cB_Asser->currentText();
        QString other_value;
        if(m_ihm.ui.lE_Asser_1->isEnabled())
            value.setNum(m_ihm.ui.lE_Asser_1->value());

        if(m_ihm.ui.lE_Asser_2->isEnabled())
        {
            other_value.setNum(m_ihm.ui.lE_Asser_2->value());
            value=value+","+other_value;
        }
        if(m_ihm.ui.lE_Asser_3->isEnabled())
        {
            other_value.setNum(m_ihm.ui.lE_Asser_3->value());
            value=value+","+other_value;
        }
    }
    else if(pB_Add==m_ihm.ui.pB_Add_Free_Transition)
    {
        QString FreeText=m_ihm.ui.freeTransition->toPlainText();
        if(!FreeText.isEmpty())
        {
            type="FreeEvent";
            id="0";
            value=FreeText;
            bFormat=true;
        }
    }
    else if(pB_Add==m_ihm.ui.pB_Add_Free_Action)
    {
        QString FreeText=m_ihm.ui.freeAction->toPlainText();
        if(!FreeText.isEmpty())
        {
            type="FreeAction";
            id="0";
            value=FreeText;
        }
    }
    else if(pB_Add==m_ihm.ui.pB_Add_Node)
    {
        QString NodeText=m_ihm.ui.nodeName->text();
        QString autoName="NODE_%1";
        int autoIndex=0;
        bool isNamed=false;
        if(!NodeText.isEmpty())
        {
            name=NodeText;
            if(findState(currentSequence,name)<0)
                isNamed=true;
        }
        while(!isNamed)
        {
            name=autoName.arg(autoIndex);
            if(findState(currentSequence,name)<0)
                isNamed=true;
            autoIndex++;
        }

        type="Node";
        id="0";
        value="";
    }

    if(type!="NA")
    {
        QTableWidgetItem *newItem_type = new QTableWidgetItem(type);
        QTableWidgetItem *newItem_id = new QTableWidgetItem(id);
        QTableWidgetItem *newItem_value = new QTableWidgetItem(value);
        QTableWidgetItem *newItem_state = new QTableWidgetItem(name);
        QTableWidgetItem *newItem_comments = new QTableWidgetItem(comments);

        if(bFormat)
        {
            newItem_type->setBackgroundColor(Qt::cyan);
            newItem_id->setBackgroundColor(Qt::cyan);
            newItem_value->setBackgroundColor(Qt::cyan);
            newItem_state->setBackgroundColor(Qt::cyan);
            newItem_comments->setBackgroundColor(Qt::cyan);
        }


        int indexAdd=currentSequence->rowCount();

        QList<QTableWidgetItem*> list_item_selected=currentSequence->selectedItems();
        if(!list_item_selected.isEmpty())
        {
            QTableWidgetItem* item_selected=list_item_selected.first();
            int row_selected=item_selected->row();
            if(row_selected>=0)
                indexAdd=row_selected+1;
        }

        currentSequence->insertRow(indexAdd);
        fillRow(currentSequence,indexAdd,newItem_type,newItem_id,newItem_value,newItem_state,newItem_comments,false,false);
        currentSequence->selectRow(indexAdd);
    }
}

void CActuatorSequencer::removeSequenceItem()
{
    int tabIndex=m_ihm.ui.tW_TabSequences->currentIndex();
    QTableWidget * currentSequence=listSequence.at(tabIndex);

    QList<QTableWidgetItem*> list_item_selected=currentSequence->selectedItems();
    if(!list_item_selected.isEmpty())
    {
        QTableWidgetItem* item_selected=list_item_selected.first();
        int row_selected=item_selected->row();
        if(row_selected>=0)
            currentSequence->removeRow(row_selected);
    }
}

/*!
 * \brief CActuatorSequencer::Slot_Play
 * \param oneStep à mettre à vrai si on veut executer une seule ligne de la stratégie
 * \param idStart index de la ligne à jouer si oneStep est vrai
 */
void CActuatorSequencer::Slot_Play(bool oneStep, int idStart)
{
    //maj de l'ihm
    bStop=false;
    bPlay=true;
    bResume=false;
    update_sequenceButtons();
    m_ihm.ui.txtPlayInfo->clear();

    //index et ses bornes pour parcourir les états et transitions de la stratégie
    int indexItem=0;
    int idxMin=0;
    int idxMax=0;
    int nextSate=-1;

    //On se positionne sur la bonne stratégie et on récupère pointeur et index
    int tabIndex=m_ihm.ui.tW_TabSequences->currentIndex();
    QTableWidget * table_sequence=listSequence.at(tabIndex);
    int numberOfIndex=m_ihm.ui.tW_TabSequences->count();

    //On désactive temporairement les autres stratégies
    for(int i=0;i<numberOfIndex;i++)
    {
        if(i!=tabIndex)
            m_ihm.ui.tW_TabSequences->widget(i)->setEnabled(false);
    }

    //pour récupérer les infos des états et transitions
    QString sActuator;
    QString id;
    QString value;

    //pour dérouler la stratégie n fois en boucle
    int nbSequence=1;
    if ((m_ihm.ui.cB_bRepeatSequence->isChecked())&&(!oneStep))
        nbSequence=m_ihm.ui.sB_numberOfSequence->value();

    /*------------------------------------------------------------
     * DEBUT DU DEROULEMENT DE LA STRATEGIE
     *------------------------------------------------------------*/
    //donc on déroule autant de fois que nbSequence la stratégie

    QTime chrono;
    chrono.start();
    QString msg="DEBUT STRATEGIE "+m_ihm.ui.strategyName->text();
    setPlayMessage(chrono.elapsed(),"INFO",msg);

    for(int i=0;i<nbSequence;i++)
    {
        //le bouton stop a été actionné on sort de la boucle
        if((!bPlay)&&(!bResume))
            break;

        //on ne déroule la stratégie que s'il y a au moins une ligne
        if(table_sequence->rowCount()>0)
        {
            //pour executer une seule ligne de la stratégie
            if (oneStep)
            {
                idxMax=qMax(0,idStart);
                idxMin=qMin(table_sequence->rowCount(),idStart);
            }
            else
            {
                idxMax=table_sequence->rowCount()-1;
                idxMin=0;
            }

            //affichage du nombre de fois qu'on déroule la stratégie
            QString showNubSequence;
            showNubSequence.setNum(i+1);
            m_ihm.ui.label_showNbSequence->setText(showNubSequence);
            if(nbSequence>1)
            {
                msg="REPETITION STRATEGIE n°"+showNubSequence;
                setPlayMessage(chrono.elapsed(),"INFO",msg);
            }

            //on déroule la séquence
            indexItem=idxMin;

            //pour différencier les actions et les transitions
            bool isTransitionType=false;
            bool isMutliplesTransitions=false;
            bool goToNextValidStep=false;
            int idxTransition=0;
            QString strIdxTransitions;
            int cptTransition=0;
            QList<int> timeoutList;
            QList<int> nextStateList;
            QString strStateName;

            while(indexItem<idxMax+1)
            //for (indexItem=idxMin;indexItem<idxMax+1;indexItem++)
            {
                //récupération de type de ligne (actionneur, transition, événement,...)
                sActuator=getTypeText(table_sequence,indexItem);

                //est-ce une transition?
                isTransitionType=isTransition(sActuator);
                if(!isTransitionType)
                    goToNextValidStep=false;

                //on met en rouge que la ligne actuellement jouée (ne seront visibles dans les faits que les transitions)
                formatSequence(table_sequence);
                table_sequence->item(indexItem,0)->setBackgroundColor(Qt::red);

                //pour récupérer des infos de la ligne juste après et gérer les transitions multiples
                QString sNextActuator="";
                if((indexItem+1)<table_sequence->rowCount())
                    sNextActuator=getTypeText(table_sequence,indexItem+1);

                if(isTransitionType&&(isTransition(sNextActuator)))
                {
                    if(!isMutliplesTransitions)
                    {
                        idxTransition=0;
                        cptTransition=0;
                        timeoutList.clear();
                        nextStateList.clear();
                        isMutliplesTransitions=true;
                        //qDebug() << "lancement du multitransition";
                    }
                }


                if(isMutliplesTransitions)
                    strIdxTransitions.setNum(idxTransition);

                //Si la transition contient un nom d'état on y va
                if((isTransition(sActuator))&&(!goToNextValidStep))
                {
                    if(isMutliplesTransitions && (nextStateList.count()>idxTransition))
                        nextSate=nextStateList.at(idxTransition);
                    else
                    {
                        strStateName=getSateNameText(table_sequence,indexItem);
                        if(!strStateName.isEmpty())
                        {
                            int foundSate=findState(table_sequence,strStateName);
                            if(foundSate<0)
                            {
                                msg="Impossible de trouver l'état "+strStateName;
                                setPlayMessage(chrono.elapsed(),"WARNING",msg);
                            }
                            else
                                nextSate=foundSate;
                        }
                    }
                }
                //qDebug() << "prochain état" << nextSate;

                if(sActuator.compare("SD20")==0) //SERVO SD20
                {
                    id=getIdText(table_sequence,indexItem);
                    value=getValueText(table_sequence,indexItem);
                    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", true);
                    m_application->m_data_center->write("NumeroServoMoteur1", id);
                    m_application->m_data_center->write("PositionServoMoteur1", value);
                    m_application->m_data_center->write("VitesseServoMoteur1", 0);
                    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", false);
                    msg="SERVO SD20 "+id+" à la position "+value;
                    setPlayMessage(chrono.elapsed(),"ACTION",msg);
                }

                if(sActuator.compare("AX-Position")==0) //SERVO AX position
                {
                    id=getIdText(table_sequence,indexItem);
                    value=getValueText(table_sequence,indexItem);
                    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", true);
                    m_application->m_data_center->write("num_servo_ax", id);
                    m_application->m_data_center->write("commande_ax", cSERVO_AX_POSITION);
                    m_application->m_data_center->write("valeur_commande_ax", value);
                    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", false);
                    QTest::qWait(100); //Pour eviter la non prise en compte I2C
                    msg="SERVO AX "+id+" à la position "+value;
                    setPlayMessage(chrono.elapsed(),"ACTION",msg);
                }

                if(sActuator.compare("AX-Speed")==0) //SERVO AX vitesse
                {
                    id=getIdText(table_sequence,indexItem);
                    value=getValueText(table_sequence,indexItem);
                    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", true);
                    m_application->m_data_center->write("num_servo_ax", id);
                    m_application->m_data_center->write("commande_ax", cSERVO_AX_VITESSE);
                    m_application->m_data_center->write("valeur_commande_ax", value);
                    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", false);
                    QTest::qWait(100); //Pour eviter la non prise en compte I2C
                    msg="SERVO AX "+id+" vitesse à la valeur "+value;
                    setPlayMessage(chrono.elapsed(),"ACTION",msg);
                }

                if(sActuator.compare("Motor")==0) //MOTEUR
                {
                    id=getIdText(table_sequence,indexItem);
                    value=getValueText(table_sequence,indexItem);
                    QString str_name=QString("cde_moteur_%1").arg(id);
                    m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 1);
                    m_application->m_data_center->write(str_name, value);
                    m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 0);
                    msg="MOTEUR "+id+" à la valeur "+value;
                    setPlayMessage(chrono.elapsed(),"ACTION",msg);
                }

                if(sActuator.compare("Power")==0) //POWERSWITCH
                {
                    id=getIdText(table_sequence,indexItem);
                    value=getValueText(table_sequence,indexItem);
                    QString str_name=QString("PowerSwitch_xt%1").arg(id);
                    bool bvalue=false;
                    if(value.compare("1")==0)
                        bvalue=true;
                    m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 1);
                    m_application->m_data_center->write(str_name, bvalue);
                    m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 0);
                    msg="POWERSWITCH "+id+" à "+ (bvalue?"ON":"OFF");
                    setPlayMessage(chrono.elapsed(),"ACTION",msg);
                }

                if(sActuator.compare("Asser")==0) //ASSERVISSEMENT
                {
                    id=getIdText(table_sequence,indexItem);
                    value=getValueText(table_sequence,indexItem);
                    QStringList args=value.split(",",QString::SkipEmptyParts);
                    int nb_args=args.count();
                    //"XY","DistAng","Rack"
                    if(id.compare("XY")==0)
                    {
                        if(nb_args==2)
                        {
                            msg="Commande (X,Y) = ("+value+")";
                            setPlayMessage(chrono.elapsed(),"ACTION",msg);
                            m_application->m_data_center->write("COMMANDE_MVT_XY_TxSync", 1);
                            m_application->m_data_center->write("X_consigne", args.at(0).toFloat());
                            m_application->m_data_center->write("Y_consigne", args.at(1).toFloat());
                            m_application->m_data_center->write("COMMANDE_MVT_XY_TxSync", 0);
                        }
                    }
                    if(id.compare("XYTheta")==0)
                        if(nb_args==3)
                        {
                            msg="Commande (X,Y,TETA) = ("+value+")";
                            setPlayMessage(chrono.elapsed(),"ACTION",msg);
                            m_application->m_data_center->write("COMMANDE_MVT_XY_TETA_TxSync", 1);
                            m_application->m_data_center->write("XYT_X_consigne", args.at(0).toFloat());
                            m_application->m_data_center->write("XYT_Y_consigne", args.at(1).toFloat());
                            m_application->m_data_center->write("XYT_angle_consigne", args.at(2).toFloat());
                            m_application->m_data_center->write("COMMANDE_MVT_XY_TETA_TxSync", 0);
                        }
                    if(id.compare("DistAng")==0)
                    {
                        if(nb_args==2)
                        {
                            msg="Commande (Distance,Angle) = ("+value+")";
                            setPlayMessage(chrono.elapsed(),"ACTION",msg);
                            m_application->m_data_center->write("COMMANDE_DISTANCE_ANGLE_TxSync", 1);
                            m_application->m_data_center->write("distance_consigne",args.at(0).toFloat());
                            m_application->m_data_center->write("angle_consigne", args.at(1).toFloat());
                            m_application->m_data_center->write("COMMANDE_DISTANCE_ANGLE_TxSync", 0);
                        }
                    }
                    if(id.compare("Init_Asser")==0)
                        if(nb_args==3)
                        {
                            msg="Commande init(X,Y,TETA) = ("+value+")";
                            setPlayMessage(chrono.elapsed(),"ACTION",msg);
                            m_application->m_data_center->write("COMMANDE_REINIT_XY_TETA_TxSync", 1);
                            m_application->m_data_center->write("reinit_x_pos",args.at(0).toFloat());
                            m_application->m_data_center->write("reinit_y_pos", args.at(1).toFloat());
                            m_application->m_data_center->write("reinit_teta_pos", args.at(2).toFloat());
                            m_application->m_data_center->write("COMMANDE_REINIT_XY_TETA_TxSync", 0);
                        }
                    if(id.compare("Init_rack")==0)
                    {
                        msg="Commande init Rack";
                        setPlayMessage(chrono.elapsed(),"ACTION",msg);
                        m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", 1);
                        m_application->m_data_center->write("NumeroServoMoteur1", 52);
                        m_application->m_data_center->write("PositionServoMoteur1", 1);
                        m_application->m_data_center->write("VitesseServoMoteur1", 0);
                        m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", 0);
                    }
                    if(id.compare("Rack")==0)
                    {
                        if(nb_args==1)
                        {
                            float consigne=args.at(0).toInt();
                            int sens_position=0;
                            if(consigne>=0)
                                sens_position=5;
                            else
                                sens_position=15;
                            float consigne_messagerie=qRound(qAbs(consigne)/10.0);
                            msg="Commande rack à la valeur "+value;
                            setPlayMessage(chrono.elapsed(),"ACTION",msg);
                            m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", true);
                            m_application->m_data_center->write("NumeroServoMoteur1", 50);
                            m_application->m_data_center->write("PositionServoMoteur1", consigne_messagerie);
                            m_application->m_data_center->write("VitesseServoMoteur1", sens_position);
                            m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", false);
                        }
                    }
                }

                if(sActuator.compare("Node")==0)
                {
                    msg="ACTION VIDE "+getSateNameText(table_sequence,indexItem)+" : on ne fait rien!";
                    setPlayMessage(chrono.elapsed(),"ACTION",msg);
                }

                if(sActuator.compare("Wait")==0) //TEMPO
                {
                    value=getValueText(table_sequence,indexItem);
                    if(isMutliplesTransitions)
                    {
                        if(timeoutList.count()<(idxTransition+1))
                        {
                            timeoutList << value.toInt();
                            nextStateList << nextSate;
                            msg="Multiples transitions (priorité "+strIdxTransitions+"): attente de "+value+" ms";
                            //qDebug() << msg;
                            setPlayMessage(chrono.elapsed(),"TRANSITION",msg);
                        }
                    }
                    else
                    {
                        if(!goToNextValidStep)
                        {
                            msg="Attente de "+value+" ms";
                            setPlayMessage(chrono.elapsed(),"TRANSITION",msg);
                            QTest::qWait(value.toInt()); //no waiting if invalid cast
                        }
                    }
                }

                if(sActuator.compare("Event")==0) //EVENEMENT
                {
                    //flag de convergence
                    bool bConv=false;

                    id=getIdText(table_sequence,indexItem);
                    value=getValueText(table_sequence,indexItem);

                    if(isMutliplesTransitions)
                    {
                        if(timeoutList.count()<(idxTransition+1))
                        {
                            timeoutList << value.toInt();
                            nextStateList << nextSate;
                            msg="Multiples transitions (priorité "+strIdxTransitions+"): attente de convergence asservissement OU "+value+" ms";
                            //qDebug() << msg;
                            setPlayMessage(chrono.elapsed(),"TRANSITION",msg);
                        }
                        if(id.compare("convAsserv")==0) // événement de convergence d'asservissement
                        {
                            if(m_application->m_data_center->read("Convergence").toInt()==cCONVERGENCE_OK)
                            {
                                isMutliplesTransitions=false;
                                goToNextValidStep=true;
                            }
                        }
                        if(id.compare("convRack")==0) // événement de convergence de l'asenceur
                        {
                            if(m_application->m_data_center->read("rack_convergence").toBool())
                            {
                                isMutliplesTransitions=false;
                                goToNextValidStep=true;
                            }
                        }
                    }
                    else
                    {
                        if(!goToNextValidStep)
                        {
                            //init du timeout et du compteur associe
                            short cmptTe=0;
                            int timeOut=value.toInt();

                            if(id.compare("convAsserv")==0) // événement de convergence d'asservissement
                            {
                                bConv=false;
                                msg="Attente de convergence asservissement OU "+value+" ms";
                                setPlayMessage(chrono.elapsed(),"TRANSITION",msg);
                                while(!bConv && !bResume && !bStop && (cmptTe<(timeOut/40)))
                                {
                                    QTest::qWait(40);
                                    if((m_application->m_data_center->read("Convergence").toInt()==cCONVERGENCE_OK)&& (cmptTe>10))
                                        bConv=true;
                                    cmptTe++;
                                }
                            }

                            if(id.compare("convRack")==0) // événement de convergence de l'asenceur
                            {
                                bConv=false;
                                msg="Attente de convergence rack OU "+value+" ms";
                                setPlayMessage(chrono.elapsed(),"TRANSITION",msg);
                                while((!bConv && !bResume && !bStop) && (cmptTe<(timeOut/40)))
                                {
                                    QTest::qWait(40);
                                    if((m_application->m_data_center->read("rack_convergence").toBool()) && (cmptTe>10))
                                    {
                                        //qDebug() << m_application->m_data_center->read("rack_convergence").toBool() << "Timeout "<<timeOut/40<<" => "<<cmptTe;
                                        bConv=true;
                                    }
                                    cmptTe++;
                                }
                            }
                        }
                    }
                }

                if(sActuator.contains("Sensor")) //CAPTEUR
                {
                    //flag d'evenement
                    bool bReachCondition=false;

                    id=getIdText(table_sequence,indexItem);
                    value=getValueText(table_sequence,indexItem);

    //                //init du timeout et du compteur associe
    //                short cmptTe=0;
    //                int timeOut=value.toInt();
                    if(isMutliplesTransitions)
                    {
                        if(timeoutList.count()<(idxTransition+1))
                        {
                            timeoutList << 3600000;
                            nextStateList << nextSate;
                            if(sActuator.compare("Sensor_sup")==0)
                                msg="Multiples transitions (priorité "+strIdxTransitions+"): attente de "+id+" > "+value;
                            if(sActuator.compare("Sensor_egal")==0)
                                msg="Multiples transitions (priorité "+strIdxTransitions+"): attente de "+id+" = "+value;
                            if(sActuator.compare("Sensor_inf")==0)
                                msg="Multiples transitions (priorité "+strIdxTransitions+"): attente de "+id+" < "+value;
                            //qDebug() << msg;
                            setPlayMessage(chrono.elapsed(),"TRANSITION",msg);
                        }

                        if(sActuator.compare("Sensor_sup")==0)
                        {
                            if(m_application->m_data_center->read(id).toFloat()> value.toFloat())
                            {
                                isMutliplesTransitions=false;
                                goToNextValidStep=true;
                            }

                        }

                        if(sActuator.compare("Sensor_egal")==0)
                        {
                             if(m_application->m_data_center->read(id).toFloat()== value.toFloat())
                             {
                                 isMutliplesTransitions=false;
                                 goToNextValidStep=true;
                             }

                        }

                        if(sActuator.compare("Sensor_inf")==0)
                        {
                             if(m_application->m_data_center->read(id).toFloat()< value.toFloat())
                             {
                                 isMutliplesTransitions=false;
                                 goToNextValidStep=true;
                             }

                        }
                    }
                    else
                    {
                        if(!goToNextValidStep)
                        {
                            if(sActuator.compare("Sensor_sup")==0)
                            {
                                bReachCondition=false;
                                msg="Attente de "+id+" > "+value;
                                setPlayMessage(chrono.elapsed(),"TRANSITION",msg);
                                while(!bReachCondition && !bResume && !bStop)// && (cmptTe<(timeOut/40)))
                                {
                                    QTest::qWait(40);
                                    if(m_application->m_data_center->read(id).toFloat()> value.toFloat()) //&& (cmptTe>10))
                                        bReachCondition=true;
                                    //cmptTe++;
                                }
                            }

                            if(sActuator.compare("Sensor_egal")==0)
                            {
                                bReachCondition=false;
                                msg="Attente de "+id+" = "+value;
                                setPlayMessage(chrono.elapsed(),"TRANSITION",msg);
                                while(!bReachCondition && !bResume && !bStop)// && (cmptTe<(timeOut/40)))
                                {
                                    QTest::qWait(40);
                                    if(m_application->m_data_center->read(id).toFloat()== value.toFloat()) //&& (cmptTe>10))
                                        bReachCondition=true;
                                    //cmptTe++;
                                }
                            }

                            if(sActuator.compare("Sensor_inf")==0)
                            {
                                bReachCondition=false;
                                msg="Attente de "+id+" < "+value;
                                setPlayMessage(chrono.elapsed(),"TRANSITION",msg);
                                while(!bReachCondition && !bResume && !bStop)// && (cmptTe<(timeOut/40)))
                                {
                                    QTest::qWait(40);
                                    if(m_application->m_data_center->read(id).toFloat()< value.toFloat()) //&& (cmptTe>10))
                                        bReachCondition=true;
                                    //cmptTe++;
                                }
                            }
                        }
                    }
                }

                 //action sur le bouton de pause
                while(bResume)
                    QTest::qWait(40);

                //action bouton stop
                if (bStop)
                {
                    //on arrête tous les actionneurs
                    m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 1);
                    m_application->m_data_center->write("cde_moteur_1", 0);
                    m_application->m_data_center->write("cde_moteur_2", 0);
                    m_application->m_data_center->write("cde_moteur_3", 0);
                    m_application->m_data_center->write("cde_moteur_4", 0);
                    m_application->m_data_center->write("cde_moteur_5", 0);
                    m_application->m_data_center->write("cde_moteur_6", 0);
                    m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 0);
                    m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 1);
                    m_application->m_data_center->write("PowerSwitch_xt1", 0);
                    m_application->m_data_center->write("PowerSwitch_xt2", 0);
                    m_application->m_data_center->write("PowerSwitch_xt3", 0);
                    m_application->m_data_center->write("PowerSwitch_xt4", 0);
                    m_application->m_data_center->write("PowerSwitch_xt5", 0);
                    m_application->m_data_center->write("PowerSwitch_xt6", 0);
                    m_application->m_data_center->write("PowerSwitch_xt7", 0);
                    m_application->m_data_center->write("PowerSwitch_xt8", 0);
                    m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 0);
                    m_application->m_data_center->write("COMMANDE_MVT_MANUEL_TxSync", 1);
                    m_application->m_data_center->write("PuissanceMotD", 0);
                    m_application->m_data_center->write("PuissanceMotG", 0);
                    m_application->m_data_center->write("COMMANDE_MVT_MANUEL_TxSync", 0);
                    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", 1);
                    m_application->m_data_center->write("NumeroServoMoteur1", 50);
                    m_application->m_data_center->write("PositionServoMoteur1", 10);
                    m_application->m_data_center->write("VitesseServoMoteur1", 0);
                    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", 0);

                    //on stoppe le déroulement
                    bPlay=false;
                    bResume=false;
                    nextSate=-1;

                    //mise en forme de l'ihm
                    formatSequence(table_sequence);
                    m_ihm.ui.label_showNbSequence->clear();

                    //on réactive les autres stratégies
                    for(int i=0;i<numberOfIndex;i++)
                        m_ihm.ui.tW_TabSequences->widget(i)->setEnabled(true);

                    //on met les boutons dans un état cohérent
                    update_sequenceButtons();

                    //on sort de la boucle de déroulement
                    break;
                } //fin action bouton stop

                //Compteur pour multiples transitions
                if(isMutliplesTransitions)
                {
                    QTest::qWait(40);
                    cptTransition++;
                    int timeoutTransition=cptTransition*40;
                    for(int i=0;i<timeoutList.count();i++)
                    {
                        if(timeoutList.at(i)<timeoutTransition)
                        {
                            isMutliplesTransitions=false;
                            goToNextValidStep=true;
                            nextSate=nextStateList.at(i);
                            //qDebug() << "timeout sur la transition " << i+1 << " go to " << nextSate;
                        }
                    }
                }

                if(isMutliplesTransitions)
                {
                    if(isTransition(sNextActuator))
                    {
                        idxTransition++;
                        indexItem++;
                    }
                    else
                    {
                        indexItem=indexItem-idxTransition;
                        idxTransition=0;
                    }
                    //qDebug()<<"Prochaine transition n°" << idxTransition <<" ligne "<< indexItem;
                }
                else
                {
                    if(nextSate>=0)
                    {
                        indexItem=nextSate;
                        msg="Saut vers l'état "+getSateNameText(table_sequence,indexItem)+"\n";
                        //qDebug() << msg;
                        setPlayMessage(chrono.elapsed(),"TRANSITION",msg);
                        nextSate=-1;
                    }
                    else
                        indexItem++;
                }
            } //fin déroulement de la séquence
        } //fin de la répétition de déroulement de la séquence
        else
        {
            msg="La Stratégie ne contient aucune action ou transition!";
            setPlayMessage(chrono.elapsed(),"WARNING",msg);
        }

        //on remet en forme l'ihm à la fin de la séquence
        formatSequence(table_sequence);
    }

    msg="FIN STRATEGIE "+m_ihm.ui.strategyName->text();
    setPlayMessage(chrono.elapsed(),"INFO",msg);

    //on remet en forme l'ihm à la fin de la séquence
    m_ihm.ui.label_showNbSequence->clear();

    //on réactive les autres stratégies
    for(int i=0;i<numberOfIndex;i++)
        m_ihm.ui.tW_TabSequences->widget(i)->setEnabled(true);

    //on stoppe la sequence
    bStop=true;
    bPlay=false;
    bResume=false;

    m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 1);
    m_application->m_data_center->write("cde_moteur_1", 0);
    m_application->m_data_center->write("cde_moteur_2", 0);
    m_application->m_data_center->write("cde_moteur_3", 0);
    m_application->m_data_center->write("cde_moteur_4", 0);
    m_application->m_data_center->write("cde_moteur_5", 0);
    m_application->m_data_center->write("cde_moteur_6", 0);
    m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 0);
    m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 1);
    m_application->m_data_center->write("PowerSwitch_xt1", 0);
    m_application->m_data_center->write("PowerSwitch_xt2", 0);
    m_application->m_data_center->write("PowerSwitch_xt3", 0);
    m_application->m_data_center->write("PowerSwitch_xt4", 0);
    m_application->m_data_center->write("PowerSwitch_xt5", 0);
    m_application->m_data_center->write("PowerSwitch_xt6", 0);
    m_application->m_data_center->write("PowerSwitch_xt7", 0);
    m_application->m_data_center->write("PowerSwitch_xt8", 0);
    m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 0);
    m_application->m_data_center->write("COMMANDE_MVT_MANUEL_TxSync", 1);
    m_application->m_data_center->write("PuissanceMotD", 0);
    m_application->m_data_center->write("PuissanceMotG", 0);
    m_application->m_data_center->write("COMMANDE_MVT_MANUEL_TxSync", 0);
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", 1);
    m_application->m_data_center->write("NumeroServoMoteur1", 50);
    m_application->m_data_center->write("PositionServoMoteur1", 10);
    m_application->m_data_center->write("VitesseServoMoteur1", 0);
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", 0);

    //on met les boutons dans un état cohérent
    update_sequenceButtons();

    //on sélectionne la première ligne
    table_sequence->selectRow(qMin(idxMax+1,table_sequence->rowCount()-1));
}



void CActuatorSequencer::Slot_Resume()
{
    //bStop=false;
    //bPlay=false;
    bResume=!bResume;

    //update_sequenceButtons();

}



void CActuatorSequencer::Slot_Stop()
{
    bStop=true;
    bPlay=false;
    bResume=false;

    update_sequenceButtons();

    int numberOfIndex=m_ihm.ui.tW_TabSequences->count();
    for(int i=0;i<numberOfIndex;i++)
        m_ihm.ui.tW_TabSequences->widget(i)->setEnabled(true);


}

void CActuatorSequencer::update_sequenceButtons()
{
    m_ihm.ui.pB_Play->setEnabled(!bPlay);
    m_ihm.ui.pB_Resume->setEnabled(!bResume);
    m_ihm.ui.pB_Stop->setEnabled(!bStop);
}

void CActuatorSequencer::Slot_Generate_XML(QString strPath)
{
    int indexItem=0;
    int tabIndex=m_ihm.ui.tW_TabSequences->currentIndex();
    QTableWidget * table_sequence=listSequence.at(tabIndex);

    QDomDocument doc("sequence_xml");
    // root node
    QDomElement strategieNode = doc.createElement("sequence");
    strategieNode.setAttribute("name",m_ihm.ui.strategyName->text());
    doc.appendChild(strategieNode);

    //commentaire global
//    QDomElement commentsNode = doc.createElement("commentaires");
//    QTextEdit *textEdit_comment=m_ihm.findChild<QTextEdit*>("textEdit_comment");
//    commentsNode.appendChild(doc.createTextNode(textEdit_comment->toPlainText()));
//    strategieNode.appendChild(commentsNode);

    for (indexItem=0;indexItem<table_sequence->rowCount();indexItem++)
    {
        QDomElement ligneNode = doc.createElement("ligne");
        ligneNode.setAttribute("type",getTypeText(table_sequence,indexItem));
        ligneNode.setAttribute("id",getIdText(table_sequence,indexItem));
        ligneNode.setAttribute("value",getValueText(table_sequence,indexItem));
        ligneNode.setAttribute("state",getSateNameText(table_sequence,indexItem));
        ligneNode.setAttribute("comments",getCommentsText(table_sequence,indexItem));
        QString strSym="false";
        if(getSymChecked(table_sequence,indexItem))
            strSym="true";
        ligneNode.setAttribute("symetrie",strSym);
        QString strProto="false";
        if(getProtoChecked(table_sequence,indexItem))
            strProto="true";
        ligneNode.setAttribute("prototype",strProto);
        strategieNode.appendChild(ligneNode);
    }


    if(!strPath.isEmpty())
    {
        QFile fichier(strPath);
        if(fichier.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
        {
            QTextStream stream(&fichier);
            QString contenuXML=doc.toString();
            stream << contenuXML;
            fichier.close();
        }
        else
            fichier.close();
    }
}


void CActuatorSequencer::Slot_Save()
{
    // Creation d'un objet XML
    QString ficName("savedSequence_");
    QString temps2=QTime::currentTime().toString("hh_mm");
    QString temps1 = QDate::currentDate().toString("yyyy_MM_dd_at_");
    ficName.append(temps1);
    ficName.append(temps2);
    ficName.append(".xml");
    //qDebug() << ficName;
    QString caption("Open XML Strategie File");
    QString filter("XML Files (*.xml)");
    QString fileName = QFileDialog::getSaveFileName(&m_ihm,caption, ficName,filter);

    Slot_Generate_XML(fileName);
}

void CActuatorSequencer::Slot_Load()
{
    int indexItem=0;
    /*int tabIndex=m_ihm.ui.tW_TabSequences->currentIndex();
    QTableWidget * table_sequence=listSequence.at(tabIndex);*/


    QString caption("Open XML Strategie File");
    QString filter("XML Files (*.xml)");
    QString fileName = QFileDialog::getOpenFileName(&m_ihm,caption, "",filter);

    indexItem=0;
    if(fileName.isEmpty()==false)
    {
        QFile fichier(fileName);
        if(fichier.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTableWidget * table_sequence=Slot_Add_Sequence();
            int nb_lignes=table_sequence->rowCount();
            for (int i=nb_lignes-1;i>=0;i--)
                table_sequence->removeRow(i);

            //qDebug() << "remove" << nb_lignes << "rows";

            QDomDocument doc("sequence_xml");
            if (doc.setContent(&fichier)) {
                QDomElement docElem = doc.documentElement();
                if(docElem.tagName().compare("sequence")==0)
                {
                    QString strateName=docElem.attribute("name","myStrategy");
                    m_ihm.ui.tW_TabSequences->setTabText(m_ihm.ui.tW_TabSequences->currentIndex(),strateName);
                    m_ihm.ui.strategyName->setText(strateName);
                }
                QDomNode n = docElem.firstChild();
                while(!n.isNull()) {
                    QDomElement e = n.toElement();
                    if(!e.isNull())
                    {
                        //qDebug() << e.tagName();
//                        if(e.tagName().compare("commentaires")==0)
//                        {
//                            QTextEdit *textEdit_comment=m_ihm.findChild<QTextEdit*>("textEdit_comment");
//                            textEdit_comment->setPlainText(e.text());
//                        }
                        if(e.tagName().compare("sequence")==0)
                        {
                            QString strateName=e.attribute("name","myStrategy");
                            //qDebug()<<strateName;
                        }

                        if(e.tagName().compare("ligne")==0)
                        {
                            QTableWidgetItem *newItem_type = new QTableWidgetItem(e.attribute("type","Wait"));
                            QTableWidgetItem *newItem_id = new QTableWidgetItem(e.attribute("id",""));
                            QTableWidgetItem *newItem_value = new QTableWidgetItem(e.attribute("value","1000"));
                            QTableWidgetItem *newItem_state = new QTableWidgetItem(e.attribute("state",""));
                            QTableWidgetItem *newItem_comments = new QTableWidgetItem(e.attribute("comments",""));
                            QString strSym = e.attribute("symetrie","false");
                            QString strProto = e.attribute("prototype","false");
                            bool bSym=false;
                            if(strSym=="true")
                                    bSym=true;
                            bool bProto=false;
                            if(strProto=="true")
                                    bProto=true;


                            if(isTransition(e.attribute("type","Wait")))
                            {
                                newItem_type->setBackgroundColor(Qt::cyan);
                                newItem_id->setBackgroundColor(Qt::cyan);
                                newItem_value->setBackgroundColor(Qt::cyan);
                                newItem_state->setBackgroundColor(Qt::cyan);
                                newItem_comments->setBackgroundColor(Qt::cyan);
                            }

                            table_sequence->insertRow(indexItem);
                            fillRow(table_sequence,indexItem,newItem_type,newItem_id,newItem_value,newItem_state,newItem_comments,bSym,bProto);
                            //qDebug() << "insert" << e.attribute("type","Wait") << "at row number" << indexItem;
                            indexItem++;
                        }
                    }
                    n = n.nextSibling();
                }
            }
            fichier.close();
        }
        else
            fichier.close();
    }
}

void CActuatorSequencer::Slot_Clear()
{
    int indexItem=0;
    int tabIndex=m_ihm.ui.tW_TabSequences->currentIndex();
    QTableWidget * table_sequence=listSequence.at(tabIndex);

    int nb_lignes=table_sequence->rowCount();
    for (indexItem=nb_lignes-1;indexItem>=0;indexItem--)
        table_sequence->removeRow(indexItem);
}

void CActuatorSequencer::Slot_Play_only_SD20()
{
    QString id;
    QString value;
    id.setNum(m_ihm.ui.cB_SD20->currentIndex()+13);
    value.setNum(m_ihm.ui.sB_SD20->value());

    qDebug() << "SD20 number" << id << "at" <<value;
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", true);
    m_application->m_data_center->write("NumeroServoMoteur1", id);
    m_application->m_data_center->write("PositionServoMoteur1", value);
    m_application->m_data_center->write("VitesseServoMoteur1", 0);
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", false);
}

void CActuatorSequencer::Slot_Play_only_AX()
{



    QString id;
    QString value;
    id.setNum(m_ihm.ui.cB_AX->currentIndex());
    value.setNum(m_ihm.ui.sB_AX->value());

    qDebug() << "AX number" << id <<m_ihm.ui.cB_AX_type_cde->currentText() <<"at" <<value;
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", true);
    m_application->m_data_center->write("num_servo_ax", id);
    m_application->m_data_center->write("commande_ax", m_ihm.ui.cB_AX_type_cde->currentData());
    m_application->m_data_center->write("valeur_commande_ax", value);
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", false);
}

void CActuatorSequencer::Slot_Play_only_motor()
{
    QString id;
    QString value;
    id.setNum(m_ihm.ui.cB_Motor->currentIndex()+1);
    value.setNum(m_ihm.ui.sB_Motor->value());

    QString str_name=QString("cde_moteur_%1").arg(id);
    qDebug() << "Motor" << m_ihm.ui.cB_Motor->currentText() << "at" <<value;
    m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 1);
    m_application->m_data_center->write(str_name, value);
    m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 0);
}

void CActuatorSequencer::Slot_Play_only_power()
{
    QString id;
    QString value;
    id.setNum(m_ihm.ui.cB_Power->currentIndex()+1);
    value.setNum(m_ihm.ui.sB_Power->value());

    bool bvalue=false;
    if(value.compare("1")==0)
        bvalue=true;

    //bvalue=(value.compare("1")==0?true:false);

    QString str_name=QString("PowerSwitch_xt%1").arg(id);
    qDebug() << "PowerSwitch" << m_ihm.ui.cB_Power->currentText() << "at" <<bvalue;
    m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 1);
    m_application->m_data_center->write(str_name, bvalue);
    m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 0);
}

void CActuatorSequencer::Slot_Stop_only_motors()
{
   qDebug() << "Stop all motors";
    m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 1);
    m_application->m_data_center->write("cde_moteur_1", 0);
    m_application->m_data_center->write("cde_moteur_2", 0);
    m_application->m_data_center->write("cde_moteur_3", 0);
    m_application->m_data_center->write("cde_moteur_4", 0);
    m_application->m_data_center->write("cde_moteur_5", 0);
    m_application->m_data_center->write("cde_moteur_6", 0);
    m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 0);
}

void CActuatorSequencer::Slot_Stop_only_power()
{
   qDebug() << "Stop all powerswitch";
m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 1);
m_application->m_data_center->write("PowerSwitch_xt1", false);
m_application->m_data_center->write("PowerSwitch_xt2", false);
m_application->m_data_center->write("PowerSwitch_xt3", false);
m_application->m_data_center->write("PowerSwitch_xt4", false);
m_application->m_data_center->write("PowerSwitch_xt5", false);
m_application->m_data_center->write("PowerSwitch_xt6", false);
m_application->m_data_center->write("PowerSwitch_xt7", false);
m_application->m_data_center->write("PowerSwitch_xt8", false);
m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 0);
}

void CActuatorSequencer::Slot_Play_only_asser()
{
    QString id;
    QList<float> float_values;
    id=m_ihm.ui.cB_Asser->currentText();

    if(m_ihm.ui.lE_Asser_1->isEnabled())
        float_values << m_ihm.ui.lE_Asser_1->text().toFloat();
    if(m_ihm.ui.lE_Asser_2->isEnabled())
        float_values << m_ihm.ui.lE_Asser_2->text().toFloat();
    if(m_ihm.ui.lE_Asser_3->isEnabled())
        float_values << m_ihm.ui.lE_Asser_3->text().toFloat();

    int nb_args=float_values.count();
    //qDebug() << id << nb_args << float_values;
    //"XY","DistAng","Rack"
    if(id=="XY")
        if(nb_args==2)
        {
            qDebug() << "MVT XY" << float_values;
            m_application->m_data_center->write("COMMANDE_MVT_XY_TxSync", 1);
            m_application->m_data_center->write("X_consigne", float_values.at(0));
            m_application->m_data_center->write("Y_consigne", float_values.at(1));
            m_application->m_data_center->write("COMMANDE_MVT_XY_TxSync", 0);
        }
    if(id=="XYTheta")
        if(nb_args==3)
        {
            qDebug() << "MVT XYTheta" << float_values;
            m_application->m_data_center->write("COMMANDE_MVT_XY_TETA_TxSync", 1);
            m_application->m_data_center->write("XYT_X_consigne", float_values.at(0));
            m_application->m_data_center->write("XYT_Y_consigne", float_values.at(1));
            m_application->m_data_center->write("XYT_angle_consigne", float_values.at(2));
            m_application->m_data_center->write("COMMANDE_MVT_XY_TETA_TxSync", 0);
        }

    if(id=="DistAng")
        if(nb_args==2)
        {
            qDebug() << "MVT Distance Angle" << float_values;
            m_application->m_data_center->write("COMMANDE_DISTANCE_ANGLE_TxSync", 1);
            m_application->m_data_center->write("distance_consigne",float_values.at(0));
            m_application->m_data_center->write("angle_consigne", float_values.at(1));
            m_application->m_data_center->write("COMMANDE_DISTANCE_ANGLE_TxSync", 0);
        }
    if(id=="Init_Asser")
        if(nb_args==3)
        {
            qDebug() << "Init X Y TETA" << float_values;
            m_application->m_data_center->write("COMMANDE_REINIT_XY_TETA_TxSync", 1);
            m_application->m_data_center->write("reinit_x_pos",float_values.at(0));
            m_application->m_data_center->write("reinit_y_pos", float_values.at(1));
            m_application->m_data_center->write("reinit_teta_pos", float_values.at(2));
            m_application->m_data_center->write("COMMANDE_REINIT_XY_TETA_TxSync", 0);
        }
    if(id=="Init_rack")
    {
        qDebug() << "INIT RACK" << float_values;
        m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", 1);
        m_application->m_data_center->write("NumeroServoMoteur1", 52);
        m_application->m_data_center->write("PositionServoMoteur1", 1);
        m_application->m_data_center->write("VitesseServoMoteur1", 0);
        m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", 0);
    }

    if(id=="Rack")
        if(nb_args==1)
        {
            float consigne=float_values.at(0);
            int sens_position=0;
            if(consigne>=0)
                sens_position=5;
            else
                sens_position=15;
            float consigne_messagerie=qRound(qAbs(consigne)/10.0);
            qDebug() << "MVT Rack" << float_values;
            m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", true);
            m_application->m_data_center->write("NumeroServoMoteur1", 50);
            m_application->m_data_center->write("PositionServoMoteur1",consigne_messagerie );
            m_application->m_data_center->write("VitesseServoMoteur1", sens_position);
            m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", false);

        }

}

void CActuatorSequencer::Slot_Stop_only_asser()
{
    qDebug() << "Stop all asser";
    m_application->m_data_center->write("COMMANDE_MVT_MANUEL_TxSync", 1);
    m_application->m_data_center->write("PuissanceMotD", 0);
    m_application->m_data_center->write("PuissanceMotG", 0);
    m_application->m_data_center->write("COMMANDE_MVT_MANUEL_TxSync", 0);
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", true);
    m_application->m_data_center->write("NumeroServoMoteur1", 51);
    m_application->m_data_center->write("PositionServoMoteur1", 1);
    m_application->m_data_center->write("VitesseServoMoteur1", 0);
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", false);
}

QTableWidget* CActuatorSequencer::Slot_Add_Sequence()
{ 
    QTableWidget * last_table_sequence=listSequence.last();
    int lastIndex=listSequence.count()-1;

    QTableWidget * newSequence = new QTableWidget();
    QWidget * newTab=new QWidget();
    QString tabLabel=QString("Sequence%1").arg(listSequence.count());
    creatTabSequence(newTab,newSequence,tabLabel,false);

    if(last_table_sequence->rowCount()<=0)
        this->Slot_Remove_Sequence(lastIndex);

    m_ihm.ui.tW_TabSequences->setCurrentWidget(newTab);

    return newSequence;
}

void CActuatorSequencer::Slot_Delete()
{
    Slot_Remove_Sequence(m_ihm.ui.tW_TabSequences->currentIndex());
}

void CActuatorSequencer::Slot_Clone()
{
    int index2Clone=0;
    index2Clone=m_ihm.ui.tW_TabSequences->currentIndex();
    QString strName=m_ihm.ui.strategyName->text();
    QTableWidget * sequence2Clone=listSequence.at(index2Clone);

    if(sequence2Clone->rowCount()>0)//on ne clone que les sequences non vides
    {
        QTableWidget * last_table_sequence=listSequence.last();
        int lastIndex=listSequence.count()-1;

        //creation sequence vide
        QTableWidget * newSequence= new QTableWidget();
        QWidget * newTab=new QWidget();
        QString tabLabel=strName+"_cloned";
        creatTabSequence(newTab,newSequence,tabLabel,false);

        //on remplit la sequence
        for(int i=0;i<sequence2Clone->rowCount();i++)
        {
            QTableWidgetItem *newItem_type = new QTableWidgetItem(getTypeText(sequence2Clone,i));
            QTableWidgetItem *newItem_id = new QTableWidgetItem(getIdText(sequence2Clone,i));
            QTableWidgetItem *newItem_value = new QTableWidgetItem(getValueText(sequence2Clone,i));
            QTableWidgetItem *newItem_state = new QTableWidgetItem(getSateNameText(sequence2Clone,i));
            QTableWidgetItem *newItem_comments = new QTableWidgetItem(getCommentsText(sequence2Clone,i));
            bool bSym=getSymChecked(sequence2Clone,i);
            bool bProto=getProtoChecked(sequence2Clone,i);

            if(isTransition(getTypeText(sequence2Clone,i)))
            {
                newItem_type->setBackgroundColor(Qt::cyan);
                newItem_id->setBackgroundColor(Qt::cyan);
                newItem_value->setBackgroundColor(Qt::cyan);
                newItem_state->setBackgroundColor(Qt::cyan);
                newItem_comments->setBackgroundColor(Qt::cyan);
            }

            newSequence->insertRow(i);
            fillRow(newSequence,i,newItem_type,newItem_id,newItem_value,newItem_state,newItem_comments,bSym,bProto);
        }

        if(last_table_sequence->rowCount()<=0)
            this->Slot_Remove_Sequence(lastIndex);

        m_ihm.ui.tW_TabSequences->setCurrentWidget(newTab);
    }

}

void CActuatorSequencer::Slot_Remove_Sequence(int index)
{
    if(m_ihm.ui.tW_TabSequences->count()>1)
    {
        listSequence.removeAt(index);
        listChoice.removeAt(index);
        for(int i=0;i<strategies2Combine.count();i++)
        {
            if(strategies2Combine.at(i)==index)
            {
                strategies2Combine.removeAt(i);
                delete m_ihm.ui.list_Strategies->takeItem(i);
                break;
            }
        }
        m_ihm.ui.tW_TabSequences->removeTab(index);
    }
}

void CActuatorSequencer::Slot_convert_to_rad()
{
    float deg_value=m_ihm.ui.tool_deg->text().toFloat();
    QString rad_value;
    rad_value.setNum(deg_value*3.14/360.0);
    m_ihm.ui.tool_rad->setText(rad_value);
}
void CActuatorSequencer::Slot_convert_to_deg()
{
    float rad_value=m_ihm.ui.tool_rad->text().toFloat();
    QString deg_value;
    float rad_float=rad_value*360.0/3.14;
    deg_value.setNum(rad_float);
    m_ihm.ui.tool_deg->setText(deg_value);
}

void CActuatorSequencer::Slot_get_XYTeta()
{
    QString id=m_ihm.ui.cB_Asser->currentText();
    QVariant val;
    if(id=="XY")
    {
        val = m_application->m_data_center->read("PosX_robot");
        m_ihm.ui.lE_Asser_1->setValue(val.toInt());
        val = m_application->m_data_center->read("PosY_robot");
        m_ihm.ui.lE_Asser_2->setValue(val.toInt());
    }
    if(id=="XYTheta")
    {
        val = m_application->m_data_center->read("PosX_robot");
        m_ihm.ui.lE_Asser_1->setValue(val.toInt());
        val = m_application->m_data_center->read("PosY_robot");
        m_ihm.ui.lE_Asser_2->setValue(val.toInt());
        val = m_application->m_data_center->read("PosTeta_robot");
        m_ihm.ui.lE_Asser_3->setValue(val.toDouble());
    }

    if(id=="DistAng")
    {
        val = m_application->m_data_center->read("DirDistance_robot");
        m_ihm.ui.lE_Asser_1->setValue(val.toInt());
        val = m_application->m_data_center->read("PosTeta_robot");
        m_ihm.ui.lE_Asser_3->setValue(val.toDouble());
    }
}

void CActuatorSequencer::Slot_up()
{
    int tabIndex=m_ihm.ui.tW_TabSequences->currentIndex();
    QTableWidget * currentSequence=listSequence.at(tabIndex);

    QList<QTableWidgetItem*> list_item_selected=currentSequence->selectedItems();
    if(!list_item_selected.isEmpty())
    {
        QTableWidgetItem* item_selected=list_item_selected.first();
        int row_selected=item_selected->row();
        if(row_selected>0) //avoid negative index
        {
            int colCount=currentSequence->columnCount();

            //qDebug() << colCount;

            int row_target=row_selected-1; //up the list, dow the index ;-)

            QList<QTableWidgetItem*> rowItems,rowItems1;

            for (int col = 0; col < colCount; ++col)
            {
                rowItems << currentSequence->takeItem(row_selected, col);
                rowItems1 << currentSequence->takeItem(row_target, col);
            }
            bool bSymSelected=getSymChecked(currentSequence,row_selected);
            bool bSymTarget=getSymChecked(currentSequence,row_target);
            bool bProtoSelected=getProtoChecked(currentSequence,row_selected);
            bool bProtoTarget=getProtoChecked(currentSequence,row_target);

            for (int cola = 0; cola < colCount; ++cola)
            {
                currentSequence->setItem(row_target, cola, rowItems.at(cola));
                currentSequence->setItem(row_selected, cola, rowItems1.at(cola));
            }
            setSymChecked(currentSequence,row_target,bSymSelected);
            setSymChecked(currentSequence,row_selected,bSymTarget);
            setProtoChecked(currentSequence,row_target,bProtoSelected);
            setProtoChecked(currentSequence,row_selected,bProtoTarget);

            currentSequence->setCurrentCell(row_target,0);

        }
    }
}
void CActuatorSequencer::Slot_down()
{
    int tabIndex=m_ihm.ui.tW_TabSequences->currentIndex();
    QTableWidget * currentSequence=listSequence.at(tabIndex);

    QList<QTableWidgetItem*> list_item_selected=currentSequence->selectedItems();
    if(!list_item_selected.isEmpty())
    {
        QTableWidgetItem* item_selected=list_item_selected.first();
        int row_selected=item_selected->row();
        int nbRow=currentSequence->rowCount();
        if(row_selected<(nbRow-1)) //avoid buffer over flow
        {
            int colCount=currentSequence->columnCount();

            int row_target=row_selected+1; //down the list, up the index ;-)

            QList<QTableWidgetItem*> rowItems,rowItems1;

            for (int col = 0; col < colCount; ++col)
            {
                rowItems << currentSequence->takeItem(row_selected, col);
                rowItems1 << currentSequence->takeItem(row_target, col);
            }
            bool bSymSelected=getSymChecked(currentSequence,row_selected);
            bool bSymTarget=getSymChecked(currentSequence,row_target);
            bool bProtoSelected=getProtoChecked(currentSequence,row_selected);
            bool bProtoTarget=getProtoChecked(currentSequence,row_target);

            for (int cola = 0; cola < colCount; ++cola)
            {
                currentSequence->setItem(row_target, cola, rowItems.at(cola));
                currentSequence->setItem(row_selected, cola, rowItems1.at(cola));
            }
            setSymChecked(currentSequence,row_target,bSymSelected);
            setSymChecked(currentSequence,row_selected,bSymTarget);
            setProtoChecked(currentSequence,row_target,bProtoSelected);
            setProtoChecked(currentSequence,row_selected,bProtoTarget);

            currentSequence->setCurrentCell(row_target,0);

        }
    }
}

void CActuatorSequencer::setEnableRepetition(bool state)
{
    m_ihm.ui.sB_numberOfSequence->setEnabled(state);
    m_ihm.ui.label_nbSequence->setEnabled(state);
}

void CActuatorSequencer::playStep(void)
{
    int tabIndex=m_ihm.ui.tW_TabSequences->currentIndex();
    QTableWidget * currentSequence=listSequence.at(tabIndex);

    int indexPlay=0;

    QList<QTableWidgetItem*> list_item_selected=currentSequence->selectedItems();
    if(!list_item_selected.isEmpty())
    {
        QTableWidgetItem* item_selected=list_item_selected.first();
        int row_selected=item_selected->row();
        if(row_selected>=0)
            indexPlay=row_selected;
    }

    //qDebug() << indexPlay;

    Slot_Play(true,indexPlay);
}

void CActuatorSequencer::Slot_Generate_CPP()
{
    int indexItem=0;
    int tabIndex=m_ihm.ui.tW_TabSequences->currentIndex();
    QTableWidget * table_sequence=listSequence.at(tabIndex);

    //utile
    QString UnamedState="STATE_%1";
    QString N1T="\n\t";
    QString N2T="\n\t\t";
    QString N3T="\n\t\t\t";
    QString stateEnumFormat=N2T+"case %1 :\t\treturn \"%1\";";
    QString stateFormat=N1T+"// ___________________________"+
            N1T+"case %1 :"+
            N2T+"if (onEntry()) {";
    QString closePreviousState=N2T+"if (onExit()) { %1 }"+
            N2T+"break;";
    QString strProto="//For prototyping only: ";

    //Nom de la strategie
    QString nomStrategie;
    if(m_ihm.ui.strategyName->text().isEmpty())
            nomStrategie="myStrategie";
    else
            nomStrategie=m_ihm.ui.strategyName->text();
    //on en déduit le nom de la classe
    QString strClassName="SM_"+nomStrategie;

    // on en déduit également les noms des fichiers de la classe
    QString ficName_cpp="sm_"+nomStrategie.toLower()+".cpp";
    QString ficName_h="sm_"+nomStrategie.toLower()+".h";

    QString caption("Generate Strategie in CPP file");
    QString filter("CPP Files (*.cpp)");
    QString fullPathFile=defaultPath+"/"+ficName_cpp;
    QString fileName_cpp = QFileDialog::getSaveFileName(&m_ihm,caption, fullPathFile,filter);

    /*
      * pour le fichier .cpp
      */

    QString strWholeFile_cpp;
    QString strComments;
    QString strInclude;
    QString strConstructor;
    QString strGetName;
    QString strState2Name;
    QString strEnumStates;
    QString strStep;

    //construction du commentaire
    QString strTime=QTime::currentTime().toString("hh_mm");
    QString strDate=QDate::currentDate().toString("dd_MM_yyyy");
    strComments="/**\n * Generated "+strDate+" at "+strTime+"\n */\n\n";

    //construction des include
    strInclude="#include \""+ficName_h+"\"\n#include \"CGlobale.h\"\n\n";

    //construction du constructeur de la classe
    strConstructor=strClassName+"::"+strClassName+"()\n{\n\tm_main_mission_type = true;\n\tm_max_score = 0;\n}\n\n";

    //construction du getName()
    strGetName="const char* "+strClassName+"::getName()\n{\n\treturn \""+strClassName+"\";\n}\n\n";

    //construction du StepToName() - A COMPLETER PENDANT LA SEQUENCE
    strState2Name="const char* "+strClassName+"::stateToName(unsigned short state)"+
            "\n{"+
            N1T+"switch(state)"+
            N1T+"{"+
            "%1"+
            N2T+"case FIN_MISSION :\treturn \"FIN_MISSION\";"+
            N1T+"}"+
            N1T+"return \"UNKNOWN_STATE\";"+
            "\n}\n\n";

    //construction du step() - A COMPLETER PENDANT LA SEQUENCE
    strStep="// _____________________________________\nvoid "+strClassName+"::step()\n{"+
            N1T+"switch (m_state)"+
            N1T+"{"+
            "\n%1\n"+
            N1T+"// ___________________________"+
            N1T+"case FIN_MISSION :"+
            N2T+"m_succes = true;"+
            N2T+"m_score = m_max_score;"+
            N2T+"stop();"+
            N2T+"break;"+
            N1T+"}"+
            "\n}\n";


    /*
     * Pour le fichier .h
     */
    QString strAddedStates;
    QString strHeader=strComments+
                    "#ifndef SM_"+nomStrategie.toUpper()+"_H\n#define SM_"+nomStrategie.toUpper()+"_H\n\n#include \"sm_statemachinebase.h\"\n\n"+
                    "class "+strClassName+" : public SM_StateMachineBase\n{\npublic:"+
                    N1T+strClassName+"();"+
                    N1T+"void step();"+
                    N1T+"const char* getName();"+
                    N1T+"const char* stateToName(unsigned short state);\n"+
                    N1T+"typedef enum {"+
                    N1T+"STATE_1 = SM_StateMachineBase::SM_FIRST_STATE,";

    QString champStrategie;

    bool isInState=false;
    int numState=0;
    bool isInTransition=false;
    int numTransition=0;
    QList<int> transitionsList;
    QString warningOnExit="";
    bool isNode=false;
    QString oldState="";

    for (indexItem=0;indexItem<table_sequence->rowCount();indexItem++)
    {
        QString sActuator=getTypeText(table_sequence,indexItem);
        QString sId=getIdText(table_sequence,indexItem);
        QString sValue=getValueText(table_sequence,indexItem);
        QString sState=getSateNameText(table_sequence,indexItem);
        QString sComments=getCommentsText(table_sequence,indexItem);
        bool isSym=getSymChecked(table_sequence,indexItem);
        bool isProto=getProtoChecked(table_sequence,indexItem);
        QString sConverted;
        bool bState=false;
        bool bTransition=false;

        //est-ce une transition ou un etat
        bTransition=isTransition(sActuator);
        bState=!bTransition;
        
        //pour récupérer des infos de la ligne juste après et gérer les transitions multiples
        QString sNextActuator="";
        if((indexItem+1)<table_sequence->rowCount())
            sNextActuator=getTypeText(table_sequence,indexItem+1);
        
        //numéro de l'état suivant
        QString strNumNextState;
        QString strNextSate;

        //recherche du prochain état si on est dans une transition afin d'en récupérer le nom
        //si le nom est vide il sera automatiquement nommé
        QString strFoundState;
        bool foundedState=false;
        for(int i=indexItem+1;i<table_sequence->rowCount();i++)
        {
            QString sType=getTypeText(table_sequence,i);
            if(!foundedState)
            {
                QString strNameState=getSateNameText(table_sequence,i);
                if(!isTransition(sType))
                {
                    strFoundState=strNameState;
                    foundedState=true;
                }
            }
        }
        if(strFoundState.isEmpty())
        {
            strNumNextState.setNum(numState+1);
            strNextSate="STATE_"+strNumNextState;
        }
        else
            strNextSate=strFoundState;

        //On parcoure la dernière ligne de la séquence, le prochain état est la FIN DE MISSION
        if(indexItem==(table_sequence->rowCount())-1)
            strNextSate="FIN_MISSION";
        if(bTransition&&(!sState.isEmpty()))
            strNextSate=sState;

        QString strConsigne;
        QStringList args;
        int nb_args=0;
        QString strFreeAction;
        //Génération de la ligne de code en fonction du type
        switch(getType(sActuator))
        {
            case SD20:
                sConverted=sConverted+(isProto?strProto:"")+QString("Application.m_servos_sd20.CommandePosition(%1,%2);/*%3*/").arg(sId).arg(sValue).arg(sComments);
                break;

            case AX_POSITION:
                sConverted=sConverted+(isProto?strProto:"")+QString("Application.m_servos_ax.setPosition(%1,%2);/*%3*/").arg(sId).arg(sValue).arg(sComments);
                break;

            case AX_SPEED:
                sConverted=sConverted+(isProto?strProto:"")+QString("Application.m_servos_ax.setSpeed(%1,%2);/*%3*/").arg(sId).arg(sValue).arg(sComments);
                break;

            case MOTOR:
                sConverted=sConverted+(isProto?strProto:"")+QString("Application.m_moteurs.CommandeVitesse(%1,%2);/*%3*/").arg(sId).arg(sValue).arg(sComments);
                break;

            case POWER:
                strConsigne = (sValue.compare("1")==0) ? "true" : "false";
                sConverted=sConverted+QString("Application.m_power_electrobot.setOutput(%1,%2);/*%3*/").arg(sId).arg(strConsigne).arg(sComments);
                break;

            case ASSER:
                args=sValue.split(",",QString::SkipEmptyParts);
                nb_args=args.count();
                if(isSym)
                {
                    if((sId.compare("XY")==0)&&(nb_args>=2))
                        sConverted=sConverted+(isProto?strProto:"")+QString("outputs()->CommandeMouvementXY_sym(%1,%2);/*%3*/").arg(args.at(0)).arg(args.at(1)).arg(sComments);
                    if((sId.compare("XYTheta")==0)&&(nb_args>=3))
                        sConverted=sConverted+(isProto?strProto:"")+QString("outputs()->CommandeMouvementXY_TETA_sym(%1,%2,%3);/*%4*/").arg(args.at(0)).arg(args.at(1)).arg(args.at(2)).arg(sComments);
                    if((sId.compare("DistAng")==0)&&(nb_args>=2))
                        sConverted=sConverted+(isProto?strProto:"")+QString("outputs()->CommandeMouvementDistanceAngle_sym(%1,%2);/*%3*/").arg(args.at(0)).arg(args.at(1)).arg(sComments);
                    if((sId.compare("Init_asser")==0)&&(nb_args>=3))
                        sConverted=sConverted+(isProto?strProto:"")+QString("outputs()->setPosition_XYTeta_sym(%1,%2,%3);/*%4*/").arg(args.at(0)).arg(args.at(1)).arg(args.at(2)).arg(sComments);
                }
                else
                {
                    if((sId.compare("XY")==0)&&(nb_args>=2))
                        sConverted=sConverted+(isProto?strProto:"")+QString("Application.m_asservissement.CommandeMouvementXY(%1,%2);/*%3*/").arg(args.at(0)).arg(args.at(1)).arg(sComments);
                    if((sId.compare("XYTheta")==0)&&(nb_args>=3))
                        sConverted=sConverted+(isProto?strProto:"")+QString("Application.m_asservissement.CommandeMouvementXY_TETA(%1,%2,%3);/*%4*/").arg(args.at(0)).arg(args.at(1)).arg(args.at(2)).arg(sComments);
                    if((sId.compare("DistAng")==0)&&(nb_args>=2))
                        sConverted=sConverted+(isProto?strProto:"")+QString("Application.m_asservissement.CommandeMouvementDistanceAngle(%1,%2);/*%3*/").arg(args.at(0)).arg(args.at(1)).arg(sComments);
                    if((sId.compare("Init_asser")==0)&&(nb_args>=3))
                        sConverted=sConverted+(isProto?strProto:"")+QString("Application.m_asservissement.setPosition_XYTeta(%1,%2,%3);/*%4*/").arg(args.at(0)).arg(args.at(1)).arg(args.at(2)).arg(sComments);
                }
                if(sId.compare("Init_rack")==0)
                    sConverted=sConverted+(isProto?strProto:"")+QString("Application.m_asservissement_chariot.Recal_Chariot();/*%1*/").arg(sComments);
                if(sId.compare("Rack")==0)
                    sConverted=sConverted+(isProto?strProto:"")+QString("Application.m_asservissement_chariot.setConsigne(%1);/*%2*/").arg(sValue).arg(sComments);
                break;

            case WAIT:
                if(isProto)
                {
                    sConverted=sConverted+strProto+"dummy transition"+N3T+QString("gotoStateAfter(%1,20);").arg(strNextSate);
                }
                sConverted=sConverted+(isProto?"//":"")+QString("gotoStateAfter(%1,%2);").arg(strNextSate).arg(sValue);
                break;

            case EVENT:
                if(isProto)
                {
                    sConverted=sConverted+strProto+"dummy transition"+N3T+QString("gotoStateAfter(%1,20);").arg(strNextSate);
                }
                if(sId.compare("convAsserv")==0)
                    sConverted=sConverted+(isProto?"//":"")+QString("gotoStateIfConvergence(%1,%2);").arg(strNextSate).arg(sValue);
                if(sId.compare("convRack")==0)
                    sConverted=sConverted+(isProto?"//":"")+QString("gotoStateIfConvergenceRack(%1,%2);").arg(strNextSate).arg(sValue);
                break;

            case SENSOR:
                if(isProto)
                {
                    sConverted=sConverted+strProto+"dummy transition"+N3T+QString("gotoStateAfter(%1,20);").arg(strNextSate);
                }
                if(sActuator.compare("Sensor_sup")==0)
                    sConverted=sConverted+(isProto?"//":"")+QString("gotoStateIfTrue(%1,(%2>%3));").arg(strNextSate).arg(sId).arg(sValue);
                if(sActuator.compare("Sensor_egal")==0)
                    sConverted=sConverted+(isProto?"//":"")+QString("gotoStateIfTrue(%1,(%2==%3));").arg(strNextSate).arg(sId).arg(sValue);
                if(sActuator.compare("Sensor_inf")==0)
                    sConverted=sConverted+(isProto?"//":"")+QString("gotoStateIfTrue(%1,(%2<%3));").arg(strNextSate).arg(sId).arg(sValue);
                break;

            case FREE_EVENT:
                if(isProto)
                {
                    sConverted=sConverted+strProto+"dummy transition"+N3T+QString("gotoStateAfter(%1,20);").arg(strNextSate);
                }
                sConverted=sConverted+(isProto?"//":"")+QString("gotoStateIfTrue(%1,%2);").arg(strNextSate).arg(sValue);
                break;
            case FREE_ACTION:
                strFreeAction=sValue.replace("\n",N3T);
                sConverted=sConverted+N3T+(isProto?(strProto+N3T+"/*"+N3T):"")+QString("%1\n").arg(strFreeAction)+(isProto?(N3T+"*/"):"");
            break;

            case NODE:
                isNode=true;
                sConverted=sConverted+QString("/*Ne rien mettre ici (cf doc Modélia)*/");
            break;

            default: break;
        }

        //Ajout de la l'action ou la transition
        QString strNumState;
        QString strThisState;
        /*-----------------------------------------
        |   TRAITEMENT DES ETATS
        -------------------------------------------*/
        if(bState) //ça fait partie d'un état
        {
            if(!isInState) //on initialise et on ajoute l'action
            {
                //numéro de l'état qu'on incrémente quoiqu'il arrive
                numState++;

                //on récupère le nom de l'état s'il existe, sinon nommage automatique
                QString strThisEnum;
                if(sState.isEmpty())
                {
                    strNumState.setNum(numState);
                    QString strUnamedState=UnamedState.arg(strNumState);
                    strThisState=strUnamedState;
                    strThisEnum=stateEnumFormat.arg(strUnamedState);
                }
                else
                {
                    strThisState=sState;
                    strThisEnum=stateEnumFormat.arg(sState);
                    //on complète l'entete
                    strAddedStates=strAddedStates+N1T+sState+",";
                }

                //on complète l'enum des états et l'entete
                strEnumStates.append(strThisEnum);

                //est-ce qu'il y avait une transition auparavant
                QList<int> previousTransitions=findTransitionsIn(table_sequence,strThisState);
                if(isInTransition) //oui on la ferme avant de passer à l'action suivante
                {
                    if(isNode && previousTransitions.count()<=1)
                        warningOnExit=" /*Un seul lien vers un noeud: Ne rien mettre ici  (cf doc Modélia)*/ ";
                    champStrategie=champStrategie+closePreviousState.arg(warningOnExit);
                    warningOnExit="";
                    isInTransition=false;
                    transitionsList.clear();
                }

                //Si c'est un noeud on modifie les actions de Sortie
                if(isNode)
                {
                    if(isNode && previousTransitions.count()<=1)
                        warningOnExit=" /* Mettre ici le code du onExit de létat "+oldState+" car un seul lien avant le noeud (cf doc Modélia)*/ ";
                    else
                        warningOnExit=" /*Ne rien mettre ici  (cf doc Modélia)*/ ";
                    isNode=false;
                }
                previousTransitions.clear();
                //ajout de l'action
                champStrategie=champStrategie+stateFormat.arg(strThisState)+N3T+sConverted;

                oldState=strThisState;

                isInState=true;
            }
            else //l'état est déjà initialisé, on ajoute l'action
                champStrategie=champStrategie+N3T+sConverted;
        }

        /*-----------------------------------------
        |   TRAITEMENT DES TRANSITIONS
        -------------------------------------------*/
        if(bTransition)
        {
            transitionsList << getType(sActuator);
            if(!isInTransition) //On initialise et on ajoute la transition
            {
                numTransition++;

                if(numState<=0) //on commence la stratégie par une transition: ajout d'un état vide
                {
                    //numéro de l'état
                    numState++;
                    strNumState.setNum(numState);

                    //on complète l'enum des états
                    strThisState=stateEnumFormat.arg(strNumState);
                    strEnumStates.append(strThisState);

                    //ajout de l'action
                    champStrategie=champStrategie+stateFormat.arg(strNumState)+N3T+"//AUCUNE ACTION";

                    isInState=true;
                }

                //est-ce qu'il y avait une(des) action(s) auparavant
                if(isInState) //oui on le ferme avant de passer à la transition
                {
                    champStrategie=champStrategie+N2T+"}\n";
                    isInState=false;
                }

                //ajout de la transition
                champStrategie=champStrategie+N3T+sConverted;

                isInTransition=true;
            }
            else //la transition est initialisée, on l'ajoute
            {
                //Vérifier si il y a uniquement 2 transitions et si cela se termine par un wait
                //qDebug() <<transitionsList.count()<<!isTransition(sNextActuator)<<transitionsList.at(1)<<transitionsList.at(0);
                if((transitionsList.count()==2)&&(!isTransition(sNextActuator))&&(transitionsList.at(1)==WAIT)&&(transitionsList.at(0)==SENSOR))
                {
                    int iInsert=champStrategie.lastIndexOf(");");
                    QString strInsert=","+sValue;
                    champStrategie.insert(iInsert,strInsert);
                }
                else
                    champStrategie=champStrategie+N3T+sConverted;
            }
        }
    }

    //Finalisation du dernier état
    if(isInState)
    {
        champStrategie=champStrategie+N2T+"}\n"+N3T+"gotoStateAfter(FIN_MISSION, 4000);"+closePreviousState.arg(warningOnExit);
        warningOnExit="";
        isInState=false;
    }

    //Si on termine avec une transition, ajout d'un état vide
    if(isInTransition)
    {
        //on ferme l'action précédente avant l'état vide
        champStrategie=champStrategie+closePreviousState.arg(warningOnExit);
        warningOnExit="";
        isInTransition=false;
    }

    //assemblage du cpp
    strWholeFile_cpp= strComments + strInclude + strConstructor + strGetName + strState2Name.arg(strEnumStates)+strStep.arg(champStrategie);

    //bool isAlreadyExist=false;
    if(!fileName_cpp.isEmpty())
    {
        QFile file_cpp(fileName_cpp);
        if(file_cpp.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
        {
            QTextStream stream(&file_cpp);
            stream << strWholeFile_cpp;
            file_cpp.close();
        }
        else{
            //isAlreadyExist=true;
            file_cpp.close();
        }
    }

    //assemblage du .h
    for(int j=2;j<numState+15;j++){ //on ajoute quelques états
        //numéro de l'état
        QString strNumState;
        strNumState.setNum(j);
        strHeader=strHeader+N1T+"STATE_"+strNumState+",";
    }
    strHeader=strHeader+strAddedStates;
    strHeader=strHeader+N1T+"FIN_MISSION"+N1T+"}tState;\n};\n\n#endif // SM_"+nomStrategie.toUpper()+"_H";

    QString fileName_h=fileName_cpp.replace(".cpp", ".h");
    QFile file_h(fileName_h);
    if(file_h.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
    {
        QTextStream stream(&file_h);
        stream << strHeader;
        file_h.close();
    }
    else
        file_h.close();

    // Creation d'un objet XML : sauvegarde_auto
    QString xmlExtension;
    QString temps2=QTime::currentTime().toString("hh_mm");
    QString temps1 = QDate::currentDate().toString("yyyy_MM_dd_at_");
    xmlExtension.append(temps1);
    xmlExtension.append(temps2);
    xmlExtension.append(".xml");
    QString fileName_xml=fileName_h.replace(".h",xmlExtension);
    //qDebug() << fileName_xml;
    Slot_Generate_XML(fileName_xml);


}

void CActuatorSequencer::Slot_SetPosFromSimu()
{
    m_ihm.ui.cB_Asser->setCurrentIndex(1);
    Slot_get_XYTeta();
    m_ihm.ui.sB_TimeOut->setValue(5000);
    m_ihm.ui.cB_Events->setCurrentIndex(0);
    m_ihm.ui.pB_Add_Asser->click();
    m_ihm.ui.pB_Add_Event->click();
}

void CActuatorSequencer::Slot_editFreeItem()
{
    int tabIndex=m_ihm.ui.tW_TabSequences->currentIndex();
    QTableWidget * currentSequence=listSequence.at(tabIndex);
    QObject * pB_Edit=sender();

    QString type="NA";

    QList<QTableWidgetItem*> list_item_selected=currentSequence->selectedItems();
    if(!list_item_selected.isEmpty())
    {
        QTableWidgetItem* item_selected=list_item_selected.first();
        int row_selected=item_selected->row();
        type=getTypeText(currentSequence,row_selected);
        if((pB_Edit==m_ihm.ui.pB_Edit_Free_Action)&&(type.contains("FreeAction")))
        {
            m_ihm.ui.freeAction->clear();
            m_ihm.ui.freeAction->setText(getValueText(currentSequence,row_selected));
        }
        if((pB_Edit==m_ihm.ui.pB_Edit_Free_Transition)&&(type.contains("FreeEvent")))
        {
            m_ihm.ui.freeTransition->clear();
            m_ihm.ui.freeTransition->setText(getValueText(currentSequence,row_selected));
        }
    }
}

void CActuatorSequencer::Slot_loadFreeItem()
{
    int tabIndex=m_ihm.ui.tW_TabSequences->currentIndex();
    QTableWidget * currentSequence=listSequence.at(tabIndex);
    QObject * pB_Load_Free=sender();
    QString type="NA";

    QList<QTableWidgetItem*> list_item_selected=currentSequence->selectedItems();
    if(!list_item_selected.isEmpty())
    {
        QTableWidgetItem* item_selected=list_item_selected.first();
        int row_selected=item_selected->row();
        type=getTypeText(currentSequence,row_selected);
        if((pB_Load_Free==m_ihm.ui.pB_Load_Free_Action)&&(type.contains("FreeAction")))
        {
            QTableWidgetItem* item_selected=currentSequence->item(row_selected,2);
            QString FreeText=m_ihm.ui.freeAction->toPlainText();
            if(!FreeText.isEmpty())
            {
            item_selected->setText(FreeText);
            m_ihm.ui.freeAction->clear();
            }
        }
        if((pB_Load_Free==m_ihm.ui.pB_Load_Free_Transition)&&(type.contains("FreeEvent")))
        {
            QTableWidgetItem* item_selected=currentSequence->item(row_selected,2);
            QString FreeText=m_ihm.ui.freeTransition->toPlainText();
            if(!FreeText.isEmpty())
            {
                item_selected->setText(FreeText);
                m_ihm.ui.freeTransition->clear();
            }
        }
    }
}

void CActuatorSequencer::Slot_setStrategyName_tab(QString strName)
{
    int index=m_ihm.ui.tW_TabSequences->currentIndex();
    m_ihm.ui.tW_TabSequences->setTabText(index,strName);
    for(int i=0;i<strategies2Combine.count();i++)
    {
        if(strategies2Combine.at(i)==index)
        {
            m_ihm.ui.list_Strategies->item(i)->setText(strName);
        }
    }
}

void CActuatorSequencer::Slot_setStrategyName_text(int index)
{
    QString strName=m_ihm.ui.tW_TabSequences->tabText(index);
    m_ihm.ui.strategyName->setText(strName);
}

void CActuatorSequencer::Slot_chooseStrategy(int state)
{
    if(state==Qt::Checked)
    {
        QCheckBox* check = qobject_cast<QCheckBox*>(sender());
        for(int idx=0; idx<listChoice.count();idx++)
        {
            if(check==listChoice.at(idx))
            {
                QTableWidget * currentSequence=listSequence.at(idx);
                if(currentSequence->rowCount()>=2)
                {
                    strategies2Combine.append(idx);
                    m_ihm.ui.list_Strategies->insertItem(strategies2Combine.count(),m_ihm.ui.tW_TabSequences->tabText(idx));
                }
                else
                    check->setCheckState(Qt::Unchecked);
            }
        }
    }
    if(state==Qt::Unchecked)
    {
        QCheckBox* check = qobject_cast<QCheckBox*>(sender());
        for(int idx=0; idx<listChoice.count();idx++)
        {
            if(check==listChoice.at(idx))
            {
                for(int i=0;i<strategies2Combine.count();i++)
                {
                    if(strategies2Combine.at(i)==idx)
                    {
                        strategies2Combine.removeAt(i);
                        delete m_ihm.ui.list_Strategies->takeItem(i);
                        break;
                    }
                }
            }
        }
    }
}

void CActuatorSequencer::Slot_combineStrategies(void)
{
    if(strategies2Combine.count()>=2)
    {
        QTableWidget * newStrategy=this->Slot_Add_Sequence();
        int row=0;
        int idx=0;

        //quelle façon de combiner
        int how=0;
        if(m_ihm.ui.rdB_combine1->isChecked())
            how=0;
        else if(m_ihm.ui.rdB_combine2->isChecked())
            how=1;

        //premiere stratégie de la liste
        QTableWidget * firstStrategy=listSequence.at(strategies2Combine.at(0));

        //on détermine si une ligne est sélectionnée
        int indexAdd=firstStrategy->rowCount();
        if(how==0)
        {
            QList<QTableWidgetItem*> list_item_selected=firstStrategy->selectedItems();
            if(!list_item_selected.isEmpty())
            {
                QTableWidgetItem* item_selected=list_item_selected.first();
                int row_selected=item_selected->row();
                if(row_selected>=0)
                    indexAdd=row_selected;
            }
        }

        //Tout ou partie de la première stratégie
        for(int i=0;i<indexAdd;i++)
        {
            QTableWidgetItem *newItem_type = new QTableWidgetItem(getTypeText(firstStrategy,i));
            QTableWidgetItem *newItem_id = new QTableWidgetItem(getIdText(firstStrategy,i));
            QTableWidgetItem *newItem_value = new QTableWidgetItem(getValueText(firstStrategy,i));
            QTableWidgetItem *newItem_state = new QTableWidgetItem(getSateNameText(firstStrategy,i));
            QTableWidgetItem *newItem_comments = new QTableWidgetItem(getCommentsText(firstStrategy,i));
            bool bSym=getSymChecked(firstStrategy,i);
            bool bProto=getProtoChecked(firstStrategy,i);

            if(isTransition(getTypeText(firstStrategy,i)))
            {
                newItem_type->setBackgroundColor(Qt::cyan);
                newItem_id->setBackgroundColor(Qt::cyan);
                newItem_value->setBackgroundColor(Qt::cyan);
                newItem_state->setBackgroundColor(Qt::cyan);
                newItem_comments->setBackgroundColor(Qt::cyan);
            }

            newStrategy->insertRow(row);
            fillRow(newStrategy,row,newItem_type,newItem_id,newItem_value,newItem_state,newItem_comments,bSym,bProto);
            row++;
        }

        //le reste des strategie
        for(idx=1;idx<strategies2Combine.count();idx++)
        {
            QTableWidget * otherStrategy=listSequence.at(strategies2Combine.at(idx));
            //on remplit la sequence
            for(int i=0;i<otherStrategy->rowCount();i++)
            {
                QTableWidgetItem *newItem_type = new QTableWidgetItem(getTypeText(otherStrategy,i));
                QTableWidgetItem *newItem_id = new QTableWidgetItem(getIdText(otherStrategy,i));
                QTableWidgetItem *newItem_value = new QTableWidgetItem(getValueText(otherStrategy,i));
                QTableWidgetItem *newItem_state = new QTableWidgetItem(getSateNameText(otherStrategy,i));
                QTableWidgetItem *newItem_comments = new QTableWidgetItem(getCommentsText(otherStrategy,i));
                bool bSym=getSymChecked(otherStrategy,i);
                bool bProto=getProtoChecked(otherStrategy,i);

                if(isTransition(getTypeText(otherStrategy,i)))
                {
                    newItem_type->setBackgroundColor(Qt::cyan);
                    newItem_id->setBackgroundColor(Qt::cyan);
                    newItem_value->setBackgroundColor(Qt::cyan);
                    newItem_state->setBackgroundColor(Qt::cyan);
                    newItem_comments->setBackgroundColor(Qt::cyan);
                }

                newStrategy->insertRow(row);
                fillRow(newStrategy,row,newItem_type,newItem_id,newItem_value,newItem_state,newItem_comments,bSym,bProto);
                row++;
            }
        }

        //le reste de la première stratégie s'il le faut
        if(indexAdd<firstStrategy->rowCount())
        {
            for(int i=indexAdd;i<firstStrategy->rowCount();i++)
            {
                QTableWidgetItem *newItem_type = new QTableWidgetItem(getTypeText(firstStrategy,i));
                QTableWidgetItem *newItem_id = new QTableWidgetItem(getIdText(firstStrategy,i));
                QTableWidgetItem *newItem_value = new QTableWidgetItem(getValueText(firstStrategy,i));
                QTableWidgetItem *newItem_state = new QTableWidgetItem(getSateNameText(firstStrategy,i));
                QTableWidgetItem *newItem_comments = new QTableWidgetItem(getCommentsText(firstStrategy,i));
                bool bSym=getSymChecked(firstStrategy,i);
                bool bProto=getProtoChecked(firstStrategy,i);

                if(isTransition(getTypeText(firstStrategy,i)))
                {
                    newItem_type->setBackgroundColor(Qt::cyan);
                    newItem_id->setBackgroundColor(Qt::cyan);
                    newItem_value->setBackgroundColor(Qt::cyan);
                    newItem_state->setBackgroundColor(Qt::cyan);
                    newItem_comments->setBackgroundColor(Qt::cyan);
                }

                newStrategy->insertRow(row);
                fillRow(newStrategy,row,newItem_type,newItem_id,newItem_value,newItem_state,newItem_comments,bSym,bProto);
                row++;
            }
        }
    }
}

void CActuatorSequencer::Slot_moveStrategy(void)
{
    QObject * button=sender();
    bool b_up=false;
    bool b_down=false;
    if(button==m_ihm.ui.pB_Strategy_up)
        b_up=true;
    if(button==m_ihm.ui.pB_Strategy_down)
        b_down=true;

    QList<QListWidgetItem*> list_item_selected=m_ihm.ui.list_Strategies->selectedItems();
    if(!list_item_selected.isEmpty())
    {
        QListWidgetItem* item_selected=list_item_selected.first();
        int row_selected=m_ihm.ui.list_Strategies->row(item_selected);
        if(((row_selected>0)&&(b_up))||((row_selected<m_ihm.ui.list_Strategies->count())&&(b_down)))
        {

            int row_target=-1;
            if(b_up)
                   row_target=row_selected-1;
            if(b_down)
                   row_target=row_selected+1;
            if(row_target>0)
            {
                QListWidgetItem * rowItem;
                QListWidgetItem * rowItem1;

                rowItem = m_ihm.ui.list_Strategies->takeItem(row_selected);
                rowItem1 = m_ihm.ui.list_Strategies->takeItem(row_target);


                m_ihm.ui.list_Strategies->insertItem(row_target, rowItem);
                m_ihm.ui.list_Strategies->insertItem(row_selected, rowItem1);


                m_ihm.ui.list_Strategies->setCurrentRow(row_target);

                strategies2Combine.move(row_selected,row_target);
            }
        }
    }


}

bool CActuatorSequencer::isTransition(QString sType)
{
    return (!((sType.compare("SD20")==0)||(sType.compare("AX-Position")==0)||(sType.compare("AX-Speed")==0)||
           (sType.compare("Motor")==0)||(sType.compare("Power")==0)||(sType.compare("Asser")==0)||(sType.contains("FreeAction"))||(sType.compare("Node")==0) ));

}

void CActuatorSequencer::formatSequence(QTableWidget* table_sequence)
{
    if(table_sequence->rowCount()>0)
        for (int indexItem=0;indexItem<table_sequence->rowCount();indexItem++)
        {
            if(isTransition(getTypeText(table_sequence,indexItem)))
                table_sequence->item(indexItem,0)->setBackgroundColor(Qt::cyan);
            else
                table_sequence->item(indexItem,0)->setBackgroundColor(Qt::white);
        }
}

int CActuatorSequencer::findState(QTableWidget* table_sequence, QString stateName)
{
    int index=-1;
    bool prevWasCondition=false;
    if(!stateName.isEmpty())
    {
        if(table_sequence->rowCount()>0)
        {
            for (int indexItem=0;indexItem<table_sequence->rowCount();indexItem++)
            {
                QString strType=getTypeText(table_sequence,indexItem);
                QString strName=getSateNameText(table_sequence,indexItem);
                if(isTransition(strType))
                    prevWasCondition=true;
                else
                {
                    if(stateName.compare(strName)==0)
                    {
                        if(prevWasCondition)
                        {
                            index=indexItem;
                            break;
                        }
                    }
                    prevWasCondition=false;
                }
            }
        }
    }
    return index;
}
QString CActuatorSequencer::getTypeText(QTableWidget* table_sequence, int idx) {return table_sequence->item(idx,0)->text();}
QString CActuatorSequencer::getIdText(QTableWidget* table_sequence, int idx) {return table_sequence->item(idx,1)->text();}
QString CActuatorSequencer::getValueText(QTableWidget* table_sequence, int idx) {return table_sequence->item(idx,2)->text();}
QString CActuatorSequencer::getSateNameText(QTableWidget* table_sequence, int idx) {return table_sequence->item(idx,3)->text();}
QString CActuatorSequencer::getCommentsText(QTableWidget* table_sequence, int idx) {return table_sequence->item(idx,4)->text();}

void CActuatorSequencer::fillRow(QTableWidget* table_sequence, int idx,QTableWidgetItem * type,QTableWidgetItem * id,QTableWidgetItem * value,QTableWidgetItem * state,QTableWidgetItem * comments,bool symetrie,bool prototype)
{
    table_sequence->setItem(idx, 0, type);
    table_sequence->setItem(idx, 1, id);
    table_sequence->setItem(idx, 2, value);
    table_sequence->setItem(idx, 3, state);
    table_sequence->setItem(idx, 4, comments);
    QCheckBox * newWidget= new QCheckBox();
    if(symetrie)
        newWidget->setCheckState(Qt::Checked);
    table_sequence->setCellWidget(idx,5,newWidget);
    QCheckBox * newWidget2= new QCheckBox();
    if(prototype)
        newWidget2->setCheckState(Qt::Checked);
    table_sequence->setCellWidget(idx,6,newWidget2);
}

void CActuatorSequencer::setPlayMessage(int elapsed,QString type, QString msg)
{
    float fTime;
    QString sTime;
    QString fullMsg;
    fTime=(float)(elapsed/1000.);
    sTime.setNum(fTime,'f',3);

    if(type=="INFO")
    {
        m_ihm.ui.txtPlayInfo->setTextColor(Qt::blue);
        fullMsg="\n"+sTime+"\tINFO:\t\t"+msg+"\n";
        m_ihm.ui.txtPlayInfo->append(fullMsg);
    }

    if(type=="WARNING")
    {
        m_ihm.ui.txtPlayInfo->setTextColor(Qt::darkYellow);
        fullMsg="\n"+sTime+"\tWARNING:\t"+msg+"\n";
        m_ihm.ui.txtPlayInfo->append(fullMsg);
    }

    if(type=="TRANSITION")
    {
        m_ihm.ui.txtPlayInfo->setTextColor(Qt::darkGreen);
        fullMsg=sTime+"\tTRANSITION:\t"+msg+"\n";
        m_ihm.ui.txtPlayInfo->append(fullMsg);
    }

    if(type=="ACTION")
    {
        m_ihm.ui.txtPlayInfo->setTextColor(Qt::black);
        fullMsg=sTime+"\tACTION:\t\t"+msg;
        m_ihm.ui.txtPlayInfo->append(fullMsg);
    }
}

void CActuatorSequencer::creatTabSequence(QWidget * newTab, QTableWidget * newSequence, QString strName, bool isFirstSequence)
{
    int targetIndex=m_ihm.ui.tW_TabSequences->count();
    if(isFirstSequence)
        targetIndex=0;
    newSequence->setColumnCount(7);
    QStringList QS_Labels;
    QS_Labels << "Type" << "Id" << "Value" << "Etape" << "Comments"<<"Symetrie"<<"Only_Prototype";
    newSequence->setHorizontalHeaderLabels(QS_Labels);
    listSequence.append(newSequence);
    QHBoxLayout * hLayout=new QHBoxLayout;
    hLayout->addWidget(newSequence);
    if(isFirstSequence)
        m_ihm.ui.tW_TabSequences->setTabText(0,strName);
    else
        m_ihm.ui.tW_TabSequences->addTab(newTab,strName);
    newTab->setLayout(hLayout);

    QTabBar *tabBar = m_ihm.ui.tW_TabSequences->tabBar();
    QCheckBox *newCheck=new QCheckBox();
    listChoice.append(newCheck);
    connect(newCheck,SIGNAL(stateChanged(int)),this,SLOT(Slot_chooseStrategy(int)));
    tabBar->setTabButton(targetIndex, QTabBar::LeftSide, newCheck);
}

bool CActuatorSequencer::getSymChecked(QTableWidget * sequence,int row)
{
    QCheckBox * widget=qobject_cast<QCheckBox *>(sequence->cellWidget(row,5));
    return widget->isChecked();
}
void CActuatorSequencer::setSymChecked(QTableWidget * sequence,int row, bool state)
{
    QCheckBox * widget=qobject_cast<QCheckBox *>(sequence->cellWidget(row,5));
    if(state)
        widget->setCheckState(Qt::Checked);
    else
        widget->setCheckState(Qt::Unchecked);
}
bool CActuatorSequencer::getProtoChecked(QTableWidget * sequence,int row)
{
    QCheckBox * widget=qobject_cast<QCheckBox *>(sequence->cellWidget(row,6));
    return widget->isChecked();
}
void CActuatorSequencer::setProtoChecked(QTableWidget * sequence,int row, bool state)
{
    QCheckBox * widget=qobject_cast<QCheckBox *>(sequence->cellWidget(row,6));
    if(state)
        widget->setCheckState(Qt::Checked);
    else
        widget->setCheckState(Qt::Unchecked);
}
int CActuatorSequencer::getType(QString sActuator)
{
    int iType=-1;
    if(sActuator.compare("SD20")==0)
        iType=SD20;

    if(sActuator.compare("AX-Position")==0)
        iType=AX_POSITION;

    if(sActuator.compare("AX-Speed")==0)
        iType=AX_SPEED;

    if(sActuator.compare("Motor")==0)
        iType=MOTOR;

    if(sActuator.compare("Power")==0)
        iType=POWER;

    if(sActuator.compare("Asser")==0)
        iType=ASSER;

    if(sActuator.compare("Wait")==0)
        iType=WAIT;

    if(sActuator.compare("Event")==0)
        iType=EVENT;

    if(sActuator.contains("Sensor"))
        iType=SENSOR;

    if(sActuator.contains("FreeEvent"))
        iType=FREE_EVENT;

    if(sActuator.contains("FreeAction"))
        iType=FREE_ACTION;

    if(sActuator.contains("Node"))
        iType=NODE;

    return iType;
}

QList<int> CActuatorSequencer::findTransitionsIn(QTableWidget* table_sequence, QString StateName)
{
    QList<int> transitionsList;
    if(!StateName.isEmpty())
    {
        if(table_sequence->rowCount()>0)
        {
            for (int indexItem=0;indexItem<table_sequence->rowCount();indexItem++)
            {
                QString strType=getTypeText(table_sequence,indexItem);
                QString strName=getSateNameText(table_sequence,indexItem);
                if(isTransition(strType) && strName==StateName)
                    transitionsList << indexItem;
            }
        }
    }
    return transitionsList;
}

QList<int> CActuatorSequencer::findTransitionsOut(QTableWidget* table_sequence, QString StateName)
{
    QList<int> transitionsList;
    if(!StateName.isEmpty())
    {
        if(table_sequence->rowCount()>0)
        {
            for (int indexItem=0;indexItem<table_sequence->rowCount();indexItem++)
            {
                QString strType=getTypeText(table_sequence,indexItem);
                QString strName=getSateNameText(table_sequence,indexItem);
                if(isTransition(strType) && strName==StateName)
                    transitionsList << indexItem;
            }
        }
    }
    return transitionsList;
}
