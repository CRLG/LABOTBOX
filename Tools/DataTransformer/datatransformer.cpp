
#include "CDataManager.h"
#include "datatransformer.h"

CDataTransformer::CDataTransformer(QString datas_lst_in_str,
                                   QString data_out_str,
                                   CDataManager *data_mngr,
                                   QString event_connection,
                                   QString params,
                                   QString description,
                                   QObject *parent)
    : QObject(parent),
      m_data_out(Q_NULLPTR),
      m_event_connection(event_connection),
      m_params(params),
      m_description(description)
{
    if (!data_mngr) return;

    // Décompose la liste des données d'entrées qui peuvent arriver sous la forme "{ data1, data2 }"
    QStringList data_in_lst_str = simplifyParams(datas_lst_in_str);
    foreach (QString dataname, data_in_lst_str) {
        m_data_lst_in.append(data_mngr->getData(dataname, true));
    }
    // La donnée de sortie
    m_data_out = data_mngr->getData(data_out_str, true);

    // Décompose les paramètres d'entrés
    m_params_lst_in = simplifyParams(params);
}



// ___________________________________________________________________
/*!
 * \brief Renvoie la description de l'algorithme
 * \return la description
 */QString CDataTransformer::getDescription()
{
    return m_description;
}

// ___________________________________________________________________
/*!
 * \brief Renvoie la liste des noms des données d'entrées
 * \return la liste des noms
 */
QStringList CDataTransformer::getInputDataName()
{
    QStringList lst;
    foreach(CData *data, m_data_lst_in) {
        if (data) lst.append(data->getName());
    }
    return lst;
}

// ___________________________________________________________________
/*!
 * \brief Renvoie le nom de la donnée de sortie
 * \return le nom
 */
QString CDataTransformer::getOutputDataName()
{
    QString name;
    if (m_data_out) name = m_data_out->getName();
    return name;
}

// ___________________________________________________________________
/*!
 * \brief Renvoie l'évènement de mise à jour de la sortie
 * \return le nom de l'évènement
 */
QString CDataTransformer::getEventConnection()
{
    return m_event_connection;
}

// ___________________________________________________________________
/*!
 * \brief Renvoie les paramètres
 * \return les paramètres
 */
QString CDataTransformer::getParams()
{
    return m_params;
}

// ___________________________________________________________________
/*!
 * \brief Convertit les paramètres founis en une ligne en une liste
 * \param params la liste des paramètres de type {param1, param2, param3}
 * \return Une liste de params au format String
 */
QStringList CDataTransformer::simplifyParams(QString params)
{
    QStringList lst;
    params = params.simplified();
    if (params.startsWith(PARAMS_START_TAG)) {
        params = params.remove(PARAMS_START_TAG).remove(PARAMS_END_TAG);
    }
    lst = params.split(PARAMS_SEPARATOR);

    for (int i=0; i<lst.size(); i++) lst[i] = lst[i].simplified();

    // Traite le cas où il n'y a aucun paramètre -> la liste doit être vide
    if ( (lst.size() == 1) && (lst.at(0).isEmpty()) ) lst.clear();

    return lst;
}

// ___________________________________________________________________
/*!
 * \brief Convertit la chaine de caractère d'évènement à traiter (changement ou mise à jour de la data)
 * \param event_connection l'évènement au format chaine de caractère
 * \return l'évènement au format enum
 */
CDataTransformer::tEventConnectionType CDataTransformer::interpretEventConnection(QString event_connection)
{
    tEventConnectionType event_type;

    QString event_connection_simplified = event_connection.simplified().toLower();
    if (event_connection_simplified.contains("change")) event_type = DATA_CHANGED;
    else                                                event_type = DATA_UPDATED;

    return event_type;
}
