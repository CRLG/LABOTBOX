/*! \file CDataManager.cpp
 * A brief file description CPP.
 * A more elaborated file description.
 */
#include <QDebug>
#include <QSettings>
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
*  Initialisation du module
*
*  \remarks le nom du fichier est le nom du module .ini
*/
void CDataManager::init(CApplication *application)
{
  CBasicModule::init(application);
  loadDataProperties();
}

// _____________________________________________________________________
/*!
*  Fermeture du module
*
*/
void CDataManager::close(void)
{
  saveDataProperties();
}


// _____________________________________________________________________
/*!
*  Ecrit une  valeur dans la data
*
*  \param [in] varname nom de la variable à ecrire
*  \param [in] val la valeur à donner à la variable au format générique QVariant
*  \remarks si la data n'existe pas, elle est cree automatiquement
*/
void CDataManager::write(QString varname, QVariant val)
{
  CData *data = getData(varname);

  if (data == NULL) { // la variable n'existe pas encore -> la créée
    data = createData(varname, val);
    emit valueChanged(data);
    emit valueUpdated(data);
  }
  else {
   data->write(val);
   if (data->read() != val) {
    emit valueChanged(data);
   }
   emit valueUpdated(data);
  }
}

// _____________________________________________________________________
/*!
*  Lit la valeur de la data
*
*  \param [in] varname nom de la variable à lire
*  \return la valeur de la variable au format générique QVariant
*  \remarks l'accès au map est réalisé en section critique pour assurer l'utilisation en multi-thread
*/
QVariant CDataManager::read(QString varname)
{
  QVariant ret = "";
  if (isExist(varname)) {
      ret = getData(varname)->read();
  }
  // sinon, c'est la valeur par défaut "" qui est renvoyée dans le QVariant
  return(ret);
}


// _____________________________________________________________________
/*!
*  Crée une Data et l'ajoute au DataManager si elle n'existe pas déjà
*
*  \param [in] varname nom de la variable
*  \param [in] init_val : la valeur d'init
*  \return \li le pointeur sur la nouvelle data de type CData
*          \li NULL si la data n'éxiste pas
*/
CData *CDataManager::createData(QString varname, QVariant init_val)
{
    // Pas de doublon si la data existe déjà
    CData *data = getData(varname, false);
    if (data) return data;

    // La donnée n'existe pas, la créé
    data = new CData(varname, init_val);
    m_mutex.lock();
    m_map_data.insert(varname, data);
    m_mutex.unlock();
    emit dataCreated(data);
    return data;
}


// _____________________________________________________________________
/*!
*  Récupère un pointeur sur la data pour la manipuer directement sans passer par le DataManager
*
*  \param [in] varname nom de la variable
*  \param [in] force_creation crée la Data si elle n'existe pas
*  \return \li le pointeur sur la data de type CData
*          \li NULL si la data n'éxiste pas
*/
CData *CDataManager::getData(QString varname, bool force_creation)
{
    if (force_creation && !isExist(varname)) {
        return createData(varname);
    }

    QMutexLocker mtx_locker(&m_mutex);
    return(m_map_data.value(varname, NULL));
}


// _____________________________________________________________________
/*!
*  Affiche la valeur de chaque variable dans la console de debug
*
*/
void CDataManager::debug(void)
{
    QString sdebug=getDataValues();
    //qDebug() << sdebug;
}

// _____________________________________________________________________
/*!
*  Extrait le contenu du DataManager (liste des données et leur valeur) dans un QString
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
 * \brief Renvoie la liste des datas contenues dans le DataManager
 * \param data_list la liste de sortie
 */
void CDataManager::getDataList(QVector<CData *> &data_list)
{
    t_map_data::const_iterator it;

    for (it=m_map_data.constBegin();  it!=m_map_data.constEnd(); it++) {
        data_list.append(it.value());
    }
}


// _____________________________________________________________________
/*!
*  Vérifie l'existance d'une data dans la liste
*
*  \param [in] varname nom de la variable à tester
* \return \li true si la variable existe
*         \li false sinon.
*/
bool CDataManager::isExist(QString varname)
{
    QMutexLocker mtx_locker(&m_mutex);
    return(m_map_data.contains(varname));
}

// _____________________________________________________________________
/*!
*  Supprime une data de la liste si elle existe
*
*  \param [in] varname nom de la variable à supprimer
*  \remarks l'accès au map est réalisé en section critique pour assurer l'utilisation en multi-thread
*/
void CDataManager::remove(QString varname)
{
 t_map_data::iterator it;
 m_mutex.lock();
 it = m_map_data.find(varname);
 if (it != m_map_data.end()) {
     delete (it.value());  // libère le CData créé dynamiquement
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
    QMutexLocker mtx_locker(&m_mutex);
    return(m_map_data.size());
}

// _____________________________________________________________________
/*!
*  Renvoie la liste des noms de variables déclarées
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

// _____________________________________________________________________
/*!
*  Charge un fichier de configuration des propriétés de chaque variable
*/
void CDataManager::loadDataProperties(void)
{
 QString pathfilename;
 QSettings *settings;
 pathfilename = m_application->m_pathname_config_file + "/" + getName() + ".ini";
 settings = new QSettings(pathfilename, QSettings::IniFormat);
 settings->setIniCodec("UTF-8");

 QStringList keys = settings->childGroups();
 //qDebug() << keys;
 for (int i=0; i<keys.size(); i++) {
     QString var_name = keys.at(i);
     if (m_application->m_data_center->isExist(var_name) == false) {
        m_application->m_data_center->write(var_name, ""); // crée la variable si elle n'existe pas
     }
     // récupère la liste des propriétés de cette variable
     settings->beginGroup(var_name);
     QStringList liste_proprietes = settings->childKeys();
     for (int j=0; j<liste_proprietes.size(); j++) {
        QString prop_name = liste_proprietes.at(j);
        QVariant prop_value = settings->value(prop_name);
        setDataProperty(var_name, prop_name, prop_value);
     }
     settings->endGroup();
 }

 delete settings;
}
// _____________________________________________________________________
/*!
*  Charge un fichier de configuration des propriétés de chaque variable
*/
void CDataManager::saveDataProperties(void)
{
 QString pathfilename;
 QSettings *settings;
 pathfilename = m_application->m_pathname_config_file + "/" + getName() + ".ini";
 settings = new QSettings(pathfilename, QSettings::IniFormat);
 settings->setIniCodec("UTF-8");

 t_map_data::const_iterator it;
 for (it=m_map_data.constBegin();  it!=m_map_data.constEnd(); it++) {
     QStringList list_properties;
     it.value()->getPropertiesList(list_properties);
     for (int i=0; i<list_properties.size(); i++) {
         QString prop_name =list_properties.at(i);
         QVariant prop_value = it.value()->getProperty(prop_name);
         QString section_prop_name = it.key() + "/" + prop_name;
         settings->setValue(section_prop_name, prop_value);
     } // for toutes les propriétés de la variale
 } // for toutes les variables

 delete settings;
}


// _____________________________________________________________________
/*!
*  Positionne la valeur d'une propriété d'une variable
*
* \param [in] varname nom de la variable à tester
* \param [in] prop_name nom de la propriété à affecter (ou à créer si elle n'existe pas déjà)
* \param [in] val valeur de la propriété
* \remarks si la propriété n'existe pas déjà, elle est créée
* \remarks si la data n'existe pas, la fonction ne fait rien
*/
void CDataManager::setDataProperty(QString varname, QString prop_name, QVariant val)
{
  CData *data = getData(varname);
  if (data != NULL) {
    data->setProperty(prop_name, val);
  }
}

// _____________________________________________________________________
/*!
*  Lit la valeur d'une propriété d'une variable
*
* \param [in] varname nom de la variable à tester
* \param [in] prop_name nom de la propriété à affecter (ou à créer si elle n'existe pas déjà)
* \param [in] val valeur de la propriété
* \return la valeur de la propriété
* \remarks si la data ou la propriété n'existe pas, la fonction renvoie un QVariant vide
*/
QVariant CDataManager::getDataProperty(QString varname, QString prop_name)
{
  QVariant val = QVariant("");
  CData *data = getData(varname);
  if (data != NULL) {
    val = data->getProperty(prop_name);
  }
  return(val);
}


// _____________________________________________________________________
/*!
*  Lit la valeur d'une propriété d'une variable
*
* \param [in] varname nom de la variable à tester
* \param [in] timeout_msec durée avant de déclarer une perte de la donnée et forcer sa valeur par défaut
* \param [in] valdef valeur valeur par défaut à appliquer en cas de non rafraichissement de la donnée
*/
void CDataManager::startMonitoring(QString varname, unsigned long timeout_msec, QVariant valdef)
{
    CData *data = getData(varname);
    if (data != NULL) {
      data->startMonotoring(timeout_msec, valdef);
    }
}



/*! @} */
