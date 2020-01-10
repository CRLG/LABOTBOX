#ifndef POWERELECTROBOTSIMU_H
#define POWERELECTROBOTSIMU_H

#include "dspicpowerelectrobotbase.h"
#include <QString>
#include <QVariant>

class CApplication;

class PowerElectrobotSimu : public dsPicPowerElectrobotBase
{
public:
    PowerElectrobotSimu();

    /*virtual*/void writeI2C(unsigned char *buff, unsigned short size);
    /*virtual*/void readI2C(unsigned char *dest_buff, unsigned short size);

    /*virtual*/ void setOutput(tSwitchOutput output, bool state);
    /*virtual*/ void setOutputPort(unsigned char val);

    void init(CApplication *application);
    void simuSetBatteryVoltage(float val);
    void simuSetGlobalCurrent(float val);
    void simuSetCurrentOut1(float val);
    void simuSetCurrentOut2(float val);

private :
    CApplication *m_application;
    void refreshDataManager(tSwitchOutput output, bool state);
    void refreshDataManager(QString data_name, QVariant val);
};

#endif // POWERELECTROBOTSIMU_H
