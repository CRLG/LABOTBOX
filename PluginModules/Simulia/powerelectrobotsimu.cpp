#include "powerelectrobotsimu.h"
#include "CApplication.h"
#include "CDataManager.h"

PowerElectrobotSimu::PowerElectrobotSimu()
    :m_application(nullptr)
{
}

//___________________________________________________________________________
void PowerElectrobotSimu::init(CApplication *application)
{
    m_application = application;
}

//___________________________________________________________________________
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

//___________________________________________________________________________
void PowerElectrobotSimu::setOutput(tSwitchOutput output, bool state)
{
    dsPicPowerElectrobotBase::setOutput(output, state);
    refreshDataManager(output, state);
}

void PowerElectrobotSimu::setOutputPort(unsigned char val)
{
    dsPicPowerElectrobotBase::setOutputPort(val);

    for (unsigned int i=OUTPUT_STOR1; i<=OUTPUT_STOR8; i++) {
        refreshDataManager((tSwitchOutput)i, (val>>i)&0x01);
    }
}


//___________________________________________________________________________
void PowerElectrobotSimu::refreshDataManager(tSwitchOutput output, bool state)
{
    QString data_name = "PowerElectrobot.OUTPUT_STOR" + QString::number(output+1);
    refreshDataManager(data_name, state);
}

void PowerElectrobotSimu::refreshDataManager(QString data_name, QVariant val)
{
    if (m_application) {
        m_application->m_data_center->write(data_name, val);
    }
}


//___________________________________________________________________________
void PowerElectrobotSimu::simuSetBatteryVoltage(float val)
{
    m_battery_voltage = val;
    m_raw_battery_voltage = val * 1000.f;
    refreshDataManager("PowerElectrobot.BatteryVoltage", m_battery_voltage);
}

void PowerElectrobotSimu::simuSetGlobalCurrent(float val)
{
    m_global_current = val;
    m_raw_global_current = val * 1000.f;
    refreshDataManager("PowerElectrobot.GlobalCurrent", m_global_current);
}

void PowerElectrobotSimu::simuSetCurrentOut1(float val)
{
    m_current_out1 = val;
    m_raw_current_out1 = val * 1000.f;
    refreshDataManager("PowerElectrobot.CurrentOut1", m_current_out1);
}

void PowerElectrobotSimu::simuSetCurrentOut2(float val)
{
    m_current_out2 = val;
    m_raw_current_out2 = val * 1000.f;
    refreshDataManager("PowerElectrobot.CurrentOut2", m_current_out2);
}
