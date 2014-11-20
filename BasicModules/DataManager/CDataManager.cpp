/*! \file CDataManager.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include "CDataManager.h"

/*! \addtogroup DataManager
   * 
   *  @{
   */
	   
// _____________________________________________________________________
/*!
*  Constructeur
*
*/
CDataManager::CDataManager(const char *plugin_name)
    :CBasicModule(plugin_name, VERSION_DATA_MANAGER, AUTEUR_DATA_MANAGER, INFO_DATA_MANAGER)
{
 m_map_data.clear();
}


// _____________________________________________________________________
/*!
*  Destructeur
*
*/
CDataManager::~CDataManager()
{
  m_map_data.clear();
}

// _____________________________________________________________________
/*!
*  Ecrit une  valeur dans la data
*
*  \param [in] varname nom de la variable � ecrire
*  \param [in] val la valeur � donner � la variable au format g�n�rique QVariant
*  \remarks si la data n'existe pas, elle est cree automatiquement
*/
void CDataManager::write(QString varname, QVariant val)
{
  m_mutex.lock();
  CData *data = getData(varname);

  if (data == NULL) { // la variable n'existe pas encore -> la cr��e
    data = new CData(varname, val);
    m_map_data.insert(varname, data);
    emit valueChanged(data);
  }
  else {
   if (data->read() != val) {  // Pas d'�criture si la valeur n'a pas chang�
    data->write(val);
    emit valueChanged(data);
   }
  }
  m_mutex.unlock();
}

// _____________________________________________________________________
/*!
*  Lit la valeur de la data
*
*  \param [in] varname nom de la variable � lire
*  \return la valeur de la variable au format g�n�rique QVariant
*  \remarks l'acc�s au map est r�alis� en section critique pour assurer l'utilisation en multi-thread
*/
QVariant CDataManager::read(QString varname)
{
  QVariant ret = "";
  m_mutex.lock();
  if (isExist(varname)) {
      ret = m_map_data.value(varname)->read();
  }
  m_mutex.unlock();
  // sinon, c'est la valeur par d�faut "" qui est renvoy�e dans le QVariant
  return(ret);
}

// _____________________________________________________________________
/*!
*  R�cup�re un pointeur sur la data pour la manipuer directement sans passer par le DataManager
*
*  \param [in] varname nom de la variable
*  \return \li le pointeur sur la data de type CData
*          \li NULL si la data n'�xiste pas
*/
CData *CDataManager::getData(QString varname)
{
 return(m_map_data.value(varname, NULL));
}


// _____________________________________________________________________
/*!
*  Affiche la valeur de chaque variable dans la console de debug
*
*/
void CDataManager::debug(void)
{
  qDebug() << getDataValues();
}

// _____________________________________________________________________
/*!
*  Extrait le contenu du DataManager (liste des donn�es et leur valeur) dans un QString
*
*  \return la liste des variables et leur valeur au format "Nom=valeur"
*/
QString CDataManager::getDataValues(void)
{
  t_map_data::const_iterator it;
  QString ret = "";

  for (it=m_map_data.constBegin();  it!=m_map_data.constEnd(); it++) {
   ret += it.key();
   ret += "=";
   ret += it.value()->read().toString();
   ret += "\n";
  }
  return(ret);
}

// _____________________________________________________________________
/*!
*  V�rifie l'existance d'une data dans la liste
*
*  \param [in] varname nom de la variable � tester
* \return \li true si la variable existe
*         \li false sinon.
*/
bool CDataManager::isExist(QString varname)
{
 return(m_map_data.contains(varname));
}

// _____________________________________________________________________
/*!
*  Supprime une data de la liste si elle existe
*
*  \param [in] varname nom de la variable � supprimer
*  \remarks l'acc�s au map est r�alis� en section critique pour assurer l'utilisation en multi-thread
*/
void CDataManager::remove(QString varname)
{
 t_map_data::iterator it;
 m_mutex.lock();
 it = m_map_data.find(varname);
 if (it != m_map_data.end()) {
     delete (it.value());  // lib�re le CData cr�� dynamiquement
     m_map_data.erase(it);
 }
 m_mutex.unlock();
}

// _____________________________________________________________________
/*!
*  Renvoie le nombre de data contenues dans le DataManager
*
*  \return le nombre de variables (=la taille)
*/
unsigned long CDataManager::size(void)
{
 return(m_map_data.size());
}

// _____________________________________________________________________
/*!
*  Renvoie la liste des noms de variables d�clar�es
*
*  \return le nombre de variables (=la taille)
*/
void CDataManager::getListeVariablesName(QStringList &var_list)
{
 t_map_data::const_iterator it;

 for (it=m_map_data.constBegin();  it!=m_map_data.constEnd(); it++) {
    var_list.append(it.key());
 }

}


/*! @} */
