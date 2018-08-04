/*! \file CDataView.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CTCP_SERVER_H_
#define _CTCP_SERVER_H_

#include <QObject>

#include "CNetworkServerInterface.h"

 /*! \addtogroup NETWORK_SERVER
   * 
   *  @{
   */

class QTcpServer;
class QTcpSocket;
	   
/*! @brief class CWekSocketServer
 */	   
class CTcpServer : public CNetworkServerInterface
{
    Q_OBJECT

public:
    explicit CTcpServer(QObject *parent = Q_NULLPTR);
    virtual ~CTcpServer();

    virtual tNtwServerType getType();

signals :
        void opened(const QString &msg);
        void closed();
        void receivedData(int client_id, QByteArray data);
        void connected(int client_id);
        void disconnected(int client_id);
        void error(int cliend_id);
        void bytesWritten(int cliend_id, qint64 bytes);
        void errorMessage(QString msg);

public slots :
        virtual void write(int client_id, QByteArray data);
        virtual void open(tNtwConfig cfg);
        virtual void close();

private slots :
    void onNewConnection();
    void processMessage();
    void socketDisconnected();
private :
        QTcpServer *m_pTcpServer;
        QMap<QTcpSocket *, int> m_clients;
        QMap<int, QTcpSocket *> m_clients_rev;
        int m_client_count;
};

#endif // _CTCP_SERVER_H_

/*! @} */

