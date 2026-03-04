/**
* Built on ##GENERATED_DATE_TIME##
*/

#include "##NOM_FIC##.h"
#include "CGlobale.h"

##NOM_CLASSE##::##NOM_CLASSE##()
{
	m_main_mission_type = true;
	m_max_score = 0;
}

const char* ##NOM_CLASSE##::getName()
{
	return "##NOM_CLASSE##";
}

const char* ##NOM_CLASSE##::stateToName(unsigned short state)
{
	switch(state)
	{
##LISTE_ETAT_NOM##
		case FIN_MISSION :	return "FIN_MISSION";
	}
	return "UNKNOWN_STATE";
}

// _____________________________________
void ##NOM_CLASSE##::step()
{
	switch (m_state)
	{
			
##FUNCTION_STEP##

	}
}
