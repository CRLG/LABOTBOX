#ifndef EXCHANGER_TELEMETRY_H
#define EXCHANGER_TELEMETRY_H
#include "exchanger.h"
#include "docdesigner.h"

class CExchangerTelemetry : public CExchanger
{
    Q_OBJECT
public:
    explicit CExchangerTelemetry(CExchanger *parent=nullptr);
    void send_doc_designer(const DocDesigner &docdesigner);

protected :
    typedef enum {
        MSG_DOCDESIGNER = LAST_DATA_TYPE
    }tMsgTypeTelemetry;

    /*virtual*/ void receive_complete_message(const QByteArray &buff_data);          //! Réception d'un buffer de donnée à décoder en provenance du canal de communication

signals :
    void receive_doc_designer(QString docdesigner);
};

#endif // EXCHANGER_TELEMETRY_H
