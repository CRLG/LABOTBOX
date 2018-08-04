/*! \file CNetworkServerFactory.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CNETWORK_SERVER_FACTORY_H_
#define _CNETWORK_SERVER_FACTORY_H_

#include <QObject>

#include "CNetworkServerInterface.h"

 /*! \addtogroup NETWORK_SERVER
   * 
   *  @{
   */

	   
/*! @brief class CNetworksServerFactory
 */	   
class CNetworksServerFactory
{
public:
    explicit CNetworksServerFactory();
    virtual ~CNetworksServerFactory();

    static CNetworkServerInterface* createInstance(tNtwServerType server_type, QObject *parent = Q_NULLPTR);
    static CNetworkServerInterface* createInstance(QString server_type_name, QObject *parent = Q_NULLPTR);
    static tNtwServerType serverTypeNameToEnum(QString name);
};

#endif // _CNETWORK_SERVER_FACTORY_H_

/*! @} */

