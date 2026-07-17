/*! \file CRobotLogic_entry.cpp
    \brief Points d'entree ABI C du plugin de logique robot (paire symetrique).

    Resolus par QLibrary::resolve() cote shell a chaque chargement. C'est le SEUL
    point de contact symbolique entre le shell et le plugin (cf. spike / CLAUDE.md).
*/
#include "CRobotLogic.h"

extern "C" IRobotLogic* createRobotLogic()
{
    return new CRobotLogic();
}

extern "C" void destroyRobotLogic(IRobotLogic* p)
{
    delete p;
}
