#ifndef DATA_CLIENT_H
#define DATA_CLIENT_H

#include "exchangerclient.h"
#include "exchanger_data.h"

class DataClient : public ExchangerClient
{
    Q_OBJECT
public:
    explicit DataClient(QObject *parent = nullptr);

    CExchangerData m_exchanger;
};

#endif // DATA_CLIENT_H
