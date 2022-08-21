/*! \file CTelemetryServer.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CTelemetryServer.h"
#include "CApplication.h"
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
CTelemetryServer::CTelemetryServer(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_TelemetryServer, AUTEUR_TelemetryServer, INFO_TelemetryServer)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CTelemetryServer::~CTelemetryServer()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CTelemetryServer::init(CApplication *application)
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

  /*
  connect(&m_exchanger, SIGNAL(receive_doc_designer(DocDesigner)), this, SLOT(onReceiveDocDesigner(DocDesigner)));
  connect(&m_exchanger, SIGNAL(receive_double(double)), this, SLOT(onReceiveDouble(double)));
  connect(&m_exchanger, SIGNAL(receive_int32(int)), this, SLOT(onReceiveInt32(int)));
  connect(&m_exchanger, SIGNAL(receive_variant(QVariant)), this, SLOT(onReceiveVariant(QVariant)));
  connect(&m_exchanger, SIGNAL(receive_raw_buffer(QByteArray)), this, SLOT(onReceiveRawData(QByteArray)));
*/

  m_exchanger.start(2468);
/*
  tNtwConfig cfg;
  cfg.insert("port", "2468");
  m_tcp_server.open(cfg);

  connect(&m_tcp_server, SIGNAL(receivedData(QByteArray)), &m_exchanger, SLOT(receive(QByteArray)));
  connect(&m_exchanger, SIGNAL(send(QByteArray)), &m_tcp_server, SLOT(write(QByteArray)));
*/
  connect(m_ihm.ui.pb_test1, SIGNAL(clicked(bool)), this, SLOT(onPbTest1()));
  connect(m_ihm.ui.pb_test2, SIGNAL(clicked(bool)), this, SLOT(onPbTest2()));

  connect(&m_exchanger.m_exchanger, SIGNAL(receive_doc_designer(QString)), this, SLOT(onReceiveDocDesigner(QString)));
  connect(&m_exchanger.m_exchanger, SIGNAL(receive_double(double)), this, SLOT(onReceiveDouble(double)));
  connect(&m_exchanger.m_exchanger, SIGNAL(receive_int32(qint32)), this, SLOT(onReceiveInt32(int)));
  connect(&m_exchanger.m_exchanger, SIGNAL(receive_uint8(quint8)), this, SLOT(onReceiveUInt8(quint8)));
  connect(&m_exchanger.m_exchanger, SIGNAL(receive_variant(QVariant)), this, SLOT(onReceiveVariant(QVariant)));
  connect(&m_exchanger.m_exchanger, SIGNAL(receive_raw_buffer(QByteArray)), this, SLOT(onReceiveRawData(QByteArray)));
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CTelemetryServer::close(void)
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
void CTelemetryServer::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}

void CTelemetryServer::onReceiveDocDesigner(QString doc)
{
    qDebug() << "CTelemetryServer::onReceiveDocDesigner" << doc;
}

void CTelemetryServer::onReceiveDouble(double val)
{
    qDebug() << "CTelemetryServer::onReceiveDouble" << val;
}

void CTelemetryServer::onReceiveUInt8(quint8 val)
{
    qDebug() << "CTelemetryServer::onReceiveUInt8" << val;
}

void CTelemetryServer::onReceiveInt32(int val)
{
    qDebug() << "CTelemetryServer::onReceiveInt32" << val;
}

void CTelemetryServer::onReceiveVariant(const QVariant &val)
{
    qDebug() << "CTelemetryServer::onReceiveVariant" << val;
}

void CTelemetryServer::onReceiveRawData(const QByteArray &raw_data)
{
    //qDebug() << "CTelemetryServer::onReceiveRawData" << raw_data;
    qDebug() << "CTelemetryServer::onReceiveRawData / Size : " << raw_data.size();
}

void CTelemetryServer::onPbTest1()
{
   double ullval = 123.456;
   m_exchanger.m_exchanger.send_variant(ullval);
}

void CTelemetryServer::onPbTest2()
{
    DocDesigner doc;

    for (int i=0; i<80; i++)
    {
        doc.appendCRLF();
        doc.appendChapterTitle(1, "Nom du chapitre 5");
        doc.appendText("Une liste à puces");
        QStringList lst;
        lst << "Bulle1" << "Bulle2" << "Bulle3";
        doc.appendBulletList(lst);
        doc.appendText("Du texte normal juste en dessous de la liste à puces", true);
        doc.appendText("La même liste à puce mais en plus gros et en couleur !", true);
        doc.setFont(DocFont(doc.getDefaultFamilyFont(), 24, true, true, true, Qt::darkBlue));
        doc.appendBulletList(lst);
    }
    m_exchanger.m_exchanger.send_doc_designer(doc);
}

