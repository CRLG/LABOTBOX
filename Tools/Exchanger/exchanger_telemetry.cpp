#include <QDebug>
#include "exchanger_telemetry.h"

CExchangerTelemetry::CExchangerTelemetry(CExchanger *parent)
    : CExchanger(parent)
{
}

//______________________________________________________
/*!
 * \brief Méthode appelée lorsqu'un message complet est arrivé du canal de communication
 * \param buff_data
 */
void CExchangerTelemetry::receive_complete_message(const QByteArray &buff_data)
{
    CExchanger::receive_complete_message(buff_data);

    QDataStream stream(buff_data);
    int msg_type;

    QString str;

    while (!stream.atEnd()) {
        stream >> msg_type;
        switch(msg_type)
        {
        case MSG_DOCDESIGNER :
            stream >> str;
            emit receive_doc_designer(str);
            break;

        }
    }
}

//______________________________________________________
/*!
 * \brief Méthode appelée (slot) lorsqu'un message complet est arrivé du canal de communication
 * \param docdesigner
 */
void CExchangerTelemetry::send_doc_designer(const DocDesigner &docdesigner)
{
    QByteArray buff;
    QDataStream stream(&buff, QIODevice::ReadWrite);

    stream << docdesigner.toHtml();
    encode(MSG_DOCDESIGNER, buff);
}
