#include "telemetry_client.h"

TelemetryClient::TelemetryClient(QTcpSocket *parent)
    : QTcpSocket(parent)
{
    m_exchanger.setSocket(this);
}

