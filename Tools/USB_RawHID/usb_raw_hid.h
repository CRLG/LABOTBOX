#ifndef USB_RAW_HID_H
#define USB_RAW_HID_H

#include <QObject>
#include <QThread>

#include <usb.h>

class CUSBRawHID : public QThread
{
    Q_OBJECT
public:
    explicit CUSBRawHID(QObject *parent = Q_NULLPTR);
    ~CUSBRawHID();

    bool open(int vid, int pid, int serial=-1);
    bool close();

    bool isOpen();
    void enableAutoreconnect(bool enable);
    QString getInfo();

    int read(char *buf, int len, int timeout=0);
    int write(char *buf, int len, int timeout=50);

    /*virtual*/ void run(void); // Hériatage de QThread

private :
    int m_vid;
    int m_pid;
    int m_serial;

    usb_dev_handle *m_usb;
    int m_open;
    int m_iface;
    int m_ep_in;
    int m_ep_out;

    bool m_auto_reconnect_on_error;

    bool m_enable_read;

signals:
    void connected(int vid, int pid, int serial);
    void disconnected();
    void received(const QByteArray& array);

public slots:
    void start_read();
    void stop_read();
};

#endif // USB_RAW_HID_H
