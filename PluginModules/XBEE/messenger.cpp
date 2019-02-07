#include <QDebug>
#include "messenger.h"
#include "CApplication.h"
#include "CDataManager.h"

RobNetworkMessenger::RobNetworkMessenger()
{
}

RobNetworkMessenger::~RobNetworkMessenger()
{
}

// ______________________________________________
void RobNetworkMessenger::initApp(CApplication *application)
{
    m_application = application;
    initMessenger();
}

// ______________________________________________
void RobNetworkMessenger::initMessenger()
{
    init(&m_transporter, &m_database);
    m_xbee_driver.setMessengerInterface(this);
    m_xbee_driver.setApplication(m_application);
}


// ______________________________________________
void RobNetworkMessenger::initXbee()
{
    // TODO : relier ces paramètres avec le fichier de configuration du XBEE
    // Créer une fonction spéciale qui est appelée depuis CXBEE
    tXbeeSettings xbee_settings;
    xbee_settings.APIMODE = '1';
    strcpy(xbee_settings.CHANNEL, "0E");
    xbee_settings.COORDINATOR = '1';
    xbee_settings.COORDINATOR_OPTION = 0x04;
    strcpy(xbee_settings.PANID, "3321");
    strcpy(xbee_settings.KEY, "6910DEA76FC0328DEBB4307854EDFC42");
    xbee_settings.ID = '1';
    xbee_settings.SECURITY = '1';
    m_xbee_driver.init(xbee_settings);
}

// ______________________________________________
void RobNetworkMessenger::test()
{
    // Test d'envoie d'un message qui devrait solliciter le driver XBEE
    Message_TIMESTAMP_MATCH *msg = &m_database.m_TimestampMatch;
    msg->Timestamp++;
    msg->setDestinationAddress(0xFFFF);
    msg->send();
}

// ______________________________________________
void RobNetworkMessenger::receiveSerialDatas(QByteArray datas)
{
    for (int i=0; i<datas.size(); i++)
    {
        m_xbee_driver.decode(datas.at(i));
    }
}

// ==========================================
//   METHODS FROM MessengerInterfaceBase
// ==========================================
// ______________________________________________
void RobNetworkMessenger::encode(unsigned char *buff_data, unsigned short buff_size, unsigned short dest_address)
{
    qDebug() << "MessengerInterface::encode" << buff_size << "data(s) / destination address" << dest_address;
    qDebug() << QByteArray((char*)buff_data, buff_size);

    m_xbee_driver.encode(buff_data, buff_size, dest_address);
}

// ______________________________________________
void RobNetworkMessenger::newFrameReceived(tMessengerFrame *frame)
{
    qDebug() << "MessengerEvent::A frame with ID" << frame->ID << "received from address" << frame->SourceAddress;
}

// ______________________________________________
void RobNetworkMessenger::newMessageReceived(MessageBase *msg)
{
    qDebug() << "MessengerEvent::New Message" << msg->getName() << "received from address " << msg->getSourceAddress();
}

// ______________________________________________
void RobNetworkMessenger::frameTransmited(tMessengerFrame *frame)
{
    qDebug() << "MessengerEvent::A Frame with ID" << frame->ID << "was transmited by Messenger to destination :" << frame->DestinationAddress;
}

// ______________________________________________
void RobNetworkMessenger::messageTransmited(MessageBase *msg)
{
    qDebug() << "MessengerEvent::Message" << msg->getName() << "was transmited to destination :" << msg->getDestinationAddress();
}

// ______________________________________________
void RobNetworkMessenger::dataUpdated(char *name, char *val_str)
{
    qDebug() << "MessengerEvent::Signal" << name << "updated with value" << val_str;
    m_application->m_data_center->write(QString(name), QVariant(val_str));
}

// ______________________________________________
void RobNetworkMessenger::dataChanged(char *name, char *val_str)
{
    qDebug() << "Signal" << name << "changed with value" << val_str;
}
