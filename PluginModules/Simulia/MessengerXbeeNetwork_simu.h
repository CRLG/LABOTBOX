#ifndef _XBEE_NETWORK_MESSENGER_SIMU_H_
#define _XBEE_NETWORK_MESSENGER_SIMU_H_

#include "databasexbeenetwork2019.h"
#include "transportergeneric.h"
#include "messengerinterfacebase.h"

class CApplication;
// ====================================================
//
// ====================================================
class MessengerXbeeNetworkSimu : public MessengerInterfaceBase
{
public:
    MessengerXbeeNetworkSimu();
    virtual ~MessengerXbeeNetworkSimu();

    // =============================================
    //    Reimplement MessengerInterfaceBase virual
    virtual void encode(unsigned char *buff_data, unsigned short buff_size, unsigned short dest_address=0);

    // This method is called by messenger to get current time [msec]
    virtual long getTime();

    // Events
    // This method is called by messenger (transporter) to inform a valid frame was received
    virtual void newFrameReceived(tMessengerFrame *frame);
    // This method is called by messenger (transporter) to inform a frame was transmited
    virtual void frameTransmited(tMessengerFrame *frame);
    // This method is called by messenger (database) to inform a known message was received
    virtual void newMessageReceived(MessageBase *msg);
    // This method is called by messenger (database) to inform a message was transmited
    virtual void messageTransmited(MessageBase *msg);
    // This method is called by messenger (message) to inform a data in a message was updated
    virtual void dataUpdated(MessageBase *msg, char *name, char *val_str);
    // This method is called by messenger (message) to inform a data in a message was sent
    virtual void dataSent(MessageBase *msg, char *name, char *val_str);
    // This method is called by messenger (message) to inform a data in a message changed value
    virtual void dataChanged(MessageBase *msg, char *name, char *val_str);

    DatabaseXbeeNetwork2019 m_database;
    TransporterGeneric m_transporter;

    void initMessages();

    // Simulation
    void init(CApplication *application);
    CApplication *m_application;
    void execute();

};

#endif // _XBEE_NETWORK_MESSENGER_SIMU_H_
