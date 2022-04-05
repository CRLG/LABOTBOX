#include "CServoMoteurAX_simu.h"
#include "CApplication.h"
#include "CDataManager.h"


CServoMoteurAXSimu::CServoMoteurAXSimu()
    : m_application(nullptr)
{
    for (unsigned int i=0; i<NOMBRE_MAX_SERVOS; i++) {
        m_servos[i].position = 0;
        m_servos[i].target_position = 0;
        m_servos[i].speed = 0;
        m_servos[i].position_limit_min = 0;
        m_servos[i].position_limit_max = 0xFFFF;
    }
}

CServoMoteurAXSimu::~CServoMoteurAXSimu()
{
}

// ============================================================
//         IMPLEMENTATION DES METHODES VIRTUELLES PURES
//                  (en lien avec le hardware)
// ============================================================
// ______________________________________________________________
tAxErr CServoMoteurAXSimu::read(unsigned char *buff_data, unsigned char size)
{
    (void)buff_data;
    (void)size;
    return AX_OK;
}

// ______________________________________________________________
tAxErr CServoMoteurAXSimu::write(unsigned char *buff_data, unsigned char size)
{
    (void)buff_data;
    (void)size;
    return AX_OK;
}

// ______________________________________________________________
tAxErr CServoMoteurAXSimu::flushSerialInput()
{
    return AX_OK;
}

// ______________________________________________________________
tAxErr CServoMoteurAXSimu::waitTransmitComplete()
{
    return AX_OK;
}

// ______________________________________________________________
tAxErr CServoMoteurAXSimu::setTxEnable(bool state)
{
    (void)state;
    return AX_OK;
}

// ______________________________________________________________
void CServoMoteurAXSimu::delay_us(unsigned long delay)
{
    (void)delay;
}

// ============================================================
//                          SIMULATION
// ============================================================
void CServoMoteurAXSimu::init(CApplication *application)
{
    m_application = application;
}

void CServoMoteurAXSimu::Init()
{
    // RAZ de la position du servo dans le DataManager si elle existe
    for (unsigned int i=0; i<NOMBRE_MAX_SERVOS; i++) {
        m_servos[i].position = 0;
        m_servos[i].target_position = m_servos[i].position;
        QString data_name = "ServoAX_" + QString::number(i) + ".Position";
        if (m_application->m_data_center->isExist(data_name)) {
            m_application->m_data_center->write(data_name, m_servos[i].position);
        }
    }
}

// ____________________________________________________________
int CServoMoteurAXSimu::servo_id_to_index_present(unsigned char servo_id)
{
    return servo_id; // TODO : à vérifier
}

// ____________________________________________________________
// Simule un déplacement du servo vers la position attendue
void CServoMoteurAXSimu::simu()
{
    for (unsigned int i=0; i<NOMBRE_MAX_SERVOS; i++) {
        if (m_servos[i].position < m_servos[i].target_position) {
            m_servos[i].position++;
            QString data_name = "ServoAX_" + QString::number(i) + ".Position";
            m_application->m_data_center->write(data_name, m_servos[i].position);
        }
        else if (m_servos[i].position > m_servos[i].target_position) {
            m_servos[i].position--;
            QString data_name = "ServoAX_" + QString::number(i) + ".Position";
            m_application->m_data_center->write(data_name, m_servos[i].position);
        }
        // else : le servo est à la position attendue -> ne rien faire
    }
}

// ____________________________________________________________
bool CServoMoteurAXSimu::isPresent(unsigned char id)
{
    (void)id;
    return true;
}


// ____________________________________________________________
tAxErr CServoMoteurAXSimu::setPosition(unsigned char id, unsigned short position)
{
    m_servos[id].target_position = position;
    if (m_servos[id].target_position >= m_servos[id].position_limit_max) m_servos[id].target_position = m_servos[id].position_limit_max;
    if (m_servos[id].target_position <= m_servos[id].position_limit_min) m_servos[id].target_position = m_servos[id].position_limit_min;

    return AX_OK;
}

// ____________________________________________________________
tAxErr CServoMoteurAXSimu::setSpeed(unsigned char id, unsigned short speed)
{
    m_servos[id].speed = speed;
    QString data_name = "ServoAX_" + QString::number(id) + ".Speed";
    m_application->m_data_center->write(data_name, m_servos[id].speed);

    return AX_OK;
}

// ____________________________________________________________
tAxErr CServoMoteurAXSimu::setPositionSpeed(unsigned char id, unsigned short position, unsigned short speed)
{
    setPosition(id, position);
    setSpeed(id, speed);

    return AX_OK;
}

// ____________________________________________________________
unsigned short CServoMoteurAXSimu::getPosition(unsigned char id, tAxErr *err_status)
{
    if (err_status) *err_status = AX_OK;
    return m_servos[id].position;
}

// ____________________________________________________________
bool CServoMoteurAXSimu::isMoving(unsigned char id, tAxErr *err_status)
{
    if (err_status) *err_status = AX_OK;
    return m_servos[id].position != m_servos[id].target_position;
}

// ____________________________________________________________
tAxErr CServoMoteurAXSimu::setLimitPositionMin(unsigned char id, unsigned short pos)
{
    m_servos[id].position_limit_min = pos;
    return AX_OK;
}

// ____________________________________________________________
tAxErr CServoMoteurAXSimu::setLimitPositionMax(unsigned char id, unsigned short pos)
{
    m_servos[id].position_limit_max = pos;
    return AX_OK;

}
