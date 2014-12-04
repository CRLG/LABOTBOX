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
          listeAddedSignals << new viewWidget(listeStringSignalsAdded.at(ind),listePointsSignalsAdded.at(ind));
          addWidget(listeStringSignalsAdded.at(ind),listePointsSignalsAdded.at(ind));
      }

  connect(m_ihm.ui.viewWidget,SIGNAL(addWidget(QString,QPoint)),this,SLOT(addWidget(QString,QPoint)));
  refreshListeVariables();
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

  for (int i=0; i<var_list.count(); i++)
  {
    QVariant property_value=m_application->m_data_center->getDataProperty(var_list.at(i),"Type");
    QString mime_property=property_value.toString();

    if(mime_property.isEmpty())
        m_ihm.ui.listWidget->addElement(QPixmap(":/icons/mime_default.png"), var_list.at(i));
    else if(mime_property.compare("sensor_tor"))
        m_ihm.ui.listWidget->addElement(QPixmap(":/icons/mime_sensor_tor.png"), var_list.at(i));
    else if(mime_property.compare("computed_signal"))
        m_ihm.ui.listWidget->addElement(QPixmap(":/icons/mime_computed_signal.png"), var_list.at(i));
    else
        m_ihm.ui.listWidget->addElement(QPixmap(":/icons/mime_default.png"), var_list.at(i));
  }
}

void CSensorView::addWidget(QString var_name, QPoint var_pos)
{
    QVariant property_value=m_application->m_data_center->getDataProperty(var_name,"Type");
    QString mime_property=property_value.toString();

    if(mime_property.isEmpty())
    {
        QLCDNumber *newSensor=new QLCDNumber(m_ihm.ui.viewWidget);
        newSensor->setObjectName(var_name);
        newSensor->move(var_pos);
        newSensor->setVisible(true);
    }
    else if(mime_property.compare("sensor_tor"))
    {    
        QLed *newSensor=new QLed(m_ihm.ui.viewWidget);
        newSensor->setObjectName(var_name);
        newSensor->setMinimumSize(20,20);
        newSensor->setMaximumSize(20,20);
        newSensor->setValue(true);
        newSensor->move(var_pos);
        newSensor->setVisible(true);
    }
    else if(mime_property.compare("computed_signal"))
    {
        QLCDNumber *newSensor=new QLCDNumber(m_ihm.ui.viewWidget);
        newSensor->setObjectName(var_name);
        newSensor->move(var_pos);
        newSensor->setVisible(true);
    }
    else
    {
        QLCDNumber *newSensor=new QLCDNumber(m_ihm.ui.viewWidget);
        newSensor->setObjectName(var_name);
        newSensor->move(var_pos);
        newSensor->setVisible(true);
    }
}




