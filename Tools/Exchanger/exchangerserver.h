#ifndef _EXCHANGERSERVER_H_
#define _EXCHANGERSERVER_H_

#include <QtNetwork>
#include <exchanger.h>

class CExchangerServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit CExchangerServer(QObject *parent = nullptr);

    bool start(int port);

    void setExchanger(CExchanger *exchanger);

private slots:
    void new_connection();
    void client_disconnected();
private:
    QList<QTcpSocket *> m_clients;

protected :
    CExchanger *m_p_exchanger;

signals :
    void connectionToSocket(QTcpSocket *sender);
    void started();
    void connected();
    void disconnected();
};

#endif // _EXCHANGERSERVER_H_
