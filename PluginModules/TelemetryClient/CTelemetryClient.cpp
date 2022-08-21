/*! \file CTelemetryClient.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "math.h"
#include "CTelemetryClient.h"
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
CTelemetryClient::CTelemetryClient(const char *plugin_name)
    :CPluginModule(plugin_name, VERSION_TelemetryClient, AUTEUR_TelemetryClient, INFO_TelemetryClient)
{
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CTelemetryClient::~CTelemetryClient()
{

}


// _____________________________________________________________________
/*!
*  Initialisation du module
*
*/
void CTelemetryClient::init(CApplication *application)
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

  init_socket("169.254.186.70", 2468);
  //init_socket("127.0.0.1", 2468);

  //connect(&m_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
  //connect(&m_exchanger, SIGNAL(send(QByteArray)), this, SLOT(onWrite(QByteArray)));

  /*
  connect(&m_exchanger, SIGNAL(receive_doc_designer(DocDesigner)), this, SLOT(onReceiveDocDesigner(DocDesigner)));
  connect(&m_exchanger, SIGNAL(receive_int32(int)), this, SLOT(onReceiveInt32(int)));
  connect(&m_exchanger, SIGNAL(receive_double(double)), this, SLOT(onReceiveDouble(double)));
  connect(&m_exchanger, SIGNAL(receive_variant(QVariant)), this, SLOT(onReceiveVariant(QVariant)));
  connect(&m_exchanger, SIGNAL(receive_raw_buffer(QByteArray)), this, SLOT(onReceiveRawData(QByteArray)));
*/

  connect(&m_telemetry_client, SIGNAL(connected()), this, SLOT(connected()));
  connect(&m_telemetry_client, SIGNAL(disconnected()), this, SLOT(disconnected()));

  connect(&m_telemetry_client.m_exchanger, SIGNAL(receive_doc_designer(QString)), this, SLOT(onReceiveDocDesigner(QString)));
  connect(&m_telemetry_client.m_exchanger, SIGNAL(receive_double(double)), this, SLOT(onReceiveDouble(double)));
  connect(&m_telemetry_client.m_exchanger, SIGNAL(receive_int32(qint32)), this, SLOT(onReceiveInt32(int)));
  connect(&m_telemetry_client.m_exchanger, SIGNAL(receive_variant(QVariant)), this, SLOT(onReceiveVariant(QVariant)));
  connect(&m_telemetry_client.m_exchanger, SIGNAL(receive_raw_buffer(QByteArray)), this, SLOT(onReceiveRawData(QByteArray)));

  connect(m_ihm.ui.pb_test1, SIGNAL(clicked(bool)), this, SLOT(onPbTest1()));
  connect(m_ihm.ui.pb_test2, SIGNAL(clicked(bool)), this, SLOT(onPbTest2()));

  connect(&m_timer, SIGNAL(timeout()), this, SLOT(onPbTest1()));
  m_timer.start(14);
}


// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CTelemetryClient::close(void)
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
void CTelemetryClient::onRightClicGUI(QPoint pos)
{
  QMenu *menu = new QMenu();

  menu->addAction("Select background color", this, SLOT(selectBackgroundColor()));
  menu->exec(m_ihm.mapToGlobal(pos));
}


void CTelemetryClient::init_socket(QString ip_add, int port)
{
    m_telemetry_client.connectToHost(ip_add, port);
    //connect(&m_telemetry_client, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

void CTelemetryClient::onPbTest1()
{
    QByteArray ba;
    //for (int i=0; i<1478; i++) {
    for (long i=0; i<pow(2, 10); i++) {
        ba.append((char)i+5);
    }
    //ba.append(0x13);
    m_telemetry_client.m_exchanger.send_raw_buffer(ba);
    m_telemetry_client.m_exchanger.send_raw_buffer(ba);
    m_telemetry_client.m_exchanger.send_uint8(123);
    m_telemetry_client.m_exchanger.send_raw_buffer(ba);
}

void CTelemetryClient::onPbTest2()
{
/*
    DocDesigner doc;
    doc.appendCRLF();
    doc.appendChapterTitle(1, "Nom du chapitre 3");

    doc.appendText("Une liste à puces");
    QStringList lst;
    lst << "Bonjour" << "Coucou" << "Hello" << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
    doc.appendBulletList(lst);
    doc.appendText("Du texte normal juste en dessous de la liste à puces", true);
    doc.appendText("La même liste à puce mais en plus gros et en couleur !", true);
    doc.setFont(DocFont(doc.getDefaultFamilyFont(), 24, true, true, true, Qt::darkBlue));
    doc.appendBulletList(lst);

    m_telemetry_client.send_doc_designer(doc);
*/
    quint8 ui8val = 9;
    m_telemetry_client.m_exchanger.send_uint8(ui8val);
}

void CTelemetryClient::connected()
{
   qDebug() << "CTelemetryClient:: Connected to host";
}

void CTelemetryClient::disconnected()
{
    qDebug() << "CTelemetryClient:: Disconnected from host";
}


void CTelemetryClient::onReceiveDocDesigner(QString doc)
{
    qDebug() << "CTelemetryClient::onReceiveDocDesigner" << doc;
}

void CTelemetryClient::onReceiveRawData(const QByteArray &raw_data)
{
    qDebug() << "CTelemetryClient::onReceiveRawData" << raw_data;
    qDebug() << "Size : " << raw_data.size();
}

void CTelemetryClient::onReceiveDouble(double val)
{
   qDebug() << "CTelemetryClient::onReceiveDouble" << val;
}

void CTelemetryClient::onReceiveInt32(int val)
{
    qDebug() << "CTelemetryClient::onReceiveInt32" << val;
}

void CTelemetryClient::onReceiveVariant(const QVariant &val)
{
    qDebug() << "CTelemetryClient::onReceiveVariant" << val;
}

