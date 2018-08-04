/*! \file CWebSocketServer.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>

#include "CNetworkServerFactory.h"
#include "CWebSocketServer.h"
#include "CTcpServer.h"


/*! \addtogroup NETWORK_SERVER
   * 
   *  @{
   */
	   
// _____________________________________________________________________
CNetworksServerFactory::CNetworksServerFactory()
{
}


// _____________________________________________________________________
CNetworksServerFactory::~CNetworksServerFactory()
{
}

// _____________________________________________________________________
CNetworkServerInterface* CNetworksServerFactory::createInstance(tNtwServerType server_type, QObject *parent)
{
    switch(server_type)
    {
        case WEBSOCKET_SERVER :
            return new CWebSocketServer(parent);
        case TCP_SERVER :
            return new CTcpServer(parent);

        default :
            qDebug() << "Can't instantiate unknown server type";
    }
    return NULL;
}

// _____________________________________________________________________
CNetworkServerInterface* CNetworksServerFactory::createInstance(QString server_type_name, QObject *parent)
{
    return createInstance(serverTypeNameToEnum(server_type_name), parent);
}

// _____________________________________________________________________
tNtwServerType CNetworksServerFactory::serverTypeNameToEnum(QString name)
{
    if (name.toLower().contains("tcp"))
        return TCP_SERVER;
    else if (name.toLower().contains("websocket"))
        return WEBSOCKET_SERVER;

    return UNKOWN_SERVER_TYPE;
}
