#include <stdio.h>

#include <QDebug>

#include "xbeedriver.h"
#include "messengerinterfacebase.h"
#include "CApplication.h"
#include "CRS232.h"
#include "CXBEE.h"


XbeeDriver::XbeeDriver()
{
}

XbeeDriver::~XbeeDriver()
{
}

void XbeeDriver::setApplication(CApplication *application)
{
    m_application = application;
}

// ________________________________________________________________
void XbeeDriver::readyBytes(unsigned char *buff_data, unsigned char buff_size, unsigned short source_id)
{
    m_application->m_XBEE->XbeeEvt_readyBytes(buff_data, buff_size, source_id);
}

// ________________________________________________________________
void XbeeDriver::write(unsigned char *buff_data, unsigned char buff_size)
{
    m_application->m_XBEE->XbeeEvt_write(buff_data, buff_size);
}

// ________________________________________________________________
void XbeeDriver::delay_us(unsigned long delay)
{
    //QThread::usleep(delay); // ne pas utiliser cette instruction dans ce cas car les données ne sont plus envoyées vers la RS232 (il faut rendre la main à la boucle principale)
    int delay_ms = (delay <= 1000) ? 1 : (delay/1000);
    QTime dieTime= QTime::currentTime().addMSecs(delay_ms);
     while (QTime::currentTime() < dieTime)
         QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}
