#include "MessengerXbeeNetwork_simu.h"
#include "messagebase.h"
#include "CGlobale.h"

#include <QDateTime>
#include <QDebug>
#include "CApplication.h"
#include "CDataManager.h"

MessengerXbeeNetworkSimu::MessengerXbeeNetworkSimu()
{
    MessengerInterfaceBase::init(&m_transporter, &m_database);
    initMessages();
}

MessengerXbeeNetworkSimu::~MessengerXbeeNetworkSimu()
{
}

void MessengerXbeeNetworkSimu::init(CApplication *application)
{
    m_application = application;
}


// ______________________________________________
void MessengerXbeeNetworkSimu::initMessages()
{
    m_database.m_TimestampMatch.setDestinationAddress(0xFFFF);
    m_database.m_TimestampMatch.setTransmitPeriod(1000);

    m_database.m_GrosbotPosition.setDestinationAddress(0xFFFF);
    m_database.m_GrosbotPosition.setTransmitPeriod(2100);
}



// ______________________________________________
void MessengerXbeeNetworkSimu::execute()
{
    m_database.m_GrosbotPosition.Position_X = Application.m_asservissement.X_robot;
    m_database.m_GrosbotPosition.Position_Y = Application.m_asservissement.Y_robot;
    m_database.m_GrosbotPosition.Angle = Application.m_asservissement.angle_robot*180./3.14f;

    m_database.checkAndSendPeriodicMessages();
    m_database.checkNodeCommunication();
}


// ===================================================
//              MESSENGER OUTPUT
// ===================================================
// ______________________________________________
void MessengerXbeeNetworkSimu::encode(unsigned char *buff_data, unsigned short buff_size, unsigned short dest_address)
{
    (void)buff_data;
    (void)buff_size;
    (void)dest_address;
}

// ===================================================
//              MESSENGER RESSOURCES
// ===================================================
// ______________________________________________
long MessengerXbeeNetworkSimu::getTime()
{
    return QDateTime::currentDateTime().toMSecsSinceEpoch();
}

// ===================================================
//                  MESSENGER EVENTS
// ===================================================
// ______________________________________________
void MessengerXbeeNetworkSimu::newFrameReceived(tMessengerFrame *frame)
{
    (void)frame;
}

// ______________________________________________
void MessengerXbeeNetworkSimu::newMessageReceived(MessageBase *msg)
{
    (void)msg;
}

// ______________________________________________
void MessengerXbeeNetworkSimu::frameTransmited(tMessengerFrame *frame)
{
    (void)frame;
}

// ______________________________________________
void MessengerXbeeNetworkSimu::messageTransmited(MessageBase *msg)
{
    (void)msg;
}

// ______________________________________________
void MessengerXbeeNetworkSimu::dataUpdated(MessageBase *msg, char *name, char *val_str)
{
    QString data_name = QString("XbeeNetworkRX.") + msg->getName() + "." + QString(name);
    m_application->m_data_center->write(data_name, val_str);
}

// ______________________________________________
void MessengerXbeeNetworkSimu::dataChanged(MessageBase *msg, char *name, char *val_str)
{
    (void)msg;
    (void)name;
    (void)val_str;
}

// ______________________________________________
void MessengerXbeeNetworkSimu::dataSent(MessageBase *msg, char *name, char *val_str)
{
    QString data_name = QString("XbeeNetworkTX.") + msg->getName() + "." + QString(name);
    m_application->m_data_center->write(data_name, val_str);
}
