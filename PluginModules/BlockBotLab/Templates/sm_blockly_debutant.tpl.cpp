/**
 * Fichier généré le : ##GENERATED_DATE_TIME##
 */

#include "sm_blockly_debutant.h"
#include "CGlobale.h"

SM_BlocklyDebutant::SM_BlocklyDebutant()
{
    m_main_mission_type = true;
    m_max_score = 0;
}

const char* SM_BlocklyDebutant::getName()
{
    return "SM_BlocklyDebutant";
}

const char* SM_BlocklyDebutant::stateToName(unsigned short state)
{
    switch(state)
    {
    //##CASE_STATENAME_TO_STRING##
    case FIN_MISSION :	return "FIN_MISSION";
    }
    return "UNKNOWN_STATE";
}

##FUNCTION_STEP##
