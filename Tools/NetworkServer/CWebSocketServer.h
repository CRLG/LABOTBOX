/*! \file CDataView.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CWEBSOCKET_SERVER_H_
#define _CWEBSOCKET_SERVER_H_

#include <QObject>

#include "CNetworkServerInterface.h"

 /*! \addtogroup NETWORK_SERVER
   * 
   *  @{
   */

class QWebSocketServer;
class QWebSocket;

/*! @brief class CWekSocketServer
 */	   
class CWebSocketServer : public CNetworkServerInterface
{
    Q_OBJECT

private :
    typedef enum {
        TEXT_MODE = 0,
        BINARY_MODE
    }tTransmissionMode;

public:
    explicit CWebSocketServer(QObject *parent = Q_NULLPTR);
    virtual ~CWebSocketServer();

    virtual tNtwServerType getType();
    void setTransmissionMode(tTransmissionMode mode);

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
    void processMessage(QString message);
    void processMessage(QByteArray message);
    void socketDisconnected();

private:
    QWebSocketServer *m_pWebSocketServer;
    QMap<QWebSocket *, int> m_clients;
    QMap<int, QWebSocket *> m_clients_rev;
    int m_client_count;

    tTransmissionMode m_transmission_mode;
};

#endif // _CWEBSOCKET_SERVER_H_

/*! @} */

