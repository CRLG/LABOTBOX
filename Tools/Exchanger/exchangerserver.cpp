#include <QDataStream>
#include "exchangerserver.h"

CExchangerServer::CExchangerServer(QTcpServer *parent)
    : QTcpServer(parent),
      m_p_exchanger(Q_NULLPTR)
{
}

// ----------------------------------------------------
bool CExchangerServer::start(int port)
{
    bool state;
    state = listen(QHostAddress::Any, port);
    // Démarrage du serveur sur toutes les IP disponibles et sur le port 50585
    if (state) {
        // Si le serveur a été démarré correctement
        connect(this, SIGNAL(newConnection()), this, SLOT(new_connection()));
        emit started();
    }
    return state;
}

// ----------------------------------------------------
void CExchangerServer::setExchanger(CExchanger *exchanger)
{
    m_p_exchanger = exchanger;
}

// ----------------------------------------------------
void CExchangerServer::new_connection()
{
    if (m_clients.size() >0 ) return; // limite le nombre de connexion à 1 (un seul client connecté au serveur pour le moment)

    QTcpSocket *new_client = nextPendingConnection();
    m_clients << new_client;
    connect(new_client, SIGNAL(disconnected()), this, SLOT(client_disconnected()));
    emit connectionToSocket(new_client);
    if (m_p_exchanger) m_p_exchanger->setSocket(new_client);
}

// ----------------------------------------------------
void CExchangerServer::client_disconnected()
{
    // On détermine quel client se déconnecte
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if ( socket == 0 )
        // Si on n'a pas trouvé le client à l'origine du signal, on arrête la méthode
        return;
    m_clients.removeOne(socket);
    socket->deleteLater();
    if (m_p_exchanger) m_p_exchanger->setSocket(Q_NULLPTR);
}
