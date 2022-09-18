#ifndef EXCHANGER_DATA_H
#define EXCHANGER_DATA_H
#include "exchanger.h"
#include "docdesigner.h"

class CExchangerData : public CExchanger
{
    Q_OBJECT
public:
    explicit CExchangerData(CExchanger *parent=nullptr);
    void send_data(QString name, QVariant val);
    void send_datas(QHash<QString, QVariant> datas);

protected :
    typedef enum {
        MSG_DATA = LAST_DATA_TYPE,
        MSG_DATAS,
    }tMsgTypeData;

    /*virtual*/ void receive_complete_message(const QByteArray &buff_data);          //! Réception d'un buffer de donnée à décoder en provenance du canal de communication

signals :
    void receive_data(QString name, QVariant value);
    void receive_datas(QHash<QString, QVariant> datas);
};

#endif // EXCHANGER_DATA_H

