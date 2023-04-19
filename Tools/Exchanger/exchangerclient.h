#ifndef _EXCHANGER_CLIENT_H_
#define _EXCHANGER_CLIENT_H_

#include <QTcpSocket>
#include <QTimer>

class ExchangerClient : public QTcpSocket
{
    Q_OBJECT
public:
    explicit ExchangerClient(QObject *parent = nullptr);

    virtual void connectToHost(const QString &hostName, quint16 port, bool autoconnect=false);
    virtual void disconnection();

private :
    static const unsigned int AUTORECONNECT_PERIOD = 1000; // [msec]
    QString m_hostname;
    int m_port;
    bool m_enable_autoreconnect;
    QTimer m_timer_autoreconnect;

private slots :
    void on_connect();
    void on_disconnect();
    void on_tick_timer_autoreconnect();
};

#endif // _EXCHANGER_CLIENT_H_
