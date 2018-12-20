#ifndef _XBEE_NETWORK_MESSENGER_2019_H_
#define _XBEE_NETWORK_MESSENGER_2019_H_

#include "databasexbeenetwork2019.h"
#include "transportergeneric.h"
#include "messengerinterfacebase.h"

// ====================================================
//
// ====================================================
class CApplication;
class MessengerXbeeNetwork : public MessengerInterfaceBase
{
public:
    MessengerXbeeNetwork();
    virtual ~MessengerXbeeNetwork();

    void initApp(CApplication *application);

    void test_RX();
    void test_TX();

    // =============================================
    //    Reimplement MessengerInterfaceBase virual
    virtual void encode(unsigned char *buff_data, unsigned short buff_size, unsigned short dest_address=0);

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
    virtual void dataUpdated(char *name, char *val_str);
    // This method is called by messenger (message) to inform a data in a message changed value
    virtual void dataChanged(char *name, char *val_str);

    DatabaseXbeeNetwork2019 m_database;
    TransporterGeneric m_transporter;

private :
    CApplication *m_application;
};

#endif // _XBEE_NETWORK_MESSENGER_2019_H_
