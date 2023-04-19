#include "data_client.h"

DataClient::DataClient(QObject *parent)
    : ExchangerClient(parent)
{
    m_exchanger.setSocket(this);
}

