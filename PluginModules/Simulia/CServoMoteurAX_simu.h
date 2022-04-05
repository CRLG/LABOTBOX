#ifndef CServoMoteurAXSimu_H
#define CServoMoteurAXSimu_H

#include "servoaxbase.h"

class CApplication;

class CServoMoteurAXSimu : public ServoAXBase
{
public:
    CServoMoteurAXSimu();
    ~CServoMoteurAXSimu();

    // pure virtual methods to abstract hardware.
    // to be implemented on specific hardware.
    virtual tAxErr read(unsigned char *buff_data, unsigned char size);
    virtual tAxErr write(unsigned char *buff_data, unsigned char size);
    virtual tAxErr flushSerialInput();
    virtual tAxErr waitTransmitComplete();
    virtual tAxErr setTxEnable(bool state);
    virtual void delay_us(unsigned long delay);

    virtual bool isPresent(unsigned char id);
    virtual tAxErr setPosition(unsigned char id, unsigned short position);
    virtual unsigned short getPosition(unsigned char id, tAxErr *err_status=nullptr);
    virtual tAxErr setSpeed(unsigned char id, unsigned short speed);
    virtual tAxErr setPositionSpeed(unsigned char id, unsigned short position, unsigned short speed);
    virtual bool isMoving(unsigned char id, tAxErr *err_status=nullptr);
    virtual tAxErr setLimitPositionMin(unsigned char id, unsigned short pos);
    virtual tAxErr setLimitPositionMax(unsigned char id, unsigned short pos);

    void init(CApplication *application);
    void Init();


    int servo_id_to_index_present(unsigned char servo_id);

    void simu();

private :
    CApplication *m_application;
    typedef struct {
        unsigned short position;
        unsigned short target_position;
        unsigned short speed;
        unsigned short position_limit_min;
        unsigned short position_limit_max;
    }tServoAXSimu;
    static const int NOMBRE_MAX_SERVOS = 100;
    tServoAXSimu m_servos[NOMBRE_MAX_SERVOS];
};

#endif // CServoMoteurAXSimu_H
