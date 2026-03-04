/**
 * Built on ##GENERATED_DATE_TIME##
 */

#ifndef ##INCLUDE_DEF##
#define ##INCLUDE_DEF##

#include "sm_statemachinebase.h"

class ##NOM_CLASSE## : public SM_StateMachineBase
{
public:
	##NOM_CLASSE##();
	void step();
	const char* getName();
	const char* stateToName(unsigned short state);

	typedef enum {
##LISTE_ETAT_DEF##
	FIN_MISSION
	}tState;
};

#endif // ##INCLUDE_DEF##
