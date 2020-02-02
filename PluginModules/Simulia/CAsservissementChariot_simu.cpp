#include <math.h>
#include "CApplication.h"
#include "CDataManager.h"
#include "CAsservissementChariot_simu.h"

#define POSITION_BUTEE_MECA_HAUTE (800)
#define POSITION_BUTEE_MECA_BASSE (25)
static const QString DATANAME_POSITION_CHARIOT  = "AsservissementChariot.PositionChariot";
static const QString DATANAME_COMMANDE_MOTEUR   = "AsservissementChariot.CommandeMoteur";
static const QString DATANAME_CONSIGNE_POSITION = "AsservissementChariot.ConsignePosition";

CAsservissementChariotSimu::CAsservissementChariotSimu()
    : CAsservissementChariotBase(),
      m_application(nullptr)
{
    Init();
}

// ___________________________________________________
void CAsservissementChariotSimu::Init()
{
    CAsservissementChariotBase::Init();
    setConsigne(0);
    resetPositionCodeur(0);
    commandeMoteur(0);
    // Force la création des data dans le DataManager
    updateDataManager(DATANAME_CONSIGNE_POSITION, position_consigne);
    updateDataManager(DATANAME_POSITION_CHARIOT, m_simu_position_codeur_chariot);
    updateDataManager(DATANAME_COMMANDE_MOTEUR, m_simu_commande_moteur_memo);
}


void CAsservissementChariotSimu::setConsigne(int pos)
{
    CAsservissementChariotBase::setConsigne(pos);
    updateDataManager(DATANAME_CONSIGNE_POSITION, position_consigne);
}

// ============================================================
//                          SIMULATION
// ============================================================
void CAsservissementChariotSimu::init(CApplication *application)
{
    m_application = application;
    Init();
}

// ___________________________________________________
// Simule des déplacements codeurs en fonction du signe de la consigne moteur
// TODO : ce modèle ne prend pas en compte la valeur de la commande moteur, mais juste le signe pour
//  la vitesse de déplacemement des codeurs
void CAsservissementChariotSimu::simu()
{
    // Le moteur ne bougera que si la commande moteur dépasse la zone morte
    // et que le moteur n'est pas en butée méca

    // En montée
    if ((m_simu_commande_moteur_memo > 0) && (m_simu_commande_moteur_memo >= compensation_zone_morte_up_C) && (m_simu_position_codeur_chariot < POSITION_BUTEE_MECA_HAUTE)) {
        resetPositionCodeur(m_simu_position_codeur_chariot + 1);
    }
    // En descente
    else if ((m_simu_commande_moteur_memo < 0) && (fabs(m_simu_commande_moteur_memo) >= compensation_zone_morte_dw_C) && (m_simu_position_codeur_chariot > POSITION_BUTEE_MECA_BASSE)) {
        resetPositionCodeur(m_simu_position_codeur_chariot - 1);
    }
    // else : ne rien faire, le chariot est en butée mécanique ou la commande moteur ne permettant pas de vaincre la zone morte du moteur
}

// ___________________________________________________
bool CAsservissementChariotSimu::isButeeBasse()
{
    return (codeur_position_chariot <= POSITION_BUTEE_MECA_BASSE);
}

// ___________________________________________________
bool CAsservissementChariotSimu::isButeeHaute()
{
    return (codeur_position_chariot >= POSITION_BUTEE_MECA_HAUTE);
}

// ___________________________________________________
int CAsservissementChariotSimu::getPositionCodeur()
{
    return m_simu_position_codeur_chariot;
}

// ___________________________________________________
void CAsservissementChariotSimu::resetPositionCodeur(int pos)
{
    if (m_simu_position_codeur_chariot != pos) {
        m_simu_position_codeur_chariot = pos;
        updateDataManager(DATANAME_POSITION_CHARIOT, m_simu_position_codeur_chariot);
    }
}


// ___________________________________________________
void CAsservissementChariotSimu::commandeMoteur(float pourcent)
{
    if (m_simu_commande_moteur_memo != pourcent) {
        m_simu_commande_moteur_memo = pourcent;
        updateDataManager(DATANAME_COMMANDE_MOTEUR, m_simu_commande_moteur_memo);
    }
}

// ___________________________________________________
void CAsservissementChariotSimu::updateDataManager(QString dataname, QVariant val)
{
    if (!m_application) return;
    m_application->m_data_center->write(dataname, val);
}
