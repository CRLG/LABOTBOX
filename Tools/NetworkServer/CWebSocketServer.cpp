/*! \file CWebSocketServer.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>

#include "CWebSocketServer.h"

/*! \addtogroup NETWORK_SERVER
   * 
   *  @{
   */
	   
// _____________________________________________________________________
CWebSocketServer::CWebSocketServer(QObject *parent)
    : CNetworkServerInterface(parent),
      m_pWebSocketServer(NULL),
      m_client_count(0)
{
    setTransmissionMode(TEXT_MODE);
}

// _____________________________________________________________________
CWebSocketServer::~CWebSocketServer()
{
    close();
}

// _____________________________________________________________________
tNtwServerType CWebSocketServer::getType()
{
    return WEBSOCKET_SERVER;
}

// _____________________________________________________________________
void CWebSocketServer::setTransmissionMode(tTransmissionMode mode)
{
    m_transmission_mode = mode;
}

// =====================================================================
//                          QWebSocket
// =====================================================================
void CWebSocketServer::onNewConnection()
{
    qDebug() << "CWebSocketServer : New Connection";
    QWebSocket *pSocket = m_pWebSocketServer->nextPendingConnection();
    if (!pSocket) return;

    m_clients.insert(pSocket, m_client_count);
    m_clients_rev.insert(m_client_count, pSocket);

    connect(pSocket, SIGNAL(textMessageReceived(QString)), this, SLOT(processMessage(QString)));
    connect(pSocket, SIGNAL(binaryMessageReceived(QByteArray)), this, SLOT(processMessage(QByteArray)));
    connect(pSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));

    emit connected(m_client_count);
    m_client_count++;
}

// _____________________________________________________________________
void CWebSocketServer::processMessage(QString message)
{
    QWebSocket *pSender = qobject_cast<QWebSocket *>(sender());
    if (m_clients.contains(pSender)) {
        int cliend_id = m_clients[pSender];
        emit (receivedData(cliend_id, message.toStdString().c_str()));
    }
}
// _____________________________________________________________________
void CWebSocketServer::processMessage(QByteArray message)
{
    QWebSocket *pSender = qobject_cast<QWebSocket *>(sender());
    if (m_clients.contains(pSender)) {
        int cliend_id = m_clients[pSender];
        emit (receivedData(cliend_id, message));
    }
}

// _____________________________________________________________________
void CWebSocketServer::socketDisconnected()
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
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
void CWebSocketServer::open(tNtwConfig cfg)
{
    //qDebug() << "CWebSocketServer::open";
    if (!m_pWebSocketServer) {
        m_pWebSocketServer = new QWebSocketServer(QStringLiteral("WebSocketTcpServer"),
                                                  QWebSocketServer::NonSecureMode,
                                                  this);
    }
    unsigned long port = cfg["port"].toUInt();
    QString errorMsg = "Unable to connect WebSocketServer to port " +  QString::number(port);
    if (m_pWebSocketServer) {
        if (m_pWebSocketServer->listen(QHostAddress::Any, port))
        {
            connect(m_pWebSocketServer, SIGNAL(newConnection()),
                    this,               SLOT(onNewConnection()));
            QString msg = "WebSocketServer listening on port " + QString::number(port);
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
void CWebSocketServer::close()
{
    //qDebug() << "CWebSocketServer::close";
    for(QMap<QWebSocket *, int>::iterator it=m_clients.begin(); it!=m_clients.end(); it++) {
        it.key()->deleteLater();
    }
    m_clients.clear();
    m_clients_rev.clear();
    m_client_count = 0;
}

// _____________________________________________________________________
void CWebSocketServer::write(int client_id, QByteArray data)
{
    //qDebug() << "CWebSocketServer::write" << client_id << data;
    QWebSocket *pClient = NULL;
    if (m_clients_rev.contains(client_id)) {
        pClient = m_clients_rev[client_id];
    }
    if (pClient) {
        if (m_transmission_mode == BINARY_MODE) pClient->sendBinaryMessage(data);
        else                                    pClient->sendTextMessage(data);
        emit bytesWritten(client_id, data.size());
    }
}

