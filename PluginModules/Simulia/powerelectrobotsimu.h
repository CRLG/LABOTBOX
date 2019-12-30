#ifndef POWERELECTROBOTSIMU_H
#define POWERELECTROBOTSIMU_H

#include "dspicpowerelectrobotbase.h"

class PowerElectrobotSimu : public dsPicPowerElectrobotBase
{
public:
    PowerElectrobotSimu();

    void writeI2C(unsigned char *buff, unsigned short size);
    void readI2C(unsigned char *dest_buff, unsigned short size);

    void simuSetBatteryVoltage(float val);
    void simuSetGlobalCurrent(float val);
    void simuSetCurrentOut1(float val);
    void simuSetCurrentOut2(float val);
};

#endif // POWERELECTROBOTSIMU_H
