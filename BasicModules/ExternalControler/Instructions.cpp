/*! \file CInstruction.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */

#include <QDebug>

#include "CApplication.h"
#include "Instructions.h"
#include "CDataManager.h"



/*! \addtogroup ExternalControler
   * 
   *  @{
   */

// _____________________________________________________________________
CInstruction::CInstruction(CApplication *application):
    m_application(application)
{
    m_name = "";
    m_description = "";
    m_usage = "";
    m_example = "";
}
// _____________________________________________________________________
CInstruction::~CInstruction()
{
}
// _____________________________________________________________________
QString CInstruction::instructionName()
{
    return m_name;
}
// _____________________________________________________________________
QString CInstruction::description()
{
    return m_description;
}
// _____________________________________________________________________
QString CInstruction::usage()
{
    return m_usage;
}
// _____________________________________________________________________
QString CInstruction::example()
{
    return m_example;
}
// _____________________________________________________________________
QString CInstruction::help()
{
    return formatHelp();
}
// _____________________________________________________________________
bool CInstruction::isValid(QByteArray instruction)
{
    return QString(instruction).contains(m_reg_exp);
}
// _____________________________________________________________________
QString CInstruction::formatHelp()
{
    QString formated_help;
    formated_help = "- " + usage() + ": " + description() + "\n";
    if (!example().isEmpty()) {
        formated_help += "   > example : " + example();
    }
    return formated_help;
}
// _____________________________________________________________________
QByteArray CInstruction::formatResponse(unsigned long errCode, QByteArrayList response)
{
    QByteArray formated_response;
    formated_response = QString::number(errCode).toUtf8();
    foreach (QByteArray ret, response) {
        formated_response += SEPARATOR_BETWEEN_RESPONSE_ARGUMENTS + ret;
    }
    return formated_response;
}

// =====================================================================
//            CInstruction_Help
// =====================================================================
CInstruction_Help::CInstruction_Help(CApplication *application)
    : CInstruction(application),
      m_instructions_set(NULL)
{
    m_name = "help";
    m_description = "Display instructions set";
    m_usage = "help or help()";
    m_example = "help";
    m_reg_exp = QRegExp("^\\s*(" + m_name + ")[\\(\\s*\\)]*\\s*$");
}
// _____________________________________________________________________
void CInstruction_Help::setInstructionsSet(CInstructionsSet::tListInstruction *instructions_set)
{
    m_instructions_set = instructions_set;
}
// _____________________________________________________________________
QByteArray CInstruction_Help::execute(QByteArray instruction)
{
    QByteArrayList response;
    if (isValid(instruction)) {
        QByteArray help;
        for (CInstructionsSet::tListInstruction::iterator it = m_instructions_set->begin(); it!=m_instructions_set->end(); it++)
        {
             help += it.value()->help().toUtf8() + "\n";
        }
        response.append(help);
    }
    return formatResponse(INSTRUCTION_OK, response);
}

// =====================================================================
//            CInstruction_HelpInstruction
// =====================================================================
CInstruction_HelpInstruction::CInstruction_HelpInstruction(CApplication *application)
    : CInstruction(application),
      m_instructions_set(NULL)
{
    m_name = "helpInstruction";
    m_description = "Display help about an instruction";
    m_usage = m_name + "(instruction_name)";
    m_example = m_name + "(setData)";
    m_reg_exp = QRegExp("^\\s*(" + m_name + ")\\s*\\(\\s*(.+)\\s*\\)\\s*$");
}
// _____________________________________________________________________
void CInstruction_HelpInstruction::setInstructionsSet(CInstructionsSet::tListInstruction *instructions_set)
{
    m_instructions_set = instructions_set;
}
// _____________________________________________________________________
QByteArray CInstruction_HelpInstruction::execute(QByteArray instruction)
{
    QByteArrayList response;

    if (!m_instructions_set) {
        return formatResponse(INTERNAL_ERROR);
    }
    m_reg_exp.indexIn(instruction);
    // Vérifie si une instruction est dans la parenthèse
    if (m_reg_exp.captureCount() != 2) {
        response.append(instruction);
        return formatResponse(INVALID_ARGUMENT, response);
    }
    // Vérifie si l'instruction pour laquelle l'aide est demandé est connue
    QString instruction_to_help = m_reg_exp.cap(2).trimmed();
    CInstructionsSet::tListInstruction::iterator it=m_instructions_set->find(instruction_to_help);
    if (it == m_instructions_set->end()) {
        response.append(instruction);
        response.append(instruction_to_help.toUtf8());
        return formatResponse(INVALID_ARGUMENT, response);
    }
    response.append(it.value()->help().toUtf8());

    return formatResponse(INSTRUCTION_OK, response);
}

// =====================================================================
//            CInstruction_setData
// =====================================================================
CInstruction_setData::CInstruction_setData(CApplication *application)
    : CInstruction(application)
{
    m_name = "setData";
    m_description = "Set the value of a data in the DataManager";
    m_usage = m_name + "(data_name, data_value)";
    m_example = "setData(PosX, 12.3)";
    m_reg_exp = QRegExp("^\\s*(" + m_name + ")\\s*\\(\\s*(.+)\\s*,\\s*(.+)\\s*\\)\\s*$");
}
// _____________________________________________________________________
QByteArray CInstruction_setData::execute(QByteArray instruction)
{
    QByteArrayList response;

    m_reg_exp.indexIn(instruction);
    // Vérifie si l'instruction est bien composée
    // setData(variable, valeur)
    //   #1      #2       #3
    if (m_reg_exp.captureCount() != 3) {
        response.append(instruction);
        return formatResponse(INVALID_ARGUMENT, response);
    }
    if (!m_application) {
        return formatResponse(INTERNAL_ERROR);
    }
    // Vérifie si le nom de la data est est connue
    QString dataname = m_reg_exp.cap(2).trimmed();
    CData *data = m_application->m_data_center->getData(dataname);
    if (!data) {
        response.append(instruction);
        response.append(dataname.toUtf8());
        return formatResponse(INVALID_ARGUMENT, response);
    }

    QVariant value = m_reg_exp.cap(3).trimmed();
    data->write(value);

    return formatResponse(INSTRUCTION_OK);
}
// =====================================================================
//            CInstruction_getData
// =====================================================================
CInstruction_getData::CInstruction_getData(CApplication *application)
    : CInstruction(application)
{
    m_name = "getData";
    m_description = "Get the value of a data in the DataManager";
    m_usage = m_name + "(data_name)";
    m_example = "getData(PosX)";
    m_reg_exp = QRegExp("^\\s*(" + m_name + ")\\s*\\(\\s*(.+)\\s*\\)\\s*$");
}
// _____________________________________________________________________
QByteArray CInstruction_getData::execute(QByteArray instruction)
{
    QByteArrayList response;

    m_reg_exp.indexIn(instruction);
    // Vérifie si l'instruction est bien composée
    // getData(variable)
    //   #1      #2
    if (m_reg_exp.captureCount() != 2) {
        response.append(instruction);
        return formatResponse(INVALID_ARGUMENT, response);
    }
    if (!m_application) {
        return formatResponse(INTERNAL_ERROR);
    }
    // Vérifie si le nom de la data est est connue
    QString dataname = m_reg_exp.cap(2).trimmed();
    CData *data = m_application->m_data_center->getData(dataname);
    if (!data) {
        response.append(instruction);
        response.append(dataname.toUtf8());
        return formatResponse(INVALID_ARGUMENT, response);
    }

    response.append(data->read().toByteArray());

    return formatResponse(INSTRUCTION_OK, response);
}
// =====================================================================
//            CInstruction_getDatas
// =====================================================================
CInstruction_getDatas::CInstruction_getDatas(CApplication *application)
    : CInstruction(application)
{
    m_name = "getDatas";
    m_description = "Get all datas and their value";
    m_usage = m_name + "()";
    m_example = m_name + "()";
    m_reg_exp = QRegExp("^\\s*(" + m_name + ")[\\(\\s*\\)]*\\s*$");
}
// _____________________________________________________________________
QByteArray CInstruction_getDatas::execute(QByteArray instruction)
{
    QByteArrayList response;
    if (!isValid(instruction)) {
        response.append(instruction);
        return formatResponse(INVALID_INSTRUCTION, response);
    }
    if (!m_application) {
        return formatResponse(INTERNAL_ERROR);
    }

    response.append(m_application->m_data_center->getDataValues().toUtf8());

    return formatResponse(INSTRUCTION_OK, response);
}
// =====================================================================
//            CInstruction_createData
// =====================================================================
CInstruction_createData::CInstruction_createData(CApplication *application)
    : CInstruction(application)
{
    m_name = "createData";
    m_description = "Create a data in the DataManager";
    m_usage = m_name + "(data_name, initial_value)";
    m_example = m_name + "(myNewData, 56.78)";
    m_reg_exp = QRegExp("^\\s*(" + m_name + ")\\s*\\(\\s*(.+)\\s*,\\s*(.+)\\s*\\)\\s*$");
}
// _____________________________________________________________________
QByteArray CInstruction_createData::execute(QByteArray instruction)
{
    QByteArrayList response;

    m_reg_exp.indexIn(instruction);
    // Vérifie si l'instruction est bien composée
    // createData(variable, valeur)
    //   #1         #2       #3
    if (m_reg_exp.captureCount() != 3) {
        response.append(instruction);
        return formatResponse(INVALID_ARGUMENT, response);
    }
    if (!m_application) {
        return formatResponse(INTERNAL_ERROR);
    }
    // Vérifie si la donnée n'existe pas déjà
    QString dataname = m_reg_exp.cap(2).trimmed();
    CData *data = m_application->m_data_center->getData(dataname);
    if (data) {
        response.append(instruction);
        response.append(dataname.toUtf8());
        return formatResponse(INVALID_ARGUMENT, response);
    }

    QVariant value = m_reg_exp.cap(3).trimmed();
    m_application->m_data_center->write(dataname, value);

    return formatResponse(INSTRUCTION_OK);
}
