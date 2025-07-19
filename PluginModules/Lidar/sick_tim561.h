#ifndef _SICK_TIM561_H_
#define _SICK_TIM561_H_

#include <QTcpSocket>
#include <QTimer>
#include "lidar_data.h"

class SickTIM651 : public QObject
{
    Q_OBJECT
public:
    explicit SickTIM651(QObject *parent = Q_NULLPTR);

    enum {
        PROTOCOL_COLA_BINARY = 0,
        PROTOCOL_COLA_HEXA
    };
    static const unsigned int TIMEOUT_READ_WRITE = 200; // [msec]

    bool open(const QString &hostName, quint16 port, int protocol = PROTOCOL_COLA_BINARY, bool autoconnect=false);
    virtual void disconnection();

    bool login();
    bool start_measurement();
    bool stop_measurement();
    bool get_firmware_version(QByteArray &ba);
    bool run();
    bool poll_one_telegram(CLidarData *scan_data);

    bool isErrorReturned(const QByteArray &ba);

private :
    static const unsigned int AUTORECONNECT_PERIOD = 1000; // [msec]
    QString m_hostname;
    int m_port;
    bool m_enable_autoreconnect;
    QTimer m_timer_autoreconnect;

    bool m_cola_protocol_binary;

    QTcpSocket m_socket;

    bool decodeTelegram(const QByteArray telegram, CLidarData *scan_data);

public slots :
    /*virtual*/ void close();

private slots :
    void on_connect();
    void on_disconnect();
    void on_tick_timer_autoreconnect();

signals :
    void newData(CLidarData data);
};

#endif // _SICK_TIM561_H_
