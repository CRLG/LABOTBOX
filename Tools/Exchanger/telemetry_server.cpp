#include "telemetry_server.h"

TelemetryServer::TelemetryServer(CExchangerServer *parent)
    : CExchangerServer(parent)
{
    setExchanger(&m_exchanger);
}

