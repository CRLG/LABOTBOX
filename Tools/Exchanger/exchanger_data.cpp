#include <QDebug>
#include <QHash>
#include "exchanger_data.h"

CExchangerData::CExchangerData(CExchanger *parent)
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
