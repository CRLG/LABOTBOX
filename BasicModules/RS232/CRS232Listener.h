/*! \file CRS232Listener.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CBASIC_MODULE_RS232_LISTENER_H_
#define _CBASIC_MODULE_RS232_LISTENER_H_

#include <QThread>
#include <QMutex>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>


// Nom du port pour indiquer un fonctionnement ne mode simulé (port virtuel)
#define C_VIRTUAL_PORT_COM  "VIRTUAL"

//! Structure de donnée permettant la configuration
typedef struct {
    QSerialPort::BaudRate           baudrate;       //! Vitesse de communication
    QSerialPort::DataBits           databits;       //! Nombre de bits de données
    QSerialPort::FlowControl        flowcontrol;    //! Flow controle
    QSerialPort::Parity             parity;         //! Parité
    QSerialPort::StopBits           stopbits;       //! Nombre de bits de stop
    QSerialPort::DataErrorPolicy    dataerrorpolicy;
    bool                            echo_enabled;   //! Echo RX -> TX
}tConfigRS232;

const tConfigRS232 DEFAULT_RS232_CONFIG = { QSerialPort::Baud19200, QSerialPort::Data8, QSerialPort::NoFlowControl, QSerialPort::NoParity, QSerialPort::OneStop, QSerialPort::IgnorePolicy, false };

 /*! @brief class CRS232Listener
  * Thread d'écoute des données en provenance de la ligne physique RX RS232
  */
class CRS232Listener : public QThread
{
    Q_OBJECT

public :
    CRS232Listener();
    ~CRS232Listener();
    void startCommunication(QString portname, tConfigRS232 config=DEFAULT_RS232_CONFIG);
    void stopCommunication(void);
    bool isOpen(void);

    void run(void);

private :
    bool openRS232(QSerialPort *serial_ports, QString portname, tConfigRS232 config=DEFAULT_RS232_CONFIG);
    void closeRS232(QSerialPort *serial_port);
    bool setConfigRS232(QSerialPort *serial_port, tConfigRS232 config);

private :
    tConfigRS232    m_serial_conf;
    QString         m_serial_portname;
    bool            m_stop_thread_request;
    QByteArray      m_data_to_send;
    QMutex          m_mutex_data_to_send;

signals :
    void connected(void);
    void disconnected(void);
    void error(void);
    void readyBytes(QByteArray rcv_data);

public slots :
    void send(const QByteArray &data);
};


#endif // _CBASIC_MODULE_RS232_LISTENER_H_

/*! @} */

