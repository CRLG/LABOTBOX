/*! \file CToolBox.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _C_TOOLBOX_H_
#define _C_TOOLBOX_H_

#include <QDateTime>

 /*! \addtogroup ToolBox
   * 
   *  @{
   */

/*! @brief class CToolBox
 * Impl�mente une classe de fonctions outils g�n�riques utilis�es commun�ment par les modules
 
 */
class CToolBox
{
public:
	static QString getDateTime(void) { return(QDateTime::currentDateTime().toString("yyyy_MM_dd_hh'h'mm'm'ss's'")); }
};

#endif // _C_TOOLBOX_H_

/*! @} */

