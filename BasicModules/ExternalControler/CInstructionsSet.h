/*! \file CInstructionsSet.h
 * A brief file description header.
 * A more elaborated file description.
 */

#ifndef _CCOMMAND_INTERPRETER_H_
#define _CCOMMAND_INTERPRETER_H_

/*! \addtogroup ExternalControler
   *  Additional documentation for group ExternalControler
   *  @{
   */

#include <QString>
#include <QMap>

class CApplication;
class CInstruction;

/*! @brief .
 */
class CInstructionsSet
{
public:
    typedef QMap<QString, CInstruction *>tListInstruction;

    CInstructionsSet(CApplication *application);
    ~CInstructionsSet();

    QByteArray execute(QByteArray command);

private :
    CApplication *m_application;

    tListInstruction m_instruction_set;

    void init(CApplication *application);
    void clear();
};

#endif // _CCOMMAND_INTERPRETER_H_

/*! @} */

