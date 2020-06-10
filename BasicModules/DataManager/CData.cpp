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
 m_update_time = 0;
 m_monitored = false;
}

CData::CData(QString name, QVariant val)
    : m_name(name),
      m_monitored(false)
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
  m_update_time = QDateTime::currentMSecsSinceEpoch();
  if (data != m_data) {
    m_data = data;
    m_mutex.unlock();
    // Emet le signal dans différent format pour faciliter l'utilisations et la mise à jour des IHM
    emit valueChanged(data);
    emit valueChanged(data, m_update_time);
    emit valueChanged(data.toInt());
    emit valueChanged(data.toBool() );
    emit valueChanged(data.toDouble());
    emit valueChanged(data.toString());
    emit valueUpdated(data);
    emit valueUpdated(data, m_update_time);
  }
  else {
    m_data = data;
    m_mutex.unlock();
    emit valueUpdated(data);
    emit valueUpdated(data, m_update_time);
  }
  if (m_monitored) {
      m_lost_timer.start();
      m_lost = false;
  }
}

// _____________________________________________________________________
void CData::write(bool value)
{
    write(QVariant(value));
}
// _____________________________________________________________________
void CData::write(int value)
{
    write(QVariant(value));
}
// _____________________________________________________________________
void CData::write(double value)
{
    write(QVariant(value));
}
// _____________________________________________________________________
void CData::write(QString value)
{
    write(QVariant(value));
}

// _____________________________________________________________________
/*!
*  Lit la valeur de la data
*
*  \return la valeur de la data au format QVariant
*/
QVariant CData::read(void)
{
    QMutexLocker mtx_locker(&m_mutex);
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

// _____________________________________________________________________
/*!
*  Indique si la Data est monitorée
*  \return true si la Data est monitorée
*/
bool CData::isMonitored()
{
    return m_monitored;
}

// _____________________________________________________________________
/*!
*  Indique si la Data est perdue (n'a pas été réfraichie depuis un certain temps)
*  \return la valeur par défaut
*/
bool CData::isLost()
{
    return m_lost;
}

// _____________________________________________________________________
/*!
*  Active le monitoring de perte de la data
*  \param [in] timeout_msec la durée en [msec] avant de considérer que la data est perdue
*  \param [in] valdef la valeur à forcer à la data lors que la perte est confirmée
*  \remarks une data est considérée perdue si elle n'a pas été rafraichie depuis un certain temps
*/
void CData::startMonotoring(unsigned long timeout_msec, QVariant valdef)
{
    m_data_valdef = valdef;
    m_lost_time_confirmation = timeout_msec;
    m_monitored = true;
    m_lost_timer.setSingleShot(true);
    connect(&m_lost_timer, SIGNAL(timeout()), this, SLOT(lost_timeout()));
    m_lost_timer.start(m_lost_time_confirmation);
}

// _____________________________________________________________________
/*!
*  Indique si la Data est perdue (n'a pas été réfraichie depuis un certain temps)
*  \param [in] timeout_msec la durée en [msec] avant de considérer que la data est perdue
*  \param [in] valdef la valeur à forcer à la data lors que la perte est confirmée
*/
void CData::stopMonitoring()
{
    m_lost_timer.stop();
    disconnect(&m_lost_timer, SIGNAL(timeout()), this, SLOT(lost_timeout()));
    m_lost = false;
    m_monitored = false;
}

// _____________________________________________________________________
/*!
*  Slot de gestion de la perte de la data
*/
void CData::lost_timeout()
{
    m_mutex.lock();
    m_data = m_data_valdef;
    m_mutex.unlock();
    m_lost = true;
    // Emet le signal dans différent format pour faciliter l'utilisations et la mise à jour des IHM
    emit valueChanged(m_data);
    emit valueChanged(m_data.toInt());
    emit valueChanged(m_data.toBool() );
    emit valueChanged(m_data.toDouble());
    emit valueChanged(m_data.toString());
    emit valueUpdated(m_data);
    emit lost();
}


/*! @} */
