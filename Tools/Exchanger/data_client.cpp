#include "data_client.h"

DataClient::DataClient(ExchangerClient *parent)
    : ExchangerClient(parent)
{
    m_exchanger.setSocket(this);
}

