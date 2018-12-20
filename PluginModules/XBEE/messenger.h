#ifndef _ROB_NETWORK_MESSENGER_
#define _ROB_NETWORK_MESSENGER_

#include "databasexbeenetwork2019.h"
#include "transportergeneric.h"
#include "xbeedriver.h"
#include "messengerinterfacebase.h"

class CApplication;
// ====================================================
//
// ====================================================
class RobNetworkMessenger : public MessengerInterfaceBase
{
public:
    RobNetworkMessenger();
    virtual ~RobNetworkMessenger();

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
    virtual void dataUpdated(char *name, char *val_str);
    // This method is called by messenger (message) to inform a data in a message changed value
    virtual void dataChanged(char *name, char *val_str);

    // This method is called by messenger to inform a buffer is ready to send on specific hardware or use by application
    virtual void encode(unsigned char *buff_data, unsigned short buff_size, unsigned short dest_address=0);


    void test();
    void initApp(CApplication *application);

    DatabaseXbeeNetwork2019 m_database;
    TransporterGeneric m_transporter;
    XbeeDriver m_xbee_driver;


    void receiveSerialDatas(QByteArray datas);
private :
    void initMessenger();

    CApplication *m_application;
};

#endif // _ROB_NETWORK_MESSENGER_
