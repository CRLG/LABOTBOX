#ifndef TELEMETRY_SERVER_H
#define TELEMETRY_SERVER_H

#include "exchangerserver.h"
#include "exchanger_telemetry.h"

class TelemetryServer : public CExchangerServer
{
    Q_OBJECT
public:
    explicit TelemetryServer(CExchangerServer *parent = nullptr);

    CExchangerTelemetry m_exchanger;
};

#endif // TELEMETRY_SERVER_H
