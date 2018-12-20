#include <QDebug>
#include "MessengerXbeeNetwork2019.h"
#include "CApplication.h"
#include "CDataManager.h"
#include "CRS232.h"

MessengerXbeeNetwork::MessengerXbeeNetwork()
{
    init(&m_transporter, &m_database);
}

MessengerXbeeNetwork::~MessengerXbeeNetwork()
{
}

// ______________________________________________
void MessengerXbeeNetwork::initApp(CApplication *application)
{
    m_application = application;
}

// ===================================================
//              MESSENGER OUTPUT
// ===================================================
// ______________________________________________
void MessengerXbeeNetwork::encode(unsigned char *buff_data, unsigned short buff_size, unsigned short dest_address)
{
    qDebug() << "Messenger::encode" << buff_size << "data(s) / destination address" << dest_address;
    qDebug() << QByteArray((char*)buff_data, buff_size);
    m_application->m_RS232_xbee->write((char*)buff_data, buff_size);
}

// ===================================================
//                  MESSENGER EVENTS
// ===================================================
// ______________________________________________
void MessengerXbeeNetwork::newFrameReceived(tMessengerFrame *frame)
{
    qDebug() << "Messenger::A frame with ID" << frame->ID << "received from address" << frame->SourceAddress;
}

// ______________________________________________
void MessengerXbeeNetwork::newMessageReceived(MessageBase *msg)
{
    qDebug() << "Messenger::New Message" << msg->getName() << "received from address " << msg->getSourceAddress();
}

// ______________________________________________
void MessengerXbeeNetwork::frameTransmited(tMessengerFrame *frame)
{
    qDebug() << "Messenger::A Frame with ID" << frame->ID << "was transmited by Messenger to destination :" << frame->DestinationAddress;

}

// ______________________________________________
void MessengerXbeeNetwork::messageTransmited(MessageBase *msg)
{
    qDebug() << "Messenger::Message" << msg->getName() << "was transmited to destination :" << msg->getDestinationAddress();
}

// ______________________________________________
void MessengerXbeeNetwork::dataUpdated(char *name, char *val_str)
{
    qDebug() << "Messenger::Signal" << name << "updated with value" << val_str;
    m_application->m_data_center->write(QString(name), QVariant(val_str));
}

// ______________________________________________
void MessengerXbeeNetwork::dataChanged(char *name, char *val_str)
{
    qDebug() << "Signal" << name << "changed with value" << val_str;
}


// ===================================================
//                  LOCAL METHODS
// ===================================================
// ______________________________________________
void MessengerXbeeNetwork::test_RX()
{
    // Simulate data reception
    // Message_TIMESTAMP_MATCH : ID 0x0001
    unsigned char data[] = { 'T', 0x00, 0x01, 0x02, 0x00, 0x11, 0x14 };
    for (unsigned int i=0; i<sizeof(data);  i++) {
        decode(data[i]);
    }
}

void MessengerXbeeNetwork::test_TX()
{
    // Send message
    Message_EXPERIENCE_STATUS *msg = &m_database.m_ExperienceStatus;
    msg->ExperienceStatus++;// = 0xABCD;
    //m_database.m_ExperienceStatus.setDestinationAddress(98);
    msg->send();
}
