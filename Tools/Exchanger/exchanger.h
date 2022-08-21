/*! \file exchanger.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _EXCHANGER_H_
#define _EXCHANGER_H_

#include <QVariant>
#include <QTcpSocket>

/*! \addtogroup Exchanger
   *
   *  @{
   */

/*! @brief class CExchanger
 * Implémente une classe de communication entre 2 applications (locales ou distantes) par une communication Ethernet
 * Elle est utilisée dans un mécanisme client / serveur et est commune aux 2.
 */
class CExchanger : public QObject
{
    Q_OBJECT
public:
    explicit CExchanger(QObject *parent = nullptr);
    virtual ~CExchanger();

    void setSocket(QTcpSocket *socket);

protected :
    typedef enum {
        // PREDEFINED MESSAGES TYPES
        DATA_STRING = 0,
        DATA_UINT8,
        DATA_UINT16,
        DATA_UINT32,
        DATA_UINT64,
        DATA_INT8,
        DATA_INT16,
        DATA_INT32,
        DATA_INT64,
        DATA_DOUBLE,
        DATA_VARIANT,
        DATA_RAW_DATA,
        // __________________________ DO NOT EDIT AFTER
        LAST_DATA_TYPE
    }tExchangerDataType;

    const quint8 START_MESSAGE_ID_BYTE1 = 0xA5;
    const quint8 START_MESSAGE_ID_BYTE2 = 0x69;
    const quint8 START_MESSAGE_ID_BYTE3 = 0x5A;
    const quint8 START_MESSAGE_ID_BYTE4 = 0x96;
    const quint32 START_MESSAGE_ID = ((quint32)START_MESSAGE_ID_BYTE1<<24) | ((quint32)START_MESSAGE_ID_BYTE2<<16) | ((quint32)START_MESSAGE_ID_BYTE3<<8) | ((quint32)START_MESSAGE_ID_BYTE4);

    void encode(int msg_type, QByteArray &data);
    void decode(const QByteArray &rcv_buff);

    //! Demande d'envoie d'un buffer encodé vers le canal de communication : à réimplémenter par la classe fille pour faire le lien avec la socket (client ou serveur)
    virtual void send(const QByteArray &buff_data);

    virtual void receive_complete_message(const QByteArray &buff_data);         //! Réception d'un buffer de donnée à décoder en provenance du canal de communication

public slots :
    void send_string(QString str);
    void send_uint8(quint8 val);
    void send_uint16(quint16 val);
    void send_uint32(quint32 val);
    void send_uint64(quint64 val);
    void send_int8(qint8 val);
    void send_int16(qint16 val);
    void send_int32(qint32 val);
    void send_int64(qint64 val);
    void send_double(double val);
    void send_variant(QVariant val);
    void send_raw_buffer(const QByteArray &rawdata);

signals :
    void receive_string (QString str);
    void receive_uint8 (quint8 val);
    void receive_uint16 (quint16 val);
    void receive_uint32 (quint32 val);
    void receive_uint64 (quint64 val);
    void receive_int8 (quint8 val);
    void receive_int16 (quint16 val);
    void receive_int32 (qint32 val);
    void receive_int64 (qint64 val);
    void receive_double (double val);
    void receive_variant (QVariant val);
    void receive_raw_buffer(QByteArray rawdata);

protected :
    QByteArray m_user_message;
    quint32 m_user_message_size;
    quint32 m_user_message_notsize;
    quint32 m_message_checksum;
    quint32 m_computed_checksum;
    int m_decode_state;

    QTcpSocket *m_socket;

    enum {
        STATE_START_SEQUENCE_ID_BYTE1 = 0,  // MSB
        STATE_START_SEQUENCE_ID_BYTE2,
        STATE_START_SEQUENCE_ID_BYTE3,
        STATE_START_SEQUENCE_ID_BYTE4,      // LSB
        STATE_SIZE_BYTE1,
        STATE_SIZE_BYTE2,
        STATE_SIZE_BYTE3,
        STATE_SIZE_BYTE4,
        STATE_NOTSIZE_BYTE1,
        STATE_NOTSIZE_BYTE2,
        STATE_NOTSIZE_BYTE3,
        STATE_NOTSIZE_BYTE4,
        STATE_USER_MESSAGE_BYTES,
        STATE_CHECKSUM_BYTE1,
        STATE_CHECKSUM_BYTE2,
        STATE_CHECKSUM_BYTE3,
        STATE_CHECKSUM_BYTE4
    }eDecodeState;

public slots :
    void onSocketDataReady();
    void onSocketDataReady(QTcpSocket *socket);

};

#endif // _EXCHANGER_H_
