#include <qdebug.h>
#include "sm_debugqdebug.h"
#include "sm_statemachinebase.h"

SM_DebugQDebug::SM_DebugQDebug()
{

}

// ________________________________________________
void SM_DebugQDebug::SM_Start(SM_StateMachineBase* origin, float match_time)
{
    if (!m_active_start) return;
    qDebug() << match_time << ": STARTED " << origin->getName();
}

// ________________________________________________
void SM_DebugQDebug::SM_Stop(SM_StateMachineBase* origin, float match_time)
{
    if (!m_active_stop) return;
    qDebug() << match_time << ": STOPPED " << origin->getName();
}

// ________________________________________________
void SM_DebugQDebug::SM_EntryState(SM_StateMachineBase* origin, unsigned short state, float match_time)
{
    if (!m_active_on_entry) return;
    qDebug() << "   >" << match_time << ": ENTRY STATE " << origin->getName() << "::" << origin->stateToName(state);
}

// ________________________________________________
void SM_DebugQDebug::SM_ExitState(SM_StateMachineBase* origin, unsigned short state, float match_time)
{
    if (!m_active_on_exit) return;
    qDebug() << "   <" << match_time << ": EXIT STATE " << origin->getName() << "::" << origin->stateToName(state);
}

// ________________________________________________
void SM_DebugQDebug::SM_InterruptedByEvitement(SM_StateMachineBase* origin, unsigned short state, float match_time)
{
    if (!m_active_interrupt_evitement) return;
    qDebug() << "   #" << match_time << ": INTERRUPT EVITEMENT" << origin->getName() << "::" << origin->stateToName(state);
}
