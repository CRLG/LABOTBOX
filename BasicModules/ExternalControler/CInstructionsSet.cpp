/*! \file CInstructionsSet.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */

#include <QDebug>
#include <QByteArray>
#include <QMap>

#include "CApplication.h"
#include "Instructions.h"
#include "CInstructionsSet.h"

/*! \addtogroup ExternalControler
   * 
   *  @{
   */

// _____________________________________________________________________
/*!
* Constructeur
*
*/
CInstructionsSet::CInstructionsSet(CApplication *application)
    :m_application(application)
{
    if (application) {
        init(application);
    }
}

// _____________________________________________________________________
/*!
* Destructeur
*
*/
CInstructionsSet::~CInstructionsSet()
{
    clear();
}


// _____________________________________________________________________
void CInstructionsSet::init(CApplication *application)
{
    CInstruction *instruction = new CInstruction_Help(application);
    if (instruction) {
        m_instruction_set.insert(instruction->instructionName(), instruction);
        static_cast<CInstruction_Help*>(instruction)->setInstructionsSet(&m_instruction_set);  // cette instruction est particulière car elle nécessite de connaitre tout le jeu d'instruction
    }

    instruction = new CInstruction_HelpInstruction(application);
    if (instruction) {
        m_instruction_set.insert(instruction->instructionName(), instruction);
        static_cast<CInstruction_HelpInstruction*>(instruction)->setInstructionsSet(&m_instruction_set);  // cette instruction est particulière car elle nécessite de connaitre tout le jeu d'instruction
    }

    instruction = new CInstruction_setData(application);
    if (instruction) m_instruction_set.insert(instruction->instructionName(), instruction);

    instruction = new CInstruction_getData(application);
    if (instruction) m_instruction_set.insert(instruction->instructionName(), instruction);

    instruction = new CInstruction_getDatas(application);
    if (instruction) m_instruction_set.insert(instruction->instructionName(), instruction);

    instruction = new CInstruction_createData(application);
    if (instruction) m_instruction_set.insert(instruction->instructionName(), instruction);
}

// _____________________________________________________________________
void CInstructionsSet::clear()
{
    for (tListInstruction::iterator it=m_instruction_set.begin(); it!=m_instruction_set.end(); it++)
    {
        if (it.value()) delete it.value();
    }
    m_instruction_set.clear();
}

// _____________________________________________________________________
// Une commande = 1 ou plusieurs instructions
// Si plusieurs intructions : séparateur '\n'
QByteArray CInstructionsSet::execute(QByteArray command)
{
    QByteArray ret;
    QList<QByteArray> instructions = command.split('\n');
    foreach (QByteArray instruction, instructions) {
        for (tListInstruction::iterator it=m_instruction_set.begin(); it!=m_instruction_set.end(); it++)
        {
            if (it.value()->isValid(instruction)) {
                if (ret.size() != 0) ret+= '\n'; // séparateur entre les différentes réponses de chaque instruction que compose la commande
                ret += it.value()->execute(instruction);
            }
        }
    }
    if (ret.size() == 0) {
        ret = QString::number(CInstruction::INVALID_INSTRUCTION).toUtf8() + SEPARATOR_BETWEEN_RESPONSE_ARGUMENTS + command;
    }
    return ret;
}
