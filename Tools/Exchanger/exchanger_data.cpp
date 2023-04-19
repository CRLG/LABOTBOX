#include <QDebug>
#include <QHash>
#include "exchanger_data.h"

CExchangerData::CExchangerData(QObject *parent)
    : CExchanger(parent)
{
}

//______________________________________________________
/*!
 * \brief Méthode appelée lorsqu'un message complet est arrivé du canal de communication
 * \param buff_data
 */
void CExchangerData::receive_complete_message(const QByteArray &buff_data)
{
    QDataStream stream(buff_data);
    int msg_type;

    QString str;
    QVariant variant;
    bool bval;
    QHash<QString, QVariant> datas;

    stream >> msg_type;
    switch(msg_type)
    {
   case MSG_DATA :
        stream >> str;
        stream >> variant;
        emit receive_data(str, variant);
        break;
    case MSG_DATAS :
        stream >> datas; // Un QHash peut directement être utilisé avec un stream
        emit receive_datas(datas);
        break;
    case MSG_ADD_DATA_REQUEST :
         stream >> str;
         emit receive_add_data_request(str);
         break;
    case MSG_REMOVE_DATA_REQUEST :
         stream >> str;
         emit receive_remove_data_request(str);
         break;
    case MSG_START_STOP_TRANSMISSION_REQUEST :
         stream >> bval;
         emit receive_start_stop_request(bval);
         break;
    default :  // tente le décodage d'une donnée basic (int, double, QString, QVariant, ...)
        CExchanger::receive_complete_message(buff_data);
        break;
    }
}

//______________________________________________________
void CExchangerData::send_data(QString name, QVariant val)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);

    stream << name;
    stream << val;
    encode(MSG_DATA, buff);
}

//______________________________________________________
void CExchangerData::send_datas(QHash<QString, QVariant> datas)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);

    stream << datas; // Un QHash peut directement être utilisé avec un stream
    encode(MSG_DATAS, buff);
}

//______________________________________________________
void CExchangerData::send_add_data_request(QString name)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);

    stream << name;
    encode(MSG_ADD_DATA_REQUEST, buff);
}
//______________________________________________________
void CExchangerData::send_remove_data_request(QString name)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);

    stream << name;
    encode(MSG_REMOVE_DATA_REQUEST, buff);
}

//______________________________________________________
void CExchangerData::send_start_stop_transmission_request(bool start_stop)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);

    stream << start_stop;
    encode(MSG_START_STOP_TRANSMISSION_REQUEST, buff);
}
