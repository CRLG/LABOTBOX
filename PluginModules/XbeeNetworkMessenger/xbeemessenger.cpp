#include <QDebug>
#include "xbeemessenger.h"
#include "CApplication.h"
#include "CDataManager.h"
#include "CXbeeNetworkMessenger.h"

XbeeMessenger::XbeeMessenger()
{
    m_time.restart();
}

XbeeMessenger::~XbeeMessenger()
{
}

// ______________________________________________
void XbeeMessenger::initApp(CApplication *application)
{
    m_application = application;
    initMessenger();
}

// ______________________________________________
void XbeeMessenger::initMessenger()
{
    init(&m_transporter, &m_database);
}


// ______________________________________________
void XbeeMessenger::test()
{
    // Test d'envoie d'un message qui devrait solliciter le driver XBEE
    Message_TIMESTAMP_MATCH *msg = &m_database.m_TimestampMatch;
    msg->Timestamp++;
    msg->setDestinationAddress(0xFFFF);
    //msg->send();
}



// ==========================================
//   METHODS FROM MessengerInterfaceBase
// ==========================================
// ______________________________________________
void XbeeMessenger::encode(unsigned char *buff_data, unsigned short buff_size, unsigned short dest_address)
{
    qDebug() << "MessengerInterface::encode" << buff_size << "data(s) / destination address" << dest_address;
    qDebug() << QByteArray((char*)buff_data, buff_size);

    m_application->m_XbeeNetworkMessenger->messengerEncodeRequest(buff_data, buff_size, dest_address);
    //m_xbee_driver.encode(buff_data, buff_size, dest_address);
}

// ______________________________________________
void XbeeMessenger::newFrameReceived(tMessengerFrame *frame)
{
    //qDebug() << "MessengerEvent::A frame with ID" << frame->ID << "received from address" << frame->SourceAddress << m_database.nodeIdToName(frame->SourceAddress);
}

// ______________________________________________
void XbeeMessenger::newMessageReceived(MessageBase *msg)
{
    //qDebug() << "MessengerEvent::New Message" << msg->getName() << "received from address " << msg->getSourceAddress();
}

// ______________________________________________
void XbeeMessenger::frameTransmited(tMessengerFrame *frame)
{
    //qDebug() << "MessengerEvent::A Frame with ID" << frame->ID << "was transmited by Messenger to destination :" << frame->DestinationAddress;
}

// ______________________________________________
void XbeeMessenger::messageTransmited(MessageBase *msg)
{
    //qDebug() << "MessengerEvent::Message" << msg->getName() << "was transmited to destination :" << msg->getDestinationAddress();
}

// ______________________________________________
void XbeeMessenger::dataUpdated(MessageBase *msg, char *name, char *val_str)
{
    //qDebug() << "MessengerEvent::Signal" << name << "updated with value" << val_str;
    m_application->m_data_center->write(msg->getName() + QString(".") + QString(name), QVariant(val_str));
    QString str = QString::number(getTime()) + ": " + msg->getName() + "." + QString(name) + "=" + QString(val_str);
    m_application->m_XbeeNetworkMessenger->messengerDisplayAnalyzer(str);
}

// ______________________________________________
void XbeeMessenger::dataChanged(MessageBase *msg, char *name, char *val_str)
{
    //qDebug() << "Signal" << name << "changed with value" << val_str;
}

// ______________________________________________
void XbeeMessenger::nodeCommunicationStatusChanged(NodeBase *node)
{
    //qDebug() << "Node Communication status changed:" << node->getName() << ": com is " << node->isPresent();
    m_application->m_data_center->write("XbeeMsngNodePresent." + QString(node->getName()), QVariant(node->isPresent()));
}

// ______________________________________________
long XbeeMessenger::getTime()
{
   return m_time.elapsed();
}

