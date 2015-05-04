/*! \file CData.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */

#include "CData.h"

/*! \addtogroup DataManager
   * 
   *  @{
   */

// _____________________________________________________________________
/*!
* Constructeurs
*
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


// _____________________________________________________________________
/*!
* Destructeur
*
*/
CData::~CData()
{
  m_data.clear();
  m_properties.clear();
}

// _____________________________________________________________________
/*!
*  Ecrit une valeur dans la data
*
*  \param [in] data la valeur de la donnée au format générique QVariant
*  \remarks le signal valueChanged est émis si la valeur de la donnée a changé
*  \remarks l'écriture de la donnée est réalisée en section critique pour assurer l'utilisation en multi-thread
*/
void CData::write(QVariant data)
{
  m_mutex.lock();
  if (data != m_data) {
    m_data = data;
    // Emet le signal dans différent format pour faciliter l'utilisations et la mise à jour des IHM
    emit valueChanged(data);
    emit valueChanged(data.toInt());
    emit valueChanged(data.toBool() );
    emit valueChanged(data.toDouble());
    emit valueChanged(data.toString());
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



// _____________________________________________________________________
/*!
*  Fixe la valeur d'une propriété
*  \param [in] name le nom de la propiete
*  \param [in] value valeur de la propriété
*  \remarks si la propriété existe déjà, sa valeur est écrasée
*  \remarks si la propriété n'existe pas, elle est créée
*  \return --
*/
void CData::setProperty(QString name, QVariant value)
{
  m_properties[name] = value;
}


// _____________________________________________________________________
/*!
*  Lit la valeur d'une propriété
*  \param [in] name le nom de la propiete
*  \remarks si la propriété n'existe pas, une valeur par défaut est renvoyée (QVariant "Invalid" qui conduit à une valeur nulle sur un toInt() ou à une chaine vide sur un toString())
*  \return la valeur de la propriété
*/
QVariant CData::getProperty(QString name)
{
 return(m_properties.value(name));
}


// _____________________________________________________________________
/*!
*  Lit la valeur d'une propriété
*  \param [in] name le nom de la propiete
*  \remarks si la propriété n'existe pas, une valeur par défaut est renvoyée (QVariant "Invalid" qui conduit à une valeur nulle sur un toInt() ou à une chaine vide sur un toString())
*  \return la valeur de la propriété
*/
void CData::getPropertiesList(QStringList &list)
{
 list = m_properties.keys();
}

// _____________________________________________________________________
/*!
*  COnstruit une liste de type propriété=valeur
*  \return une chaine de caractère prêt à afficher
*/
QString CData::getPropertiesString(void)
{
  t_map_properties::const_iterator it;
  QString ret = "";

  for (it=m_properties.constBegin();  it!=m_properties.constEnd(); it++) {
     ret += it.key();
     ret += "=";
     ret += it.value().toString();
     ret += "\n";
  }
  return(ret);
}


/*! @} */
