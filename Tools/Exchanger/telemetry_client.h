#ifndef TELEMETRY_CLIENT_H
#define TELEMETRY_CLIENT_H

#include "exchangerclient.h"
#include "exchanger_telemetry.h"

class TelemetryClient : public ExchangerClient
{
    Q_OBJECT
public:
    explicit TelemetryClient(ExchangerClient *parent = nullptr);

    CExchangerTelemetry m_exchanger;
};

#endif // TELEMETRY_CLIENT_H
