#include <QDataStream>
#include "exchanger.h"

CExchanger::CExchanger(QObject *parent)
    : QObject(parent),
      m_decode_state(STATE_START_SEQUENCE_ID_BYTE1),
      m_socket(Q_NULLPTR)
{
}

CExchanger::~CExchanger()
{
}

//______________________________________________________
void CExchanger::setSocket(QTcpSocket *socket)
{
    m_socket = socket;
    if (m_socket) {
        connect(m_socket, SIGNAL(readyRead()), this, SLOT(onSocketDataReady()));
    }
}

//______________________________________________________
// Un message est composé de :
//  1. 4 octets de START
//  2. Taille du message utilisateur
//  3. Le complémént de la taille du message utilisateur
//  4. Le message utilisateur
//  5. Le checksum sur 32 bits qui est la somme de :
//      > La taille du message utilisateur sur 32 bits
//      > Chaque octet du message utilisateur
// Définition du message utilisateur :
//      msg_type sur 32 bits et data
void CExchanger::encode(int msg_type, QByteArray &data)
{
    QByteArray buff;

    buff.append(START_MESSAGE_ID_BYTE1);
    buff.append(START_MESSAGE_ID_BYTE2);
    buff.append(START_MESSAGE_ID_BYTE3);
    buff.append(START_MESSAGE_ID_BYTE4);

    quint32 user_msg_size = quint32(data.size() + sizeof(quint32)); // Taille des données + taille de msg_type sur 4 octets
    buff.append((user_msg_size>>24)&0xFF);
    buff.append((user_msg_size>>16)&0xFF);
    buff.append((user_msg_size>>8)&0xFF);
    buff.append((user_msg_size)&0xFF);

    quint32  user_msg_notsize = (quint32)(~user_msg_size);
    buff.append((user_msg_notsize>>24)&0xFF);
    buff.append((user_msg_notsize>>16)&0xFF);
    buff.append((user_msg_notsize>>8)&0xFF);
    buff.append((user_msg_notsize)&0xFF);

    buff.append((msg_type>>24)&0xFF);
    buff.append((msg_type>>16)&0xFF);
    buff.append((msg_type>>8)&0xFF);
    buff.append((msg_type)&0xFF);

    buff.append(data);

    // Checksum final
    quint32 cs = 0;
    cs += user_msg_size;
    cs += (msg_type>>24)&0xFF;
    cs += (msg_type>>16)&0xFF;
    cs += (msg_type>>8)&0xFF;
    cs += (msg_type)&0xFF;
    foreach (quint8 bytedata, data) cs += bytedata;

    buff.append((cs>>24)&0xFF);
    buff.append((cs>>16)&0xFF);
    buff.append((cs>>8)&0xFF);
    buff.append((cs)&0xFF);

    send(buff);
}

//______________________________________________________
void CExchanger::onSocketDataReady()
{
    onSocketDataReady(m_socket);
}

//______________________________________________________
void CExchanger::onSocketDataReady(QTcpSocket *socket)
{
    if (!socket) return;
    decode( socket->readAll() );
}

//______________________________________________________
// Point d'entrée lorsqu'un buffer (message complet ou partiel) arrive
// Le buffer peut contenir :
//      - Un message complet
//      - Plusieurs messages complets
//      - Un morceau de message
//      - Un message complet et le début d'un autre
//      - La fin d'un message et le début d'un autre
// La machine d'état reconstitue le buffer reçu pour former un message cohérent
// Le message sera à son tour décodé pour en récupérer la donnée utilisateur qu'il véhiculait (QString, QVariant, int, double, ...)
void CExchanger::decode(const QByteArray &rcv_buff)
{
    foreach (quint8 data, rcv_buff) {
        switch (m_decode_state) {
        // ----------------------------
        // 1. La séquence de START : 4 octets avec une valeur sécuritaire
        case STATE_START_SEQUENCE_ID_BYTE1:
            if (data == START_MESSAGE_ID_BYTE1) m_decode_state++;
            break;
        case STATE_START_SEQUENCE_ID_BYTE2:
            if (data == START_MESSAGE_ID_BYTE2) m_decode_state++;
            else                                m_decode_state = STATE_START_SEQUENCE_ID_BYTE1;
            break;
        case STATE_START_SEQUENCE_ID_BYTE3:
            if (data == START_MESSAGE_ID_BYTE3) m_decode_state++;
            else                                m_decode_state = STATE_START_SEQUENCE_ID_BYTE1;
            break;
        case STATE_START_SEQUENCE_ID_BYTE4:
            if (data == START_MESSAGE_ID_BYTE4) m_decode_state = STATE_SIZE_BYTE1;
            else                                m_decode_state = STATE_START_SEQUENCE_ID_BYTE1;
            break;
        // ----------------------------
        // 2. La taille du message utilisateur sur 4 octets
        // (sans compter les 4 octets du checksum)
        case STATE_SIZE_BYTE1:
            m_user_message_size = (quint32)data << 24;
            m_decode_state++;
            break;
        case STATE_SIZE_BYTE2:
            m_user_message_size |= (quint32)data << 16;
            m_decode_state++;
            break;
        case STATE_SIZE_BYTE3:
            m_user_message_size |= (quint32)data << 8;
            m_decode_state++;
            break;
        case STATE_SIZE_BYTE4:
            m_user_message_size |= (quint32)data;
            m_decode_state = STATE_NOTSIZE_BYTE1;
            m_computed_checksum = m_user_message_size;
            break;
        // ----------------------------
        // 3. Sécurisation de la taille du message pour ne pas attendre 4 millions de data en cas de problème de synchro
        // On attend ici le complément à 1 de la taille du message
        case STATE_NOTSIZE_BYTE1:
            m_user_message_notsize = (quint32)data << 24;
            m_decode_state++;
            break;
        case STATE_NOTSIZE_BYTE2:
            m_user_message_notsize |= (quint32)data << 16;
            m_decode_state++;
            break;
        case STATE_NOTSIZE_BYTE3:
            m_user_message_notsize |= (quint32)data << 8;
            m_decode_state++;
            break;
        case STATE_NOTSIZE_BYTE4:
            m_user_message_notsize |= (quint32)data;
            if ( (m_user_message_size + m_user_message_notsize) == 0xFFFFFFFF ) {  //
                m_user_message.clear(); // efface les données du précédent message
                if (m_user_message_size != 0)   m_decode_state = STATE_USER_MESSAGE_BYTES;
                else                            m_decode_state = STATE_CHECKSUM_BYTE1;
            }
            // Problème de cohérence entre m_user_message_size et m_user_message_notsize
            else  m_decode_state = STATE_START_SEQUENCE_ID_BYTE1;
            break;
        // ----------------------------
        // 4. Attend la réception des "m_user_message_size" octets
        case STATE_USER_MESSAGE_BYTES :
            m_user_message.append(data);
            m_computed_checksum += data;
            m_user_message_size--;
            if (m_user_message_size==0) m_decode_state = STATE_CHECKSUM_BYTE1;
            // else : attend la suite des octets pour former le message utilisateur
            break;
        // ----------------------------
        // 5. Checksum sur 4 octets
        case STATE_CHECKSUM_BYTE1 :
            m_message_checksum = (quint32)data << 24;
            m_decode_state++;
            break;
        case STATE_CHECKSUM_BYTE2 :
            m_message_checksum |= (quint32)data << 16;
            m_decode_state++;
            break;
        case STATE_CHECKSUM_BYTE3 :
            m_message_checksum |= (quint32)data << 8;
            m_decode_state++;
            break;
        case STATE_CHECKSUM_BYTE4 :
            m_message_checksum |= (quint32)data;
            if (m_computed_checksum == m_message_checksum) {
                receive_complete_message(m_user_message);
            }
            m_decode_state = STATE_START_SEQUENCE_ID_BYTE1;
            break;
        // ----------------------------
        default:
            m_decode_state = STATE_START_SEQUENCE_ID_BYTE1;
            break;
        }
    }
}

//______________________________________________________
void CExchanger::send(const QByteArray &buff_data)
{
   if (!m_socket) return;
   m_socket->write(buff_data);
}

//______________________________________________________
/*!
 * \brief Méthode appelée (slot) lorsqu'un message complet est arrivé du canal de communication
 * \param buff_data
 */
void CExchanger::receive_complete_message(const QByteArray &buff_data)
{
    QDataStream stream(buff_data);
    int msg_type;
    quint8 ui8val;
    quint16 ui16val;
    quint32 ui32val;
    quint64 ui64val;
    qint8 i8val;
    qint16 i16val;
    qint32 i32val;
    qint64 i64val;
    double dval;
    QVariant varval;
    QString strval;
    QByteArray baval;

    stream >> msg_type;
    switch(msg_type)
    {
    case DATA_STRING :
        stream >> strval;
        emit receive_string(strval);
        break;
    case DATA_UINT8 :
        stream >> ui8val;
        emit receive_uint8(ui8val);
        break;
    case DATA_UINT16 :
        stream >> ui16val;
        emit receive_uint16(ui16val);
        break;
    case DATA_UINT32 :
        stream >> ui32val;
        emit receive_uint32(ui32val);
        break;
    case DATA_UINT64 :
        stream >> ui64val;
        emit receive_uint64(ui64val);
        break;
    case DATA_INT8 :
        stream >> i8val;
        emit receive_int8(i8val);
        break;
    case DATA_INT16 :
        stream >> i16val;
        emit receive_int16(i16val);
        break;
    case DATA_INT32 :
        stream >> i32val;
        emit receive_int32(i32val);
        break;
    case DATA_INT64 :
        stream >> i64val;
        emit receive_int64(i64val);
        break;
    case DATA_DOUBLE :
        stream >> dval;
        emit receive_double(dval);
        break;
    case DATA_VARIANT :
        stream >> varval;
        emit receive_variant(varval);
        break;
    case DATA_RAW_DATA :
        stream >> baval;
        emit receive_raw_buffer(baval);
        break;
    default :
        // ne rien faire
        break;
    }
}

// ================================================================
//                          API
// ================================================================
//______________________________________________________
/*!
 * \brief Sérialise la donnée à véhiculer et l'envoie
 * \param str : la chaine à transmettre
 */
void CExchanger::send_string(QString str)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);
    stream << str;
    encode(DATA_STRING, buff);
}

//______________________________________________________
/*!
 * \brief
 * \param val : la valeur à transmettre
 */
void CExchanger::send_uint8(quint8 val)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);
    stream << val;
    encode(DATA_UINT8, buff);
}
//______________________________________________________
/*!
 * \brief
 * \param val : la valeur à transmettre
 */
void CExchanger::send_uint16(quint16 val)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);
    stream << val;
    encode(DATA_UINT16, buff);
}
//______________________________________________________
/*!
 * \brief
 * \param val : la valeur à transmettre
 */
void CExchanger::send_uint32(quint32 val)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);
    stream << val;
    encode(DATA_UINT32, buff);
}
//______________________________________________________
/*!
 * \brief
 * \param val : la valeur à transmettre
 */
void CExchanger::send_uint64(quint64 val)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);
    stream << val;
    encode(DATA_UINT64, buff);
}

//______________________________________________________
/*!
 * \brief
 * \param val : la valeur à transmettre
 */
void CExchanger::send_int8(qint8 val)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);
    stream << val;
    encode(DATA_INT8, buff);
}
//______________________________________________________
/*!
 * \brief
 * \param val : la valeur à transmettre
 */
void CExchanger::send_int16(qint16 val)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);
    stream << val;
    encode(DATA_INT16, buff);
}
//______________________________________________________
/*!
 * \brief
 * \param val : la valeur à transmettre
 */
void CExchanger::send_int32(qint32 val)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);
    stream << val;
    encode(DATA_INT32, buff);
}
//______________________________________________________
/*!
 * \brief
 * \param val : la valeur à transmettre
 */
void CExchanger::send_int64(qint64 val)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);
    stream << val;
    encode(DATA_INT64, buff);
}

//______________________________________________________
/*!
 * \brief
 * \param val : la valeur à transmettre
 */
void CExchanger::send_double(double val)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);
    stream << val;
    encode(DATA_DOUBLE, buff);
}

//______________________________________________________
/*!
 * \brief
 * \param val : la valeur à transmettre
 */
void CExchanger::send_variant(QVariant val)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);
    stream << val;
    encode(DATA_VARIANT, buff);
}

//______________________________________________________
/*!
 * \brief
 * \param val : la valeur à transmettre
 */
void CExchanger::send_raw_buffer(const QByteArray &rawdata)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);
    stream << rawdata;
    encode(DATA_RAW_DATA, buff);
}



