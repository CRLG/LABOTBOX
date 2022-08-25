#include "telemetry_client.h"

TelemetryClient::TelemetryClient(ExchangerClient *parent)
    : ExchangerClient(parent)
{
    m_exchanger.setSocket(this);
}

