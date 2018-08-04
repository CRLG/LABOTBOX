/*! \file CInstructionsSet.h
 * A brief file description header.
 * A more elaborated file description.
 */

#ifndef _CINSTRUCTIONS_H_
#define _CINSTRUCTIONS_H_

#include <QString>

#include "CInstructionsSet.h"

/*! \addtogroup ExternalControler
   *  Additional documentation for group ExternalControler
   *  @{
   */

class CApplication;

// ________________________________________________________________
/*! @brief Classe d'interface pour toutes les instructions.
 */
class CInstruction
{
public :
    typedef enum {
        INSTRUCTION_OK = 0,
        INVALID_INSTRUCTION,
        INVALID_ARGUMENT,
        INTERNAL_ERROR
    }tInstructionErrorCode;

#define SEPARATOR_BETWEEN_RESPONSE_ARGUMENTS "|"

public:
    CInstruction(CApplication *application);
    virtual ~CInstruction();

    virtual QString instructionName();
    virtual QString description();
    virtual QString usage();
    virtual QString example();
    virtual QString help();
    virtual bool isValid(QByteArray instruction);
    virtual QByteArray execute(QByteArray instruction) = 0;

protected :
    QString formatHelp();
    QByteArray formatResponse(unsigned long errCode, QByteArrayList response=QByteArrayList());

protected:
    CApplication *m_application;
    QString m_name;
    QString m_description;
    QString m_usage;
    QString m_example;
    QRegExp m_reg_exp;
};

// ________________________________________________________________
/*! @brief .
 */
class CInstruction_Help : public CInstruction
{
public:
    CInstruction_Help(CApplication *application);

    virtual QByteArray execute(QByteArray instruction);

    void setInstructionsSet(CInstructionsSet::tListInstruction *instructions_set);
private:
    CInstructionsSet::tListInstruction *m_instructions_set;
};
// ________________________________________________________________
/*! @brief .
 */
class CInstruction_HelpInstruction : public CInstruction
{
public:
    CInstruction_HelpInstruction(CApplication *application);

    virtual QByteArray execute(QByteArray instruction);

    void setInstructionsSet(CInstructionsSet::tListInstruction *instructions_set);
private:
    CInstructionsSet::tListInstruction *m_instructions_set;
};
// ________________________________________________________________
/*! @brief .
 */
class CInstruction_setData : public CInstruction
{
public:
    CInstruction_setData(CApplication *application);

    virtual QByteArray execute(QByteArray instruction);
};
// ________________________________________________________________
/*! @brief .
 */
class CInstruction_getData : public CInstruction
{
public:
    CInstruction_getData(CApplication *application);

    virtual QByteArray execute(QByteArray instruction);
};
// ________________________________________________________________
/*! @brief .
 */
class CInstruction_getDatas : public CInstruction
{
public:
    CInstruction_getDatas(CApplication *application);

    virtual QByteArray execute(QByteArray instruction);
};
// ________________________________________________________________
/*! @brief .
 */
class CInstruction_createData : public CInstruction
{
public:
    CInstruction_createData(CApplication *application);

    virtual QByteArray execute(QByteArray instruction);
};

#endif // _CINSTRUCTIONS_H_

/*! @} */

