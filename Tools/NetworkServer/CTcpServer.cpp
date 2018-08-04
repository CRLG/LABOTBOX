/*! \file CTcpServer.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>

#include "CTcpServer.h"

/*! \addtogroup NETWORK_SERVER
   *
   *  @{
   */

// _____________________________________________________________________
CTcpServer::CTcpServer(QObject *parent)
    : CNetworkServerInterface(parent),
      m_pTcpServer(NULL),
      m_client_count(0)
{
}

// _____________________________________________________________________
CTcpServer::~CTcpServer()
{
    close();
}

// _____________________________________________________________________
tNtwServerType CTcpServer::getType()
{
    return TCP_SERVER;
}

// =====================================================================
//                          QWebSocket
// =====================================================================
void CTcpServer::onNewConnection()
{
    qDebug() << "CTcpServer : New Connection";
    QTcpSocket *pSocket = m_pTcpServer->nextPendingConnection();
    if (!pSocket) return;

    m_clients.insert(pSocket, m_client_count);
    m_clients_rev.insert(m_client_count, pSocket);

    connect(pSocket, SIGNAL(readyRead()), this, SLOT(processMessage()));
    connect(pSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

    emit connected(m_client_count);
    m_client_count++;
}

// _____________________________________________________________________
void CTcpServer::processMessage()
{
    QTcpSocket *pSender = qobject_cast<QTcpSocket *>(sender());
    if (m_clients.contains(pSender)) {
        int cliend_id = m_clients[pSender];
        emit (receivedData(cliend_id, pSender->readAll()));
    }
}

// _____________________________________________________________________
void CTcpServer::socketDisconnected()
{
    QTcpSocket *pClient = qobject_cast<QTcpSocket *>(sender());
    if (pClient) {
        int id = m_clients[pClient];
        m_clients.remove(pClient);
        m_clients_rev.remove(id);
        pClient->deleteLater();
    }
}

// =====================================================================
//                       WebSocket Server
// =====================================================================
// _____________________________________________________________________
void CTcpServer::open(tNtwConfig cfg)
{
    //qDebug() << "CTcpServer::open";
    if (!m_pTcpServer) {
        m_pTcpServer = new QTcpServer(this);
    }
    unsigned long port = cfg["port"].toUInt();
    QString errorMsg = "Unable to connect TCP Server to port " +  QString::number(port);
    if (m_pTcpServer) {
        if (m_pTcpServer->listen(QHostAddress::Any, port))
        {
            connect(m_pTcpServer, SIGNAL(newConnection()),
                    this,               SLOT(onNewConnection()));
            QString msg = "TCP server listening on port " + QString::number(port);
            emit opened(msg);
        }
        else {
            emit errorMessage(errorMsg);
        }
    }
    else {
        emit errorMessage(errorMsg);
    }
}

// _____________________________________________________________________
void CTcpServer::close()
{
    //qDebug() << "CTcpServer::close";
    for(QMap<QTcpSocket *, int>::iterator it=m_clients.begin(); it!=m_clients.end(); it++) {
        it.key()->deleteLater();
    }
    m_clients.clear();
    m_clients_rev.clear();
    m_client_count = 0;
}

// _____________________________________________________________________
void CTcpServer::write(int client_id, QByteArray data)
{
    //qDebug() << "CTcpServer::write" << client_id << data;
    QTcpSocket *pClient = NULL;
    if (m_clients_rev.contains(client_id)) {
        pClient = m_clients_rev[client_id];
    }
    if (pClient) {
        pClient->write(data);
        emit bytesWritten(client_id, data.size());
    }
}

