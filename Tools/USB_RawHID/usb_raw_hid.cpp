#include <QDebug>
#include "usb_raw_hid.h"

CUSBRawHID::CUSBRawHID(QObject *parent)
    : QObject(parent),
      m_usb(nullptr),
      m_open(false),
      m_auto_reconnect_on_error(true)
{
    connect(&m_autoread_timer, SIGNAL(timeout()), this, SLOT(read()));
}

CUSBRawHID::~CUSBRawHID()
{
    close();
}

// ___________________________________________________
/*!
 * \brief CUSBRawHID::open
 * \param vid vendor ID to match (-1 for any)
 * \param pid Product ID to match (-1 for any)
 * \param serial Serial number to match (-1 for any)
 * \return true
 */
bool CUSBRawHID::open(int vid, int pid, int serial)
{
    struct usb_bus *bus;
    struct usb_device *dev;
    struct usb_interface *iface;
    struct usb_interface_descriptor *desc;
    struct usb_endpoint_descriptor *ep;
    usb_dev_handle *u;
    uint8_t buf[1024];
    int i, n, len, ep_in, ep_out, count=0, claimed;

    m_vid = vid;
    m_pid = pid;
    m_serial = serial;

    usb_init();
    usb_find_busses();
    usb_find_devices();

    for (bus = usb_get_busses(); bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            if (vid > 0 && dev->descriptor.idVendor != vid) continue;
            if (pid > 0 && dev->descriptor.idProduct != pid) continue;
            if (!dev->config) continue;
            if (dev->config->bNumInterfaces < 1) continue;

            iface = dev->config->interface;
            u = NULL;
            claimed = 0;
            for (i=0; i<dev->config->bNumInterfaces && iface; i++, iface++) {
                desc = iface->altsetting;
                if (!desc)                          continue;
                if (desc->bInterfaceClass != 3)     continue;
                if (desc->bInterfaceSubClass != 0)  continue;
                if (desc->bInterfaceProtocol != 0)  continue;
                ep = desc->endpoint;
                ep_in = ep_out = 0;
                for (n = 0; n < desc->bNumEndpoints; n++, ep++) {
                    if (ep->bEndpointAddress & 0x80) {
                        if (!ep_in) ep_in = ep->bEndpointAddress & 0x7F;
                    } else {
                        if (!ep_out) ep_out = ep->bEndpointAddress;
                    }
                }
                if (!ep_in) continue;
                if (!u) {
                    u = usb_open(dev);
                    if (!u) {
                        break;
                    }
                }

                char string[256];
                usb_get_string_simple(u,
                                      dev->descriptor.iSerialNumber,
                                      string, sizeof(string));
                int _snum = atoi(string);
                if ((serial > 0) && (_snum != serial)) {
                    usb_close(u);
                    continue;
                }
                if (usb_get_driver_np(u, i, (char *)buf, sizeof(buf)) >= 0) {
                    if (usb_detach_kernel_driver_np(u, i) < 0) {
                        continue;
                    }
                }
                if (usb_claim_interface(u, i) < 0) {
                    continue;
                }
                len = usb_control_msg(u, 0x81, 6, 0x2200, i, (char *)buf, sizeof(buf), 250);
                if (len < 2) {
                    usb_release_interface(u, i);
                    continue;
                }
                // Serial number ans other infos
                usb_get_string_simple(u,
                                      dev->descriptor.iManufacturer,
                                      string, sizeof(string));
                usb_get_string_simple(u,
                                      dev->descriptor.iProduct,
                                      string, sizeof(string));
                m_usb = u;
                m_iface = i;
                m_ep_in = ep_in;
                m_ep_out = ep_out;
                m_open = 1;
                claimed++;
                count++;
                emit connected(vid, pid, _snum);
                return true; // le device recherché est ouvert, pas la peine d'aller plus loin
            }
            if (u && !claimed) usb_close(u);
        } // for dev
    } // for bus
    return (false);
}

// ___________________________________________________
/*!
 * \brief Vérifie si la connection est ouverte
 * \return
 */

bool CUSBRawHID::isOpen()
{
    return (m_usb && m_open);
}

// ___________________________________________________
/*!
 * \brief Autorise le mécanisme de reconnexion automatique
 * \param enable : true / false
 */
void CUSBRawHID::enableAutoreconnect(bool enable)
{
    m_auto_reconnect_on_error = enable;
}

// ___________________________________________________
bool CUSBRawHID::close()
{
    if (!isOpen()) return false;

    usb_release_interface(m_usb, m_iface);
    usb_close(m_usb);
    m_usb = NULL;
    m_open = false;
    emit disconnected();

    return true;
}

// ___________________________________________________
/*!
 * \brief Récupère les informations sous forme de chaine de caractère à afficher
 * \return
 */
QString CUSBRawHID::getInfo()
{
    if (!m_usb) return "";

    char string[256];
    QString str;
    struct usb_device *dev;
    dev = usb_device(m_usb);

    str += "Vendor ID = 0x" + QString::number(dev->descriptor.idVendor, 16) + "\n";
    str += "Product ID = 0x" + QString::number(dev->descriptor.idProduct, 16)  + "\n";

    usb_get_string_simple(m_usb, dev->descriptor.iSerialNumber, string, sizeof(string));
    str += QString ("Serial Number : %1\n").arg(string);

    usb_get_string_simple(m_usb, dev->descriptor.iManufacturer, string, sizeof(string));
    str += QString("Manufacturer: %1\n").arg(string);

    usb_get_string_simple(m_usb, dev->descriptor.iProduct, string, sizeof(string));
    str += QString("Product: %1\n").arg(string);

    return str;
}

// ___________________________________________________
/*!
*   receive a packet from device
* \param num = device to receive from (zero based)
* \param buf = buffer to receive packet
* \param len = buffer's size
* \param timeout = time to wait, in milliseconds
* \return number of bytes received, or -1 on error
*/
int CUSBRawHID::read(char *buf, int len, int timeout)
{
    if (!isOpen()) {
        if (m_auto_reconnect_on_error) {
            if (!open(m_vid, m_pid, m_serial)) return -1;
        }
        else return -1;
    }

    int r = usb_interrupt_read(m_usb, m_ep_in, buf, len, timeout);
    if (r < 0) {
        if (isOpen()) emit disconnected();
        m_open = false;
    }

    if (r >= 0) return r;
    if (r == -110) return 0;  // timeout
    return -1;
}

// ___________________________________________________
int CUSBRawHID::read()
{
    char buff[64];
    int n;
    n = read(buff, sizeof(buff), 0);
    if (n>0) {
        QByteArray array(QByteArray::fromRawData(buff, n));
        emit received(array, n);
    }
    return n;
}

// ___________________________________________________
/*!
* send a packet to device
* \param num = device to transmit to (zero based)
* \param buf = buffer containing packet to send
* \param len = number of bytes to transmit
* \param timeout = time to wait, in milliseconds
* \return number of bytes sent, or -1 on error
*/
int CUSBRawHID::write(char *buf, int len, int timeout)
{
    if (!isOpen()) {
        if (m_auto_reconnect_on_error) {
            if (!open(m_vid, m_pid, m_serial)) return -1;
        }
        else return -1;
    }

    int r;
    if (m_ep_out) {
        r = usb_interrupt_write(m_usb, m_ep_out, buf, len, timeout);
    } else {
        r = usb_control_msg(m_usb, 0x21, 9, 0, m_iface, buf, len, timeout);
    }
    if (r < 0) {
        if (isOpen()) emit disconnected();
        m_open = false;
    }
    return r;
}

// =============================================================
//                  GESTION DE LA LECTURE PERIODIQUE
// =============================================================
// ___________________________________________________
/*!
 * \brief Commence la lecture périodique automatique
 * \param period = période de lecture [msec]
 */
void CUSBRawHID::start_auto_read(int period)
{
    m_autoread_timer.start(period);
}

// ___________________________________________________
/*!
 * \brief Arrêt de la lecture périodique
 */
void CUSBRawHID::stop_auto_read()
{
    m_autoread_timer.stop();
}
