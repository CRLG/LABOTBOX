#ifndef SM_DEBUGQDEBUG_H
#define SM_DEBUGQDEBUG_H

#include "sm_debuginterface.h"

class SM_DebugQDebug : public SM_DebugInterface
{
public:
    SM_DebugQDebug();

    // Ré-implémente les méthodes virtuelles
    /*virtual*/ void SM_Start(SM_StateMachineBase* origin, float match_time);
    /*virtual*/ void SM_Stop(SM_StateMachineBase* origin, float match_time);
    /*virtual*/ void SM_EntryState(SM_StateMachineBase* origin, unsigned short state, float match_time);
    /*virtual*/ void SM_ExitState(SM_StateMachineBase* origin, unsigned short state, float match_time);
    /*virtual*/ void SM_InterruptedByEvitement(SM_StateMachineBase* origin, unsigned short state, float match_time);
};

#endif // SM_DEBUGQDEBUG_H
