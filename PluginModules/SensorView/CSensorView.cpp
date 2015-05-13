/*! \file CSensorView.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CSensorView.h"
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
CSensorView::CSensorView(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_SensorView, AUTEUR_SensorView, INFO_SensorView)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CSensorView::~CSensorView()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CSensorView::init(CLaBotBox *application)
{
  CModule::init(application);
  setGUI(&m_ihm); // indique à la classe de base l'IHM

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

  //récupération des signaux déjà placés sur la vue
  val=m_application->m_eeprom->read(getName(),"signalsPlacement",QStringList());
  QStringList listePointsWidget_=val.toStringList();
  val=m_application->m_eeprom->read(getName(),"signalsAdded",QStringList());
  listeStringSignalsAdded=val.toStringList();
  int minSize = 0;
  minSize=qMin(listePointsWidget_.size(), listeStringSignalsAdded.size());
  int ind;
  if(minSize>0)
  {
    QString unStringPoint, temp, xString, yString;
    QStringList ListXY;
    double xFromString, yFromString;
    for(ind=0;ind<minSize;ind++)
    {
        unStringPoint=listePointsWidget_.at(ind);
        ListXY=unStringPoint.split('x');
        if (ListXY.size()>=2){
            temp=ListXY.at(0);
            xString=temp.remove(QChar('('), Qt::CaseInsensitive);
            xFromString=xString.toInt();
            temp=ListXY.at(1);
            yString=temp.remove(QChar(')'), Qt::CaseInsensitive);
            yFromString=yString.toInt();
            listePointsSignalsAdded << QPoint(xFromString,yFromString);
        }
     }
  }
  if (minSize>0)
      for(ind=0;ind<minSize;ind++)
      {
          //listeAddedSignals << new viewWidget(listeStringSignalsAdded.at(ind),listePointsSignalsAdded.at(ind));
          addWidget(listeStringSignalsAdded.at(ind),listePointsSignalsAdded.at(ind));
      }

  m_ihm.ui.listWidget->setAcceptDrops(false);
  m_ihm.ui.listWidget->setDragEnabled(false);
  m_ihm.ui.viewWidget->setAcceptDrops(false);
  m_ihm.ui.viewWidget->setDragEnabled(false);

  isLocked=m_ihm.ui.pb_lockGUI->isChecked();

  connect(m_ihm.ui.pb_lockGUI,SIGNAL(toggled(bool)),this,SLOT(lock(bool)));

  connect(m_ihm.ui.viewWidget,SIGNAL(addWidget(QString,QPoint)),this,SLOT(addWidget(QString,QPoint)));
  connect(m_ihm.ui.listWidget,SIGNAL(refreshList(QString)),this,SLOT(removeWidget(QString)));
  refreshListeVariables();

  connect(&m_timer_lecture_variables, SIGNAL(timeout()), this, SLOT(refreshValeursVariables()));
  //m_timer_lecture_variables.start(PERIODE_ECHANTILLONNAGE_VARIABLES);
  connect(m_ihm.ui.pb_play,SIGNAL(toggled(bool)),this,SLOT(start(bool)));

  connect(m_ihm.ui.pb_filtering_nothing, SIGNAL(clicked()),this,SLOT(uncheckAllSignals()));
  connect(m_ihm.ui.pb_filtering_actuator, SIGNAL(clicked()),this,SLOT(refreshListeVariables()));
  connect(m_ihm.ui.pb_filtering_ana, SIGNAL(clicked()),this,SLOT(refreshListeVariables()));
  connect(m_ihm.ui.pb_filtering_computed, SIGNAL(clicked()),this,SLOT(refreshListeVariables()));
  connect(m_ihm.ui.pb_filtering_tor, SIGNAL(clicked()),this,SLOT(refreshListeVariables()));
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CSensorView::close(void)
{
    // Mémorise en EEPROM l'état de la fenêtre
    m_application->m_eeprom->write(getName(), "geometry", QVariant(m_ihm.geometry()));
    m_application->m_eeprom->write(getName(), "visible", QVariant(m_ihm.isVisible()));
    m_application->m_eeprom->write(getName(), "niveau_trace", QVariant((unsigned int)getNiveauTrace()));

    QStringList ListXY;
    QStringList ListName;
    QString unStringPoint, xString, yString;
    for(int ind=0;ind<listeAddedSignals.size();ind++)
    {
        unStringPoint="("+xString.setNum(listeAddedSignals.at(ind)->addedSignalPosition.x())+"x";
        unStringPoint=unStringPoint+yString.setNum(listeAddedSignals.at(ind)->addedSignalPosition.y())+")";
        ListXY << unStringPoint;
        ListName << listeAddedSignals.at(ind)->addedSignalName;
    }
    m_application->m_eeprom->write(getName(), "signalsPlacement", QVariant(ListXY));
    m_application->m_eeprom->write(getName(), "signalsAdded", QVariant(ListName));
}


// _____________________________________________________________________
/*!
*  Met à jour la liste des variables dans la fenêtre de gauche
*
*/
void CSensorView::refreshListeVariables(void)
{
    QStringList var_list;
    m_application->m_data_center->getListeVariablesName(var_list);
    m_ihm.ui.listWidget->clear();
    bool bFilterTor=m_ihm.ui.pb_filtering_tor->isChecked();
    bool bFilterAna=m_ihm.ui.pb_filtering_ana->isChecked();
    bool bFilterComputed=m_ihm.ui.pb_filtering_computed->isChecked();
    bool bFilterActuator=m_ihm.ui.pb_filtering_actuator->isChecked();
    bool bFilterNothing= !(bFilterTor || bFilterAna || bFilterComputed || bFilterActuator);

    for (int i=0; i<var_list.count(); i++)
    {
    if (!isAdded(var_list.at(i)))
    {
        QVariant property_value=m_application->m_data_center->getDataProperty(var_list.at(i),"Type");
        QString mime_property=property_value.toString();

        if((mime_property.isEmpty()) && bFilterNothing)
            m_ihm.ui.listWidget->addElement(QPixmap(":/icons/mime_default.png"), var_list.at(i));
        else if((mime_property.compare("sensor_tor")==0) && (bFilterTor || bFilterNothing))
            m_ihm.ui.listWidget->addElement(QPixmap(":/icons/mime_sensor_tor.png"), var_list.at(i));
        else if((mime_property.compare("computed_signal")==0) && (bFilterComputed || bFilterNothing))
            m_ihm.ui.listWidget->addElement(QPixmap(":/icons/mime_computed_signal.png"), var_list.at(i));
        else if (bFilterNothing)
            m_ihm.ui.listWidget->addElement(QPixmap(":/icons/mime_default.png"), var_list.at(i));
    }
    }
}

// _____________________________________________________________________
/*!
*  Met à jour la valeur de chaque variable contenu dans la liste des variables à surveiller
* Traces les courbes à chaque pas de temps
*
*/
//void CSensorView::refreshValeursVariables(QVariant var_value)
void CSensorView::refreshValeursVariables()
{
    //qDebug()<< "*";

    for(int ind=0;ind<listeAddedSignals.size();ind++)
    {
        QString var_name = listeAddedSignals.at(ind)->addedSignalName;
        QVariant var_value = m_application->m_data_center->read(var_name);

        switch(listeAddedSignals.at(ind)->typeSignal){
        case mime_default:
        case mime_computed_signal:
            listeAddedSignals.at(ind)->addedLCDNumber->display(var_value.toDouble());
            break;
        case mime_sensor_tor:
            listeAddedSignals.at(ind)->addedLed->setValue(((var_value.toDouble()>0) ? true : false));
            break;
        default:
            break;
        }
    }
}


void CSensorView::start(bool value)
{
    if (value)
    {
        m_timer_lecture_variables.start(PERIODE_ECHANTILLONNAGE_VARIABLES);
        m_ihm.ui.pb_lockGUI->setChecked(true);
    }
    else
        m_timer_lecture_variables.stop();
}

void CSensorView::lock(bool value)
{
    isLocked=value;
    //on fige les interfaces
    m_ihm.ui.listWidget->setAcceptDrops(!value);
    m_ihm.ui.listWidget->setDragEnabled(!value);
    m_ihm.ui.viewWidget->setAcceptDrops(!value);
    m_ihm.ui.viewWidget->setDragEnabled(!value);

    if (value)
        m_ihm.ui.pb_lockGUI->setIcon(QIcon(":/icons/locked.png"));
    else
    {
        m_ihm.ui.pb_lockGUI->setIcon(QIcon(":/icons/unlocked.png"));
        refreshListeVariables();
    }
}

void CSensorView::uncheckAllSignals()
{
    m_ihm.ui.pb_filtering_actuator->setChecked(false);
    m_ihm.ui.pb_filtering_ana->setChecked(false);
    m_ihm.ui.pb_filtering_computed->setChecked(false);
    m_ihm.ui.pb_filtering_tor->setChecked(false);
    refreshListeVariables();
}

void CSensorView::addWidget(QString var_name, QPoint var_pos)
{

    bool isExist=false;
    int indExistant=0;
    for(int ind=0;ind<listeAddedSignals.size();ind++)
        if(listeAddedSignals.at(ind)->addedSignalName.compare(var_name)==0)
        {
            isExist=true;
            indExistant=ind;
        }


    QVariant property_value=m_application->m_data_center->getDataProperty(var_name,"Type");
    QString mime_property=property_value.toString();
    int type;

    if(mime_property.isEmpty())
        type=mime_default;
    else if(mime_property.compare("sensor_tor")==0)
        type=mime_sensor_tor;
    else if(mime_property.compare("computed_signal")==0)
        type=mime_computed_signal;
    else
        type=mime_default;

    QLed * newLed;
    QLCDNumber * newLCDNumber;

    switch(type)
    {
        case mime_default:
        case mime_computed_signal:
            newLCDNumber =new QLCDNumber(m_ihm.ui.viewWidget);
            newLCDNumber->setObjectName(var_name);
            newLCDNumber->move(var_pos);
            newLCDNumber->setVisible(true);
            if (isExist)
            {
                listeAddedSignals.at(indExistant)->addedLCDNumber=newLCDNumber;
                listeAddedSignals.at(indExistant)->addedSignalPosition.setX(var_pos.x());
                listeAddedSignals.at(indExistant)->addedSignalPosition.setY(var_pos.y());
            }
            else
                listeAddedSignals << new viewWidget(var_name,var_pos,type,newLCDNumber);
            qDebug() << "LCDNumber" << var_name << "ajouté";
            break;
        case mime_sensor_tor:
            newLed = new QLed(m_ihm.ui.viewWidget);
            newLed->setObjectName(var_name);
            newLed->setMinimumSize(20,20);
            newLed->setMaximumSize(20,20);
            newLed->setValue(true);
            newLed->move(var_pos);
            newLed->setVisible(true);
            if (isExist)
            {
                listeAddedSignals.at(indExistant)->addedLed=newLed;
                listeAddedSignals.at(indExistant)->addedSignalPosition.setX(var_pos.x());
                listeAddedSignals.at(indExistant)->addedSignalPosition.setY(var_pos.y());
            }
            else
                listeAddedSignals << new viewWidget(var_name,var_pos,type,newLed);
            qDebug() << "Led" << var_name << "ajouté";
            break;
        default:
            break;
    }

}

void CSensorView::removeWidget(QString var_name)
{
    int iToRemove=0;
    bool bToRemove=false;
//    qDebug()<<"il y a"<<listeAddedSignals.size()<<"elements:";
    for(int ind=0;ind<listeAddedSignals.size();ind++)
    {
        qDebug()<<listeAddedSignals.at(ind)->addedSignalName;
        if (listeAddedSignals.at(ind)->addedSignalName.compare(var_name)==0)
        {
            iToRemove=ind;
            bToRemove=true;
        }
    }
    if (bToRemove)
    {
//        qDebug()<<"on retire"<<listeAddedSignals.at(iToRemove)->addedSignalName;
        listeAddedSignals.removeAt(iToRemove);
    }
//    qDebug()<<"il reste"<<listeAddedSignals.size()<<"elements :";
//    for(int i=0;i<listeAddedSignals.size();i++)
//    {
//        qDebug()<<listeAddedSignals.at(i)->addedSignalName;
//    }
    refreshListeVariables();
}

bool CSensorView::isAdded(QString var_name)
{
    bool return_value=false;
    for(int ind=0;ind<listeAddedSignals.size();ind++)
    {
        if (listeAddedSignals.at(ind)->addedSignalName.compare(var_name)==0)
            return_value=true;
    }
    return return_value;
}


