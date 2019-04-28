#ifndef _XBEE_MESSENGER_
#define _XBEE_MESSENGER_

#include <QTime>
#include "databasexbeenetwork2019.h"
#include "transportergeneric.h"
#include "messengerinterfacebase.h"

class CApplication;
// ====================================================
//
// ====================================================
class XbeeMessenger : public MessengerInterfaceBase
{
public:
    XbeeMessenger();
    virtual ~XbeeMessenger();

    // ==========================================
    //   METHODS FROM MessengerInterfaceBase
    // ==========================================
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
    // This method is called by messenger (message) to inform a data in a message changed value
    virtual void dataChanged(MessageBase *msg, char *name, char *val_str);
    // This method is called by messenger (node) to inform the communication status changed (lost communication or communication OK)
    virtual void nodeCommunicationStatusChanged(NodeBase *node);

    // This method is called by messenger to inform a buffer is ready to send on specific hardware or use by application
    virtual void encode(unsigned char *buff_data, unsigned short buff_size, unsigned short dest_address=0);

    // This method is called by messenger to get current time [msec]
    virtual long getTime();


    void test();
    void initApp(CApplication *application);

    DatabaseXbeeNetwork2019 m_database;
    TransporterGeneric m_transporter;

private :
    void initMessenger();
    QTime m_time;

    CApplication *m_application;
};

#endif // _XBEE_MESSENGER_
