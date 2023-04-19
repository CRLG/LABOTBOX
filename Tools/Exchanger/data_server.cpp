#include "data_server.h"

DataServer::DataServer(QObject *parent)
    : CExchangerServer(parent)
{
    setExchanger(&m_exchanger);
}

