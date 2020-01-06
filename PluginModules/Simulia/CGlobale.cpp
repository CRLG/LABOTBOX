#include "CGlobale.h"

CGlobaleSimule Application;

CGlobaleSimule::CGlobaleSimule()
    : m_led1("Led1"),
      m_led2("Led2"),
      m_led3("Led3"),
      m_led4("Led4"),
      m_leds(&m_led1, &m_led2, &m_led3, &m_led4)
{
}

CGlobaleSimule::~CGlobaleSimule()
{
}
