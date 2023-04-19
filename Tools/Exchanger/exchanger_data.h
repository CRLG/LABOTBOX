#ifndef EXCHANGER_DATA_H
#define EXCHANGER_DATA_H
#include "exchanger.h"
#include "docdesigner.h"

class CExchangerData : public CExchanger
{
    Q_OBJECT
public:
    explicit CExchangerData(QObject *parent=nullptr);
    void send_data(QString name, QVariant val);
    void send_datas(QHash<QString, QVariant> datas);
    void send_add_data_request(QString name);
    void send_remove_data_request(QString name);
    void send_start_stop_transmission_request(bool start_stop);

protected :
    typedef enum {
        MSG_DATA = LAST_DATA_TYPE,  // Envoie d'une donnée
        MSG_DATAS,                  // Envoie de plusieurs données d'un seul coup
        MSG_ADD_DATA_REQUEST,       // Envoie d'une requête de configuration d'une donnée à ajouter au transfert
        MSG_REMOVE_DATA_REQUEST,    // Envoie d'une requête de configuration d'une donnée à arrêter de transférer
        MSG_START_STOP_TRANSMISSION_REQUEST, // Envoie d'une requête de démarrage / arrêt du transfert
    }tMsgTypeData;

    /*virtual*/ void receive_complete_message(const QByteArray &buff_data);          //! Réception d'un buffer de donnée à décoder en provenance du canal de communication

signals :
    void receive_data(QString name, QVariant value);
    void receive_datas(QHash<QString, QVariant> datas);
    void receive_add_data_request(QString name);
    void receive_remove_data_request(QString name);
    void receive_start_stop_request(bool start_stop);
};

#endif // EXCHANGER_DATA_H

