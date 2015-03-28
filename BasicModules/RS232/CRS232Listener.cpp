/*! \file CRS232Listener.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CRS232Listener.h"



// _____________________________________________________________________
/*!
* Constructeur
*/
CRS232Listener::CRS232Listener()
{

}


// _____________________________________________________________________
/*!
* Destructeur
*/
CRS232Listener::~CRS232Listener()
{
  if (isRunning()) {
    terminate();
    wait(300);  // attends que le thread se termine ou un timeout
  }
}



// _____________________________________________________________________
/*!
* Ouvre le port de communication
*
*/
bool CRS232Listener::openRS232(QSerialPort *serial_port, QString portname, tConfigRS232 config/*=DEFAULT_RS232_CONFIG*/)
{
 bool return_code = true;

 // Ferme le port s'il était ouvert
 closeRS232(serial_port);

 // Port RS232 en mode simulé (port virtuel) ?
 if (portname == C_VIRTUAL_PORT_COM) {
     return_code = true;
 }
 else { // port réel (non simulé)
     serial_port->setPortName(portname);
     QIODevice::OpenMode mode = QIODevice::ReadWrite;
     if (serial_port->open(mode))  { return_code = setConfigRS232(serial_port, config); }
     else                          { return_code = false; }
 } // else le port est un port réel

 if (return_code == true) {
    emit connected();
 }
 else {
    //m_application->m_print_view->print_error(this, "Impossible d'ouvrir le port RS232: " + this->getName() + " sur " + portname);
 }

 return (return_code);
}



// _____________________________________________________________________
/*!
*
*
*/
void CRS232Listener::closeRS232(QSerialPort *serial_port)
{
  if (serial_port->isOpen()) {
      serial_port->close();
      //m_application->m_print_view->print_debug(this, "Port RS232 ferme: " + this->getName());
  }
  emit disconnected();
}


// _____________________________________________________________________
/*!
* Fixe la configuration de la liaison serie
*
*/
bool CRS232Listener::setConfigRS232(QSerialPort *serial_port, tConfigRS232 config)
{
  bool error = true;

  error |= serial_port->setBaudRate(config.baudrate);
  error |= serial_port->setDataBits(config.databits);
  error |= serial_port->setFlowControl(config.flowcontrol);
  error |= serial_port->setParity(config.parity);
  error |= serial_port->setStopBits(config.stopbits);
  //error |= serial_port->setDataErrorPolicy(config.dataerrorpolicy);

  return(error);
}


// _____________________________________________________________________
/*!
*  Point d'entrée pour configurer une liaison série et lancer le thread de surveillance/émission
*
*/
void CRS232Listener::startCommunication(QString portname, tConfigRS232 config/*=DEFAULT_RS232_CONFIG*/)
{
 if (this->isRunning()) { return; } // le thread est déjà en cours -> ne fait rien

 m_serial_portname = portname;
 m_serial_conf = config;

 m_data_to_send.clear(); // efface toute trace dans le buffer d'émission
 if (portname != C_VIRTUAL_PORT_COM) {
    start(); // commence le thread de surveillance des entrées et d'écriture
 }
 else {
    emit connected(); // simule le fait que le port est ouvert en mode virtuel
 }
}

// _____________________________________________________________________
/*!
* Point d'entrée pour terminer une laision série
*
*/
void CRS232Listener::stopCommunication(void)
{
 if (isOpen() == false) {  // cas où le port n'a pas été ouvert
     return;
 }
 else if (this->isRunning() == false) {
     emit disconnected();  // cas du mode virtuel : envoie un évènement pour simuler une fin de connexion
     return;
 } // le thread n'est pas en cours : ne rien faire

 m_stop_thread_request = true;  // demande au thread de s'arrêter
 m_serial_portname = "";
}

// _____________________________________________________________________
/*!
* Point d'entrée pour savoir si le port est ouvert
*
* this->isRunning() indique si le thread tourne
* le thread ne tourne pas en mode simulé
*/
bool CRS232Listener::isOpen(void)
{
  return( (this->isRunning()) || (m_serial_portname==C_VIRTUAL_PORT_COM) );  // suppose que si le thread tourne, c'est que le port de communication est ouvert
}


// _____________________________________________________________________
/*!
*
*
*/
void CRS232Listener::send(const QByteArray &data)
{
  // cas où le thread n'est pas en fonctionnement : les données ne sont pas empilées
  // ce test prends en compte 2 situations :
  //    1. la RS232 n'a pas encore été ouverte
  //    2. la RS232 est un port virutel (mode simulé)
  if (!this->isRunning()) { return; } // en mode simulé, les données ne sont pas empilées

  QMutexLocker mutex_locker(&m_mutex_data_to_send);  // Vérouille l'accès aux données depuis le thread
  m_data_to_send.append(data);
}


// _____________________________________________________________________
/*!
*
*
*/
void CRS232Listener::run()
{
  QSerialPort *serial;
  bool rs_opened;
  QByteArray readData;

  m_stop_thread_request = false;
  serial = new QSerialPort();

  rs_opened = openRS232(serial, m_serial_portname, m_serial_conf);
  if (rs_opened == false) { return; }

  while (!m_stop_thread_request) {
    msleep(50);
    // Traite les données en émission
    m_mutex_data_to_send.lock();
    if (m_data_to_send.size()) {
        serial->write(m_data_to_send);
        m_data_to_send.clear();
    }
    m_mutex_data_to_send.unlock();

    serial->waitForReadyRead(50);
    readData = serial->readAll();
    if (readData.size()) {
        emit readyBytes(readData);
        // echo
        if (m_serial_conf.echo_enabled) { serial->write(readData); }
        //qDebug() << "read = " << size_read;
        //qDebug() << readData;
    }
    serial->waitForBytesWritten(50);
  }

  // Termine proprement la ocnnexion et émet le signal de déconnexion
  closeRS232(serial);
}

