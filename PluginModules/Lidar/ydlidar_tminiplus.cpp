#include <QDebug>
#include <QDateTime>
#include <QTime>
#include "ydlidar_tminiplus.h"

YDLIDAR_TminiPlus::YDLIDAR_TminiPlus(QObject *parent)
    : LidarBase(parent),
      m_current_index(0)
{
    m_widget = new QWidget();
    m_ihm.setupUi(m_widget);

    QObject::connect(m_ihm.start,   &QPushButton::clicked, [&]() { this->start_measures();});
    QObject::connect(m_ihm.stop,    &QPushButton::clicked, [&]() { this->stop_measures();});
    QObject::connect(m_ihm.scan_M1, &QPushButton::clicked, [&]() { this->scan_M1();});
    QObject::connect(m_ihm.scan_P1, &QPushButton::clicked, [&]() { this->scan_P1();});
}

YDLIDAR_TminiPlus::~YDLIDAR_TminiPlus()
{
    closeSerialPort();
    if (m_widget) m_widget->deleteLater();
}


// ========================================================================
//                  CLASSE LidarBase
// ========================================================================


// _________________________________________________________
QWidget *YDLIDAR_TminiPlus::get_widget()
{
    return m_widget;
}

// _________________________________________________________
QString YDLIDAR_TminiPlus::get_name()
{
    return "YDLIDAR_TminiPlus";
}

// _________________________________________________________
void YDLIDAR_TminiPlus::start()
{
    bool status = openSerialPort("/dev/ttyUSB0");
    if (status) {
        qDebug() << "Serial port opened";
        connect(&m_serial, SIGNAL(readyRead()), this, SLOT(serial_data_received()));
        emit connected();
    }
    else  {
        qDebug() << "Serial port error";
    }
    m_ihm.start->setEnabled(status);
    m_ihm.stop->setEnabled(status);
}

// _________________________________________________________
void YDLIDAR_TminiPlus::read_settings(CEEPROM *eeprom, QString section_name)
{

}

// _________________________________________________________
void YDLIDAR_TminiPlus::save_settings(CEEPROM *eeprom, QString section_name)
{

}

// ========================================================================
//                 Classe YDLIDAR_TminiPlusBase
// ========================================================================
// ____________________________________________________________
/*!
 * \brief YDLIDAR_TminiPlus::new_packet
 */
void YDLIDAR_TminiPlus::new_packet()
{
    if (isFirstPacket(&m_packet)) {
        m_current_index = 0;
        m_current_lidar_data.m_start_angle = firstAngle(&m_packet);
    }

    for (unsigned int i=0; i<m_packet.packet_len; i++) {
        unsigned int dist = dataindex2Distance(&m_packet, i);
        m_current_lidar_data.m_dist_measures[m_current_index] = dist;
        if (m_current_index < m_current_lidar_data.MAX_MEASURES_COUNT) m_current_index++;
    }

    if (isLastPacketOfCycle(&m_packet)) {
        m_current_lidar_data.m_angle_step_resolution = 360./m_data_count_in_cycle;
        m_current_lidar_data.m_measures_count = m_data_count_in_cycle;
        if (m_current_lidar_data.m_measures_count <=  m_current_lidar_data.MAX_MEASURES_COUNT ) {
            emit new_data(m_current_lidar_data);
        }
        // else : il y a un problème dans le transfert, le cycle doit être ignoré car corrompu
    }

#if 0
    if (0) {
        //qDebug() << "-------";
        //qDebug() << "First Angle" << firstAngle(&m_packet);
        //qDebug() << "Last Angle" << lastAngle(&m_packet);
        //qDebug() << "Angle count" << m_packet.packet_len;
        //if (/*m_packet.packet_len>1*/1) qDebug() << "Resolution d'angle" << diffAngles(firstAngle(&m_packet), lastAngle(&m_packet)) / (m_packet.packet_len-1);
        qDebug() << diffAngles(firstAngle(&m_packet), lastAngle(&m_packet)) / (m_packet.packet_len-1);
        CLidarData data;
        data.m_start_angle = firstAngle(&m_packet)-90;
        data.m_measures_count = m_packet.packet_len;
        data.m_angle_step_resolution = diffAngles(firstAngle(&m_packet), lastAngle(&m_packet)) / (m_packet.packet_len-1);
        for (unsigned int i=0; i<m_packet.packet_len; i++) {
            float angle = dataindex2Angle(&m_packet, i);
            unsigned int dist = dataindex2Distance(&m_packet, i);
            bool valid = isDistanceValid(&m_packet, i);
            //if ( (dist < 100) && (dist!=0)) qDebug() << QString("Angle=%1 / Distance=%2 / Valid=%3").arg(angle).arg(dist).arg(valid);
            //if ( (angle>0) && (angle<15)) {
                qDebug() << QString("Angle=%1 / Distance=%2 / Valid=%3").arg(angle).arg(dist).arg(valid);
                data.m_dist_measures[i] = dist;
            //}
            if(0) {
                unsigned int __index=3*i;
                qDebug() << m_packet.data[__index] << m_packet.data[__index+1] << m_packet.data[__index+2];
            }
        } // for
        emit new_data(data);
    }
#endif
}

// ____________________________________________________________
/*!
 * \brief YDLIDAR_TminiPlus::packet_error
 */
void YDLIDAR_TminiPlus::packet_error()
{

}

// ____________________________________________________________
/*!
 * \brief YDLIDAR_TminiPlus::new_frame
 */
void YDLIDAR_TminiPlus::new_cycle()
{
    if (0) {
        static qint64 last_time;
        qint64 _time = QDateTime::currentMSecsSinceEpoch();
        qDebug() << "_____________________________________________";
        qint64 _diff = _time - last_time;
        qDebug() << "T=" << _diff << "/ F=" << 1000./_diff;
        last_time = _time;
        qDebug() << "Nbre data" << m_data_count_in_cycle << "Resol=" << 360./m_data_count_in_cycle;
    }
}


// ========================================================================
//                Serial Port
// ========================================================================

// ___________________________________________________
/*!
 * \brief YDLIDAR_TminiPlus::openSerialPort
 * \param portname
 * \return
 */
bool YDLIDAR_TminiPlus::openSerialPort(QString portname)
{
    m_serial.setPortName(portname);
    QIODevice::OpenMode mode = QIODevice::ReadWrite;
    if (!m_serial.open(mode)) return false;

    bool error = true;
    error &= m_serial.setBaudRate(230400);
    error &= m_serial.setDataBits(QSerialPort::Data8);
    error &= m_serial.setFlowControl(QSerialPort::NoFlowControl);
    error &= m_serial.setParity(QSerialPort::NoParity);
    error &= m_serial.setStopBits(QSerialPort::OneStop);

    return(error);
}

// ______________________________________
void YDLIDAR_TminiPlus::closeSerialPort()
{
    if (m_serial.isOpen()) m_serial.close();
}


// ______________________________________
void YDLIDAR_TminiPlus::serial_data_received()
{
    QByteArray array;
    array = m_serial.readAll();

    for (int i=0; i<array.size(); i++) {
        reconstitution(array.at(i));
    }
}

// ========================================================================
//    Implémentation des méthodes virtuelles pour driver YdLidar
//      en lien avec le hardware
// ========================================================================

bool YDLIDAR_TminiPlus::write_serial(const char buff[], unsigned long len)
{
    unsigned long byte_written = m_serial.write(buff, len);
    return (len == byte_written);
}

bool YDLIDAR_TminiPlus::read_serial(const char buff[], unsigned long len)
{
    unsigned long read_bytes_count = 0;
    if (m_serial.waitForReadyRead(1000)) {
        read_bytes_count = m_serial.read((char *)buff, len);
    }
    return (read_bytes_count == len);
}

