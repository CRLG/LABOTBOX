#ifndef TELEMETRY_CLIENT_H
#define TELEMETRY_CLIENT_H

#include <QTcpSocket>
#include "exchanger_telemetry.h"

class TelemetryClient : public QTcpSocket
{
    Q_OBJECT
public:
    explicit TelemetryClient(QTcpSocket *parent = nullptr);

    CExchangerTelemetry m_exchanger;
};

#endif // TELEMETRY_CLIENT_H
