#include "data_server.h"

DataServer::DataServer(CExchangerServer *parent)
    : CExchangerServer(parent)
{
    setExchanger(&m_exchanger);
}

