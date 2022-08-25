#include "exchangerclient.h"


ExchangerClient::ExchangerClient(QTcpSocket *parent)
    : QTcpSocket(parent),
      m_enable_autoreconnect(false)
{
    connect(&m_timer_autoreconnect, SIGNAL(timeout()), this, SLOT(on_tick_timer_autoreconnect()));

    connect(this, SIGNAL(connected()), this, SLOT(on_connect()));
    connect(this, SIGNAL(disconnected()), this, SLOT(on_disconnect()));
}

// _________________________________________________________
void ExchangerClient::connectToHost(const QString &hostName, quint16 port, bool autoconnect)
{
    m_enable_autoreconnect = autoconnect;
    m_hostname = hostName;
    m_port = port;
    QTcpSocket::connectToHost(m_hostname, m_port);
    if (m_enable_autoreconnect) m_timer_autoreconnect.start(AUTORECONNECT_PERIOD);
}

// _________________________________________________________
void ExchangerClient::disconnectFromHost()
{
    QTcpSocket::disconnectFromHost();
    m_enable_autoreconnect = false;
}

// _________________________________________________________
void ExchangerClient::on_connect()
{
    m_timer_autoreconnect.stop();
}

// _________________________________________________________
void ExchangerClient::on_disconnect()
{
    if (m_enable_autoreconnect) m_timer_autoreconnect.start(AUTORECONNECT_PERIOD);
}

// _________________________________________________________
void ExchangerClient::on_tick_timer_autoreconnect()
{
    QTcpSocket::connectToHost(m_hostname, m_port);
}
