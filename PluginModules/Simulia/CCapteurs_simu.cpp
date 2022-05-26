/*! \file CCapteursSimu.cpp
	\brief Classe qui contient toute l'application
*/
#include "CApplication.h"
#include "CDataManager.h"

#include "CCapteurs_simu.h"


//___________________________________________________________________________
CCapteursSimu::CCapteursSimu()
{
}

// ____________________________________________
void CCapteursSimu::Init(void)
{
    if (!m_application) return;
    m_application->m_data_center->write("Capteurs.Tirette", false);
    m_application->m_data_center->write("Capteurs.RecalageAVG", false);
    m_application->m_data_center->write("Capteurs.RecalageAVD", false);
    m_application->m_data_center->write("Capteurs.RecalageARG", false);
    m_application->m_data_center->write("Capteurs.RecalageARD", false);
}

// ____________________________________________
void CCapteursSimu::Traitement(void)
{
}

// ____________________________________________
// Liste tous les capteurs possibles (pour tous les robots)
bool CCapteursSimu::getTirette()
{
    if (!m_application) return false;
    return m_application->m_data_center->getData("Capteurs.Tirette")->read().toBool();
}

// ____________________________________________
bool CCapteursSimu::getContactRecalageAVG()
{
    if (!m_application) return false;
    return m_application->m_data_center->getData("Capteurs.RecalageAVG")->read().toBool();
}
bool CCapteursSimu::getContactRecalageAVD()
{
    if (!m_application) return false;
    return m_application->m_data_center->getData("Capteurs.RecalageAVD")->read().toBool();
}
bool CCapteursSimu::getContactRecalageARG()
{
    if (!m_application) return false;
    return m_application->m_data_center->getData("Capteurs.RecalageARG")->read().toBool();
}
bool CCapteursSimu::getContactRecalageARD()
{
    if (!m_application) return false;
    return m_application->m_data_center->getData("Capteurs.RecalageARD")->read().toBool();
}

// ____________________________________________
bool CCapteursSimu::getAscenseurButeeHaute()
{
    return false;
}
bool CCapteursSimu::getAscenseurButeeBasse()
{
    return false;
}

// ____________________________________________
float CCapteursSimu::getCapteurPressionKmar()
{
    return 1.25; // arbitrairement
}

// ============================================================
//                          SIMULATION
// ============================================================
void CCapteursSimu::init(CApplication *application)
{
    m_application = application;
    Init();
}



