/*! \file CNetworkServerInterface.h
 * A brief Classe de base pour tous les serveurs réseau.
 * A more elaborated file description.
 */
#ifndef _CNETWORK_SERVER_INTERFACE_H_
#define _CNETWORK_SERVER_INTERFACE_H_

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QVariant>
#include <QMap>

 /*! \addtogroup NETWORK_SERVER
   * 
   *  @{
   */

typedef enum {
    UNKOWN_SERVER_TYPE = 0,
    TCP_SERVER,
    WEBSOCKET_SERVER
}tNtwServerType;

// QMap : QString: Nom de l'option / QByteArray: Valeur de l'option
typedef QMap<QString, QVariant> tNtwConfig;

/*! @brief classe de base pour tous les serveurs.
 */	   
class CNetworkServerInterface : public QObject
{
    Q_OBJECT
public:
    CNetworkServerInterface(QObject *parent = Q_NULLPTR)
        : QObject(parent)
    { }
    virtual ~CNetworkServerInterface() { }

    virtual tNtwServerType getType() = 0;

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
       virtual void write(int client_id, QByteArray data) = 0;
       virtual void open(tNtwConfig cfg) = 0;
       virtual void close() = 0;//{ }
};

#endif // _CNETWORK_SERVER_INTERFACE_H_

/*! @} */

