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

  m_ihm.ui.tW_TabSequences->setCurrentIndex(0);
  QTableWidget * newSequence= new QTableWidget;
  newSequence->setColumnCount(4);
  QStringList QS_Labels;
  QS_Labels << "type" << "id" << "value" << "comments 1";
  newSequence->setHorizontalHeaderLabels(QS_Labels);
  listSequence.append(newSequence);
  QHBoxLayout * hLayout=new QHBoxLayout;
  hLayout->addWidget(newSequence);
  m_ihm.ui.tW_TabSequences->currentWidget()->setLayout(hLayout);
  connect(m_ihm.ui.tW_TabSequences,SIGNAL(tabCloseRequested(int)),this,SLOT(Slot_Remove_Sequence(int)));

  updateComboBox();
  updateTooltip();

  connect(m_ihm.ui.pB_Add_AX,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_SD20,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_Motor,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_Power,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_Wait,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_Asser,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_Event,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));
  connect(m_ihm.ui.pB_Add_Sensor,SIGNAL(clicked(bool)),this,SLOT(addSequenceItem()));

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
  connect(m_ihm.ui.pB_Generate, SIGNAL(clicked(bool)),this,SLOT(Slot_Generate()));

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

  connect(m_ihm.ui.tool_deg,SIGNAL(returnPressed()),this,SLOT(Slot_convert_to_rad()));
  connect(m_ihm.ui.tool_rad,SIGNAL(returnPressed()),this,SLOT(Slot_convert_to_deg()));

  connect(m_ihm.ui.cB_bRepeatSequence, SIGNAL(toggled(bool)),this,SLOT(setEnableRepetition(bool)));
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
 //m_ihm.ui.cB_Asser->addItem("Distance",QVariant(3));
 //m_ihm.ui.cB_Asser->addItem("Angle",QVariant(4));
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
    int i;
//Motor
    i=m_ihm.ui.cB_Motor->currentIndex()+1;
    str_name=QString("cde_moteur_%1").arg(i);
    str_tooltip = m_application->m_data_center->getDataProperty(str_name, "Tooltip").toString();
    if (str_tooltip != "")
    {
        m_ihm.ui.tip_Motor->setText(str_tooltip);
        m_ihm.ui.tip_Motor->setCursorPosition(0);
    }

    //PowerSwitch
    i=m_ihm.ui.cB_Power->currentIndex()+1;
         str_name=QString("PowerSwitch_xt%1").arg(i);
         str_tooltip = m_application->m_data_center->getDataProperty(str_name, "Tooltip").toString();
         if (str_tooltip != "")
         {
             m_ihm.ui.tip_Power->setText(str_tooltip);
             m_ihm.ui.tip_Power->setCursorPosition(0);
         }

 //SD20
 i=m_ihm.ui.cB_SD20->currentIndex()+13;
     str_name=QString("cde_servo_%1").arg(i);
     str_tooltip = m_application->m_data_center->getDataProperty(str_name, "Tooltip").toString();
     if (str_tooltip != "")
     {
         m_ihm.ui.tip_SD20->setText(str_tooltip);
         m_ihm.ui.tip_SD20->setCursorPosition(0);
     }

 //AX
 i=m_ihm.ui.cB_AX->currentIndex();
     str_name=QString("cde_ax_%1").arg(i);
     str_tooltip = m_application->m_data_center->getDataProperty(str_name, "Tooltip").toString();
     if (str_tooltip != "")
     {
         m_ihm.ui.tip_AX->setText(str_tooltip);
         m_ihm.ui.tip_AX->setCursorPosition(0);
     }

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

//m_ihm.ui.centralwidget->repaint();

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
    }
    else if(pB_Add==m_ihm.ui.pB_Add_Wait)
    {
        type="Wait";
        id="0";
        value.setNum(m_ihm.ui.sB_Wait->value());
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

    QTableWidgetItem *newItem_type = new QTableWidgetItem(type);
    QTableWidgetItem *newItem_id = new QTableWidgetItem(id);
    QTableWidgetItem *newItem_value = new QTableWidgetItem(value);
    QTableWidgetItem *newItem_comments = new QTableWidgetItem(comments);


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
    currentSequence->setItem(indexAdd, 0, newItem_type);
    currentSequence->setItem(indexAdd, 1, newItem_id);
    currentSequence->setItem(indexAdd, 2, newItem_value);
    currentSequence->setItem(indexAdd, 3, newItem_comments);
    currentSequence->selectRow(indexAdd);
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

void CActuatorSequencer::Slot_Play(bool oneStep, int idStart)
{
    int idxMin=0;
    int idxMax=0;

    bStop=false;
    bPlay=true;
    bResume=false;
    update_sequenceButtons();

    int indexItem=0;

    int tabIndex=m_ihm.ui.tW_TabSequences->currentIndex();
    QTableWidget * table_sequence=listSequence.at(tabIndex);
    int numberOfIndex=m_ihm.ui.tW_TabSequences->count();
    for(int i=0;i<numberOfIndex;i++)
    {
        if(i!=tabIndex)
            m_ihm.ui.tW_TabSequences->widget(i)->setEnabled(false);
    }

    QString sActuator;
    QString id;
    QString value;

    int nbSequence=1;

    if ((m_ihm.ui.cB_bRepeatSequence->isChecked())&&(!oneStep))
        nbSequence=m_ihm.ui.sB_numberOfSequence->value();

    for(int i=0;i<nbSequence;i++)
    {

    if(table_sequence->rowCount()>0)
    {
        if (oneStep)
        {
            idxMax=qMax(0,idStart);
            idxMin=qMin(table_sequence->rowCount(),idStart);
        }
        else{
            idxMax=table_sequence->rowCount()-1;
            idxMin=0;
        }
        //QString modeAndRange=oneStep?"Step":"whole sequence";
        //qDebug() << modeAndRange << "min" << idxMin << "max" << idxMax;


        QString showNubSequence;
        showNubSequence.setNum(i+1);
        m_ihm.ui.label_showNbSequence->setText(showNubSequence);

        for (indexItem=idxMin;indexItem<idxMax+1;indexItem++)
        {
            sActuator=table_sequence->item(indexItem,0)->text();
            //qDebug() << sActuator;

            //color index to tag the sequence
            table_sequence->item(indexItem,0)->setBackgroundColor(Qt::red);

            if(sActuator.compare("SD20")==0)
            {
                id=table_sequence->item(indexItem,1)->text();
                value=table_sequence->item(indexItem,2)->text();
                //qDebug() << "SD20 number" << id << "at" <<value;
                m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", true);
                m_application->m_data_center->write("NumeroServoMoteur1", id);
                m_application->m_data_center->write("PositionServoMoteur1", value);
                m_application->m_data_center->write("VitesseServoMoteur1", 0);
                m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", false);
            }
            if(sActuator.compare("AX-Position")==0)
            {
                id=table_sequence->item(indexItem,1)->text();
                value=table_sequence->item(indexItem,2)->text();
                //qDebug() << "AX number" << id << "position at" <<value;
                m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", true);
                m_application->m_data_center->write("num_servo_ax", id);
                m_application->m_data_center->write("commande_ax", cSERVO_AX_POSITION);
                m_application->m_data_center->write("valeur_commande_ax", value);
                m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", false);
                QTest::qWait(100); //Pour eviter la non prise en compte I2C
            }
            if(sActuator.compare("AX-Speed")==0)
            {
                id=table_sequence->item(indexItem,1)->text();
                value=table_sequence->item(indexItem,2)->text();
                //qDebug() << "AX number" << id << "speed at" <<value;
                m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", true);
                m_application->m_data_center->write("num_servo_ax", id);
                m_application->m_data_center->write("commande_ax", cSERVO_AX_VITESSE);
                m_application->m_data_center->write("valeur_commande_ax", value);
                m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", false);
                QTest::qWait(100); //Pour eviter la non prise en compte I2C
            }
            if(sActuator.compare("Motor")==0)
            {            
                //id_int=table_sequence->item(indexItem,1)->text().toInt();
                id=table_sequence->item(indexItem,1)->text();
                value=table_sequence->item(indexItem,2)->text();

                QString str_name=QString("cde_moteur_%1").arg(id);
                //qDebug() << "Motor" << str_name << "at" <<value;
                m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 1);
                m_application->m_data_center->write(str_name, value);
                m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 0);
            }
            if(sActuator.compare("Power")==0)
            {
                //id_int=table_sequence->item(indexItem,1)->text().toInt();
                id=table_sequence->item(indexItem,1)->text();
                value=table_sequence->item(indexItem,2)->text();

                QString str_name=QString("PowerSwitch_xt%1").arg(id);
                bool bvalue=false;
                if(value.compare("1")==0)
                    bvalue=true;
                //bvalue=(value.compare("1")==0?true:false);
                //qDebug() << "Power" << str_name << "at" <<bvalue;
                m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 1);
                m_application->m_data_center->write(str_name, bvalue);
                m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 0);
            }
            if(sActuator.compare("Wait")==0)
            {
                value=table_sequence->item(indexItem,2)->text();
                //qDebug() << "Wait during" << value << "ms";
                QTest::qWait(value.toInt()); //no waiting if invalid cast
            }
            if(sActuator.compare("Event")==0)
            {
                //evenement
                bool bConv=false;

                id=table_sequence->item(indexItem,1)->text();
                value=table_sequence->item(indexItem,2)->text();

                //init du timeout et du compteur associe
                short cmptTe=0;
                int timeOut=value.toInt();

                if(id.compare("convAsserv")==0)
                {
                    bConv=false;

                    //qDebug() << "Attente convergence asservissement ou " << timeOut << "ms";
                    while(!bConv && !bResume && !bStop && (cmptTe<(timeOut/40)))
                    {
                        QTest::qWait(40);
                        if((m_application->m_data_center->read("Convergence").toInt()==cCONVERGENCE_OK)&& (cmptTe>10))
                            bConv=true;
                        cmptTe++;
                    }
                }
                if(id.compare("convRack")==0)
                {
                    bConv=false;

                    //qDebug() << "Attente convergence RACK ou " << timeOut << "ms";
                    while((!bConv && !bResume && !bStop) && (cmptTe<(timeOut/40)))
                    {
                        QTest::qWait(40);
                        if((m_application->m_data_center->read("rack_convergence").toBool()) && (cmptTe>10))
                            bConv=true;
                        cmptTe++;
                        //qDebug() << m_application->m_data_center->read("rack_convergence").toBool() << "-->" << bConv << "ou" << (cmptTe<(timeOut/40)) << "--" << cmptTe << "<" <<(timeOut/40) ;
                    }
                }
            }
            if(sActuator.contains("Sensor"))
            {
                //evenement
                bool bReachCondition=false;

                id=table_sequence->item(indexItem,1)->text();
                value=table_sequence->item(indexItem,2)->text();

//                //init du timeout et du compteur associe
//                short cmptTe=0;
//                int timeOut=value.toInt();

                if(sActuator.compare("Sensor_sup")==0)
                {
                    bReachCondition=false;

                    //qDebug() << "Wait " << id << ">" << value;
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

                    //qDebug() << "Wait " << id << "=" << value;
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

                    qDebug() << "Wait " << id << "<" << value;
                    while(!bReachCondition && !bResume && !bStop)// && (cmptTe<(timeOut/40)))
                    {
                        QTest::qWait(40);
                        if(m_application->m_data_center->read(id).toFloat()< value.toFloat()) //&& (cmptTe>10))
                            bReachCondition=true;
                        //cmptTe++;
                    }
                }
            }
            if(sActuator.compare("Asser")==0)
            {
                id=table_sequence->item(indexItem,1)->text();
                value=table_sequence->item(indexItem,2)->text();
                QStringList args=value.split(",",QString::SkipEmptyParts);
                int nb_args=args.count();
                //"XY","DistAng","Rack"
                if(id.compare("XY")==0)
                {
                    if(nb_args==2)
                    {
                        qDebug() << "MVT XYTheta" << args;
                        m_application->m_data_center->write("COMMANDE_MVT_XY_TxSync", 1);
                        m_application->m_data_center->write("X_consigne", args.at(0).toFloat());
                        m_application->m_data_center->write("Y_consigne", args.at(1).toFloat());
                        m_application->m_data_center->write("COMMANDE_MVT_XY_TxSync", 0);
                    }
                }
                if(id.compare("XYTheta")==0)
                    if(nb_args==3)
                    {
                        qDebug() << "MVT XYTheta" << args;
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
                        qDebug() << "MVT XY" << args;
                        m_application->m_data_center->write("COMMANDE_DISTANCE_ANGLE_TxSync", 1);
                        m_application->m_data_center->write("distance_consigne",args.at(0).toFloat());
                        m_application->m_data_center->write("angle_consigne", args.at(1).toFloat());
                        m_application->m_data_center->write("COMMANDE_DISTANCE_ANGLE_TxSync", 0);
                    }
                }
                qDebug() << id;
                if(id.compare("Rack")==0)
                {
                    if(nb_args==1)
                    {
                        qDebug() << "Asser Rack" << args;
                        float consigne=args.at(0).toInt();
                        int sens_position=0;
                        if(consigne>=0)
                            sens_position=5;
                        else
                            sens_position=15;
                        float consigne_messagerie=qRound(qAbs(consigne)/10.0);
                        m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", true);
                        m_application->m_data_center->write("NumeroServoMoteur1", 50);
                        m_application->m_data_center->write("PositionServoMoteur1", consigne_messagerie);
                        m_application->m_data_center->write("VitesseServoMoteur1", sens_position);
                        m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", false);
                    }
                }
            }

            while(bResume)
                QTest::qWait(40);

            if (bStop)
            {
                m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 1);
                m_application->m_data_center->write("cde_moteur_1", 0);
                m_application->m_data_center->write("cde_moteur_2", 0);
                m_application->m_data_center->write("cde_moteur_3", 0);
                m_application->m_data_center->write("cde_moteur_4", 0);
                m_application->m_data_center->write("cde_moteur_5", 0);
                m_application->m_data_center->write("cde_moteur_6", 0);
                m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 0);
                m_application->m_data_center->write("COMMANDE_MVT_MANUEL_TxSync", 1);
                m_application->m_data_center->write("PuissanceMotD", 0);
                m_application->m_data_center->write("PuissanceMotG", 0);
                m_application->m_data_center->write("COMMANDE_MVT_MANUEL_TxSync", 0);
                m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", 1);
                m_application->m_data_center->write("NumeroServoMoteur1", 50);
                m_application->m_data_center->write("PositionServoMoteur1", 10);
                m_application->m_data_center->write("VitesseServoMoteur1", 0);
                m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", 0);
                //bStop=true;
                bPlay=false;
                bResume=false;

                if(table_sequence->rowCount()>0)
                    for (indexItem=0;indexItem<table_sequence->rowCount();indexItem++)
                        table_sequence->item(indexItem,0)->setBackgroundColor(Qt::white);

                m_ihm.ui.label_showNbSequence->clear();

                qDebug() << "end of sequence";
                for(int i=0;i<numberOfIndex;i++)
                {
                        m_ihm.ui.tW_TabSequences->widget(i)->setEnabled(true);
                }

                update_sequenceButtons();
                return;
            }

        }
    }

    if(table_sequence->rowCount()>0)
        for (indexItem=0;indexItem<table_sequence->rowCount();indexItem++)
            table_sequence->item(indexItem,0)->setBackgroundColor(Qt::white);
    }

    m_ihm.ui.label_showNbSequence->clear();

    qDebug() << "end of sequence";
    for(int i=0;i<numberOfIndex;i++)
    {
            m_ihm.ui.tW_TabSequences->widget(i)->setEnabled(true);
    }

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

    update_sequenceButtons();

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


void CActuatorSequencer::Slot_Save()
{
    int indexItem=0;
    int tabIndex=m_ihm.ui.tW_TabSequences->currentIndex();
    QTableWidget * table_sequence=listSequence.at(tabIndex);

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

    QDomDocument doc("sequence_xml");
    // root node
    QDomElement strategieNode = doc.createElement("sequence");
    doc.appendChild(strategieNode);

    //commentaire global
//    QDomElement commentsNode = doc.createElement("commentaires");
//    QTextEdit *textEdit_comment=m_ihm.findChild<QTextEdit*>("textEdit_comment");
//    commentsNode.appendChild(doc.createTextNode(textEdit_comment->toPlainText()));
//    strategieNode.appendChild(commentsNode);

    for (indexItem=0;indexItem<table_sequence->rowCount();indexItem++)
    {
        QDomElement ligneNode = doc.createElement("ligne");
        ligneNode.setAttribute("type",table_sequence->item(indexItem,0)->text());
        ligneNode.setAttribute("id",table_sequence->item(indexItem,1)->text());
        ligneNode.setAttribute("value",table_sequence->item(indexItem,2)->text());
        ligneNode.setAttribute("comments",table_sequence->item(indexItem,3)->text());
        strategieNode.appendChild(ligneNode);

        QString sActuator=table_sequence->item(indexItem,0)->text();
        QString sId=table_sequence->item(indexItem,1)->text();
        QString sValue=table_sequence->item(indexItem,2)->text();
        QString sComments=table_sequence->item(indexItem,3)->text();
        QString sConverted;

        if(sActuator.compare("SD20")==0)
        {
            sConverted=sConverted+"//"+sComments+ "\n";
            sConverted=sConverted+QString("sd20.setPos(%1,%2);\n\n").arg(sId).arg(sValue);
            qDebug() << sConverted;
        }
        if(sActuator.compare("AX-Position")==0)
        {
            sConverted=sConverted+"//"+sComments+ "\n";
            sConverted=sConverted+QString("ax.setPos(%1,%2);\n\n").arg(sId).arg(sValue);
            qDebug() << sConverted;
        }
        /*
        if(sActuator.compare("AX-Speed")==0)
        {
            id=table_sequence->item(indexItem,1)->text();
            value=table_sequence->item(indexItem,2)->text();
            qDebug() << "AX number" << id << "speed at" <<value;
            m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", true);
            m_application->m_data_center->write("num_servo_ax", id);
            m_application->m_data_center->write("commande_ax", cSERVO_AX_VITESSE);
            m_application->m_data_center->write("valeur_commande_ax", value);
            m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", false);
            QTest::qWait(100); //Pour eviter la non prise en compte I2C
        }
        if(sActuator.compare("Motor")==0)
        {
            //id_int=table_sequence->item(indexItem,1)->text().toInt();
            id=table_sequence->item(indexItem,1)->text();
            value=table_sequence->item(indexItem,2)->text();

            QString str_name=QString("cde_moteur_%1").arg(id);
            qDebug() << "Motor" << str_name << "at" <<value;
            m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 1);
            m_application->m_data_center->write(str_name, value);
            m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 0);
        }
        if(sActuator.compare("Wait")==0)
        {
            value=table_sequence->item(indexItem,2)->text();
            qDebug() << "Wait during" << value << "ms";
            QTest::qWait(value.toInt()); //no waiting if invalid cast
        }
        if(sActuator.compare("Event")==0)
        {
            //evenement
            bool bConv=false;

            id=table_sequence->item(indexItem,1)->text();
            value=table_sequence->item(indexItem,2)->text();

            //init du timeout et du compteur associe
            short cmptTe=0;
            int timeOut=value.toInt();

            if(id.compare("convAsserv")==0)
            {
                bConv=false;

                qDebug() << "Attente convergence asservissement ou " << timeOut << "ms";
                while(!bConv && !bResume && !bStop && (cmptTe<(timeOut/40)))
                {
                    QTest::qWait(40);
                    if((m_application->m_data_center->read("Convergence").toInt()==cCONVERGENCE_OK)&& (cmptTe>10))
                        bConv=true;
                    cmptTe++;
                }
            }
            if(id.compare("convRack")==0)
            {
                bConv=false;

                qDebug() << "Attente convergence RACK ou " << timeOut << "ms";
                while((!bConv && !bResume && !bStop) && (cmptTe<(timeOut/40)))
                {
                    QTest::qWait(40);
                    if((m_application->m_data_center->read("rack_convergence").toBool()) && (cmptTe>10))
                        bConv=true;
                    cmptTe++;
                    //qDebug() << m_application->m_data_center->read("rack_convergence").toBool() << "-->" << bConv << "ou" << (cmptTe<(timeOut/40)) << "--" << cmptTe << "<" <<(timeOut/40) ;
                }
            }
        }
        if(sActuator.contains("Sensor"))
        {
            //evenement
            bool bReachCondition=false;

            id=table_sequence->item(indexItem,1)->text();
            value=table_sequence->item(indexItem,2)->text();

//                //init du timeout et du compteur associe
//                short cmptTe=0;
//                int timeOut=value.toInt();

            if(sActuator.compare("Sensor_sup")==0)
            {
                bReachCondition=false;

                qDebug() << "Wait " << id << ">" << value;
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

                qDebug() << "Wait " << id << "=" << value;
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

                qDebug() << "Wait " << id << "<" << value;
                while(!bReachCondition && !bResume && !bStop)// && (cmptTe<(timeOut/40)))
                {
                    QTest::qWait(40);
                    if(m_application->m_data_center->read(id).toFloat()< value.toFloat()) //&& (cmptTe>10))
                        bReachCondition=true;
                    //cmptTe++;
                }
            }
        }
        if(sActuator.compare("Asser")==0)
        {
            id=table_sequence->item(indexItem,1)->text();
            value=table_sequence->item(indexItem,2)->text();
            QStringList args=value.split(",",QString::SkipEmptyParts);
            int nb_args=args.count();
            //"XY","DistAng","Rack"
            if(id.compare("XY")==0)
            {
                if(nb_args==2)
                {
                    qDebug() << "MVT XYTheta" << args;
                    m_application->m_data_center->write("COMMANDE_MVT_XY_TxSync", 1);
                    m_application->m_data_center->write("X_consigne", args.at(0).toFloat());
                    m_application->m_data_center->write("Y_consigne", args.at(1).toFloat());
                    m_application->m_data_center->write("COMMANDE_MVT_XY_TxSync", 0);
                }
            }
            if(id.compare("XYTheta")==0)
                if(nb_args==3)
                {
                    qDebug() << "MVT XYTheta" << args;
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
                    qDebug() << "MVT XY" << args;
                    m_application->m_data_center->write("COMMANDE_DISTANCE_ANGLE_TxSync", 1);
                    m_application->m_data_center->write("distance_consigne",args.at(0).toFloat());
                    m_application->m_data_center->write("angle_consigne", args.at(1).toFloat());
                    m_application->m_data_center->write("COMMANDE_DISTANCE_ANGLE_TxSync", 0);
                }
            }
            qDebug() << id;
            if(id.compare("Rack")==0)
            {
                if(nb_args==1)
                {
                    qDebug() << "Asser Rack" << args;
                    float consigne=args.at(0).toInt();
                    int sens_position=0;
                    if(consigne>=0)
                        sens_position=5;
                    else
                        sens_position=15;
                    float consigne_messagerie=qRound(qAbs(consigne)/10.0);
                    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", true);
                    m_application->m_data_center->write("NumeroServoMoteur1", 50);
                    m_application->m_data_center->write("PositionServoMoteur1", consigne_messagerie);
                    m_application->m_data_center->write("VitesseServoMoteur1", sens_position);
                    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", false);
                }
            }
        }*/

    }


    if(!fileName.isEmpty())
    {
        QFile fichier(fileName);
        if(fichier.open(QIODevice::ReadWrite | QIODevice::Text))
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

            qDebug() << "remove" << nb_lignes << "rows";

            QDomDocument doc("sequence_xml");
            if (doc.setContent(&fichier)) {
                QDomElement docElem = doc.documentElement();
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

                        if(e.tagName().compare("ligne")==0)
                        {
                            QTableWidgetItem *newItem_type = new QTableWidgetItem(e.attribute("type","Wait"));
                            QTableWidgetItem *newItem_id = new QTableWidgetItem(e.attribute("id",""));
                            QTableWidgetItem *newItem_value = new QTableWidgetItem(e.attribute("value","1000"));
                            QTableWidgetItem *newItem_comments = new QTableWidgetItem(e.attribute("comments",""));
                            table_sequence->insertRow(indexItem);
                            table_sequence->setItem(indexItem, 0, newItem_type);
                            table_sequence->setItem(indexItem, 1, newItem_id);
                            table_sequence->setItem(indexItem, 2, newItem_value);
                            table_sequence->setItem(indexItem, 3, newItem_comments);
                            qDebug() << "insert" << e.attribute("type","Wait") << "at row number" << indexItem;
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
    int prevIndex=0;
    prevIndex=m_ihm.ui.tW_TabSequences->currentIndex();
    QTableWidget * prev_table_sequence=listSequence.at(prevIndex);


    QTableWidget * newSequence= new QTableWidget;
    newSequence->setColumnCount(4);
    QStringList QS_Labels;
    QS_Labels << "type" << "id" << "value" << "comments";
    newSequence->setHorizontalHeaderLabels(QS_Labels);
    listSequence.append(newSequence);
    QHBoxLayout * hLayout=new QHBoxLayout;
    hLayout->addWidget(newSequence);
    QWidget * newTab=new QWidget();
    QString tabLabel=QString("Sequence%1").arg(listSequence.count());
    m_ihm.ui.tW_TabSequences->addTab(newTab,tabLabel);
    newTab->setLayout(hLayout);

    if(prev_table_sequence->rowCount()<=0)
        this->Slot_Remove_Sequence(prevIndex);

    m_ihm.ui.tW_TabSequences->setCurrentWidget(newTab);

    return newSequence;
}

void CActuatorSequencer::Slot_Remove_Sequence(int index)
{
    if(m_ihm.ui.tW_TabSequences->count()>1)
    {
    listSequence.removeAt(index);
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

            int row_target=row_selected-1; //up the list, dow the index ;-)

            QList<QTableWidgetItem*> rowItems,rowItems1;
            for (int col = 0; col < colCount; ++col)
            {
                rowItems << currentSequence->takeItem(row_selected, col);
                rowItems1 << currentSequence->takeItem(row_target, col);
            }

            for (int cola = 0; cola < colCount; ++cola)
            {
                currentSequence->setItem(row_target, cola, rowItems.at(cola));
                currentSequence->setItem(row_selected, cola, rowItems1.at(cola));
            }

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

            for (int cola = 0; cola < colCount; ++cola)
            {
                currentSequence->setItem(row_target, cola, rowItems.at(cola));
                currentSequence->setItem(row_selected, cola, rowItems1.at(cola));
            }

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

void CActuatorSequencer::Slot_Generate()
{
    int indexItem=0;
    int tabIndex=m_ihm.ui.tW_TabSequences->currentIndex();
    QTableWidget * table_sequence=listSequence.at(tabIndex);

    // Creation d'un fichier
    QString ficName("genratedSequence_");
    QString temps2=QTime::currentTime().toString("hh_mm");
    QString temps1 = QDate::currentDate().toString("yyyy_MM_dd_at_");
    ficName.append(temps1);
    ficName.append(temps2);
    ficName.append(".cpp");

    QString caption("Generate Strategie in CPP file");
    QString filter("CPP Files (*.cpp)");
    QString fileName = QFileDialog::getSaveFileName(&m_ihm,caption, ficName,filter);

    QString coreFile="#include \"strategie.h\"\n#include \"mbed.h\"\n#include \"RessourcesHardware.h\"\n\n#include \"CGlobale.h\"\n#include \"ConfigSpecifiqueCoupe.h\"\n#include <math.h>\n\n";
    coreFile=coreFile+"bool Strategie::%1()\n{\n\tbool isFinished=false;\n\tenum ID{\n%2,\n\t\tFIN\n\t};\n\n\tswitch(current_state)\n\t{%3\n\t\tdefault: isFinished=true; break;\n\t}\n\treturn isFinished;\n}";

    QString champEnum;
    QString champStrategie;

    bool isInState=false;
    int numState=0;
    bool isInTransition=false;
    int numTransition=0;

    for (indexItem=0;indexItem<table_sequence->rowCount();indexItem++)
    {

        QString sActuator=table_sequence->item(indexItem,0)->text();
        QString sId=table_sequence->item(indexItem,1)->text();
        QString sValue=table_sequence->item(indexItem,2)->text();
        QString sComments=table_sequence->item(indexItem,3)->text();
        QString sConverted;
        bool isState=false;
        bool isTransition=false;


        if(sActuator.compare("SD20")==0)
        {
            isState=true;
            sConverted=sConverted+QString("Application.m_servos_sd20.CommandePosition(%1,%2);/*%3*/").arg(sId).arg(sValue).arg(sComments);
        }
        if(sActuator.compare("AX-Position")==0)
        {
            isState=true;
            sConverted=sConverted+QString("Application.m_servos_ax.CommandePosition(%1,%2);/*%3*/").arg(sId).arg(sValue).arg(sComments);
        }
        if(sActuator.compare("AX-Speed")==0)
        {
            isState=true;
            sConverted=sConverted+QString("Application.m_servos_ax.setSpeed(%1,%2);/*%3*/").arg(sId).arg(sValue).arg(sComments);
        }
        if(sActuator.compare("Motor")==0)
        {
            isState=true;
            sConverted=sConverted+QString("Application.m_moteurs.CommandeVitesse(%1,%2);/*%3*/").arg(sId).arg(sValue).arg(sComments);
        }
        if(sActuator.compare("Wait")==0)
        {
            isTransition=true;
            int msTime=sValue.toInt();
            float convertedTime=msTime/1000.0;
            QString str;
            str.setNum(convertedTime,QLocale::C,3);
            sConverted=sConverted+QString("timeOut(%1)").arg(str);
        }

        if(sActuator.compare("Event")==0)
        {
            isTransition=true;
            int msTime=sValue.toInt();
            float convertedTime=msTime/1000.0;
            QString str;
            str.setNum(convertedTime,QLocale::C,3);
            if(sId.compare("convAsserv")==0)
                sConverted=sConverted+QString("convMvt(%1)").arg(str);
            /*if(id.compare("convRack")==0)
                sConverted=sConverted+QString("convMvt(%1)").arg(sValue);*/
        }
        if(sActuator.contains("Sensor"))
        {
            isTransition=true;
            if(sActuator.compare("Sensor_sup")==0)
                sConverted=sConverted+QString("(%1>%2)").arg(sId).arg(sValue);
            if(sActuator.compare("Sensor_egal")==0)
                sConverted=sConverted+QString("(%1==%2)").arg(sId).arg(sValue);
            if(sActuator.compare("Sensor_inf")==0)
                sConverted=sConverted+QString("(%1<%2)").arg(sId).arg(sValue);
        }
        if(sActuator.compare("Asser")==0)
        {
            isState=true;
            QStringList args=sValue.split(",",QString::SkipEmptyParts);
            int nb_args=args.count();
            if((sId.compare("XY")==0)&&(nb_args>=2))
                sConverted=sConverted+QString("Application.m_asservissement.CommandeMouvementXY(%1,%2,%3);/*%4*/").arg(args.at(0)).arg(args.at(1)).arg(sComments);
            if((sId.compare("XYTheta")==0)&&(nb_args>=3))
                sConverted=sConverted+QString("Application.m_asservissement.CommandeMouvementXY_TETA(%1,%2,%3);/*%4*/").arg(args.at(0)).arg(args.at(1)).arg(args.at(2)).arg(sComments);
            if((sId.compare("DistAng")==0)&&(nb_args>=2))
                sConverted=sConverted+QString("Application.m_asservissement.CommandeMouvementDistanceAngle(%1,%2,%3);/*%4*/").arg(args.at(0)).arg(args.at(1)).arg(sComments);
            //if(id.compare("Rack")==0)
        }
        if(isState) //ça fait partie d'un état
        {
            if(isInState) //il est initialisé, on ajoute l'action
                champStrategie=champStrategie+"\n\t\t\t"+sConverted;
            else //sinon on initialise et on ajoute l'action
            {
                QString str;
                numState++;
                str.setNum(numState);
                //est-ce qu'il y avait une transition auparavant
                if(isInTransition) //oui on la ferme avant de passer à l'action suivante
                {
                    champStrategie=champStrategie+") pass(ETAPE_"+str+"); break;\n\n";
                    isInTransition=false;
                }
                champStrategie=champStrategie+"\n\t\tcase ETAPE_"+str+":\n\t\t\t"+sConverted;
                if((numState<=0)||(numTransition<=0))
                    champEnum=champEnum+"\t\tETAPE_"+str;
                else
                    champEnum=champEnum+",\n\t\tETAPE_"+str;
                isInState=true;
            }
        }
        if(isTransition)
        {
            if(isInTransition) //elle est initialisée, on ajoute la transtition
                champStrategie=champStrategie+"&&"+sConverted;
            else //sinon on initialise et on ajoute l'action
            {
                QString str;
                numTransition++;
                str.setNum(numTransition);
                //est-ce qu'il y avait un état auparavant
                if(isInState) //oui on le ferme avant de passer à la transition
                {
                    champStrategie=champStrategie+"\n\t\t\tpass(TRANSITION_"+str+"); break;\n";
                    isInState=false;
                }
                champStrategie=champStrategie+"\t\tcase TRANSITION_"+str+": if("+sConverted;
                if((numState<=0)||(numTransition<=0))
                    champEnum=champEnum+"\t\tTRANSITION_"+str;
                else
                    champEnum=champEnum+",\n\t\tTRANSITION_"+str;
                isInTransition=true;
            }
        }
    }
    if(isInState)
    {
        champStrategie=champStrategie+"\npass(FIN); break;\n\n\t\tcase FIN: isFinished=true;break;\n";
        isInState=false;
    }
    if(isInTransition)
    {
        champStrategie=champStrategie+") pass(FIN); break;\n\n\t\tcase FIN: isFinished=true;break;\n";
        isInTransition=false;
    }

    QString nomStrategie;
    if(m_ihm.ui.strategyName->text().isEmpty())
            nomStrategie="maStrategie";
    else
            nomStrategie=m_ihm.ui.strategyName->text();

    coreFile=coreFile.arg(nomStrategie).arg(champEnum).arg(champStrategie);

    if(!fileName.isEmpty())
    {
        QFile fichier(fileName);
        if(fichier.open(QIODevice::ReadWrite | QIODevice::Text))
        {
            QTextStream stream(&fichier);
            stream << coreFile;
            fichier.close();
        }
        else
            fichier.close();
    }
}
