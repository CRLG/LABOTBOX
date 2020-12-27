/*! \file CDataListOpenSave.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef CDATA_LIST_OPEN_SAVE_H_
#define CDATA_LIST_OPEN_SAVE_H_

#include <QStringList>

 /*! \addtogroup DATA_LIST_OPEN_SAVE
   * 
   *  @{
   */

/*! @brief class CDataListOpenSave
 */	   
class CDataListOpenSave
{
public:
    CDataListOpenSave();
    ~CDataListOpenSave();

    static bool open(QString pathfilename, QStringList &out_data_lst);
    static bool save(QString pathfilename, QStringList& in_data_lst);
};

#endif // CDATA_LIST_OPEN_SAVE_H_

/*! @} */

