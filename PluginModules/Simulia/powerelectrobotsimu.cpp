#include "powerelectrobotsimu.h"

PowerElectrobotSimu::PowerElectrobotSimu()
{
}

void PowerElectrobotSimu::writeI2C(unsigned char *buff, unsigned short size)
{
    (void)buff;
    (void)size;
}

void PowerElectrobotSimu::readI2C(unsigned char *dest_buff, unsigned short size)
{
    (void)dest_buff;
    (void)size;
}

void PowerElectrobotSimu::simuSetBatteryVoltage(float val)
{
    m_battery_voltage = val;
    m_raw_battery_voltage = val * 1000.f;
}
void PowerElectrobotSimu::simuSetGlobalCurrent(float val)
{
    m_global_current = val;
    m_raw_global_current = val * 1000.f;
}
void PowerElectrobotSimu::simuSetCurrentOut1(float val)
{
    m_current_out1 = val;
    m_raw_current_out1 = val * 1000.f;
}
void PowerElectrobotSimu::simuSetCurrentOut2(float val)
{
    m_current_out2 = val;
    m_raw_current_out2 = val * 1000.f;
}
