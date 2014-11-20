/*! \file CData.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */

#include "CData.h"

/*! \addtogroup DataManager
   * 
   *  @{
   */

	   
CData::CData()
{
 m_data.clear();
 m_name = "";
}

CData::CData(QString name, QVariant val)
    : m_name(name)
{
 write(val);
}


CData::~CData()
{
  m_data.clear();
}

// _____________________________________________________________________
/*!
*  Ecrit une valeur dans la data
*
*  \param [in] data la valeur de la donn�e au format g�n�rique QVariant
*  \remarks le signal valueChanged est �mis si la valeur de la donn�e a chang�
*  \remarks l'�criture de la donn�e est r�alis�e en section critique pour assurer l'utilisation en multi-thread
*/
void CData::write(QVariant data)
{
  m_mutex.lock();
  if (data != m_data) {
    m_data = data;
    emit valueChanged(data);
  }
  else {
    m_data = data;
  }
  m_mutex.unlock();
}

// _____________________________________________________________________
/*!
*  Lit la valeur de la data
*
*  \return la valeur de la data au format QVariant
*/
QVariant CData::read(void)
{
  return(m_data);
}

/*! @} */
