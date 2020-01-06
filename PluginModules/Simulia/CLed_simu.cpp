/*! \file CLed_simu.cpp
    \brief Classe de base pour la gestion d'une LED
*/
#include "CLed_simu.h"

#include "CApplication.h"
#include "CDataManager.h"

// ============================================================
//        Gestion d'une LED
// ============================================================
CLedSimu::CLedSimu(QString name)
    : CLedBase::CLedBase(),
      m_name(name)
{
   Init();
}


//___________________________________________________________________________
void CLedSimu::_write(bool val)
{
    m_state = val;
    if (m_application) {
        m_application->m_data_center->write(m_name, val);
    }
}

//___________________________________________________________________________
bool CLedSimu::_read()
{
    return m_state;
}

//___________________________________________________________________________
void CLedSimu::init(CApplication *application)
{
    m_application = application;
}

//___________________________________________________________________________
void CLedSimu::Init()
{
    _write(0);
}
