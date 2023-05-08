#include <math.h>

#include "datatransformer_mean.h"

CDataTransformer_Mean::CDataTransformer_Mean(QString datas_lst_in_str,
                                               QString data_out_str,
                                               CDataManager *data_mngr,
                                               QString event_connection,
                                               QString params,
                                               QString description,
                                               QObject *parent)
    : CDataTransformer(datas_lst_in_str, data_out_str, data_mngr, event_connection, params, description, parent),
      m_sample_count(1)
{
    if (!m_data_out) return;
    if (m_data_lst_in.size() != 1) return;

    // Connexion de la donnée d'entrée vers la fonction de mise à jour de la donnée de sortie
    if (m_data_lst_in[0]) {
        if (interpretEventConnection(event_connection) == CDataTransformer::DATA_CHANGED) {
            connect(m_data_lst_in[0], SIGNAL(valueChanged(QVariant)), this, SLOT(data_updated(QVariant)));
        }
        else {
            connect(m_data_lst_in[0], SIGNAL(valueUpdated(QVariant)), this, SLOT(data_updated(QVariant)));
        }
    }

    QStringList lst_params_str = simplifyParams(params);
    if (lst_params_str.size() >= 1) m_sample_count = lst_params_str[0].toInt();

    // initialise la donnée de sortie
    data_updated(m_data_lst_in[0]->read());
}

// _____________________________________________________________________
/*!
* \brief Calcule la valeur de la variable de sortie à partir de la variable d'entrée et des coefs du polynome d'ordre "n"
* \param val y = Moyenne(x)
*/
void CDataTransformer_Mean::data_updated(QVariant val)
{
    double val_double = val.toDouble();
    while (m_fifo.size() < m_sample_count) m_fifo.enqueue(val_double);

    m_fifo.enqueue(val_double); // ajoute le nouvel élément à la fifo
    m_fifo.dequeue();           // élimine le dernier élément de la fifo

    double moy=0;
    for (int i=0; i<m_fifo.size(); i++) moy += m_fifo[i];
    moy = moy / m_sample_count;
    m_data_out->write(moy);
}

// ___________________________________________________________________
/*!
 * \brief Renvoie le nom court de l'algo
 * \return le nom de l'algo
 */
QString CDataTransformer_Mean::getShortAlgoName()
{
   return "mean";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie une formule simplifiée sous forme d'une chaine de caractère
 * \return la formule simplifiée
 */
QString CDataTransformer_Mean::getFormula()
{
   return "y = Mean(x)";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie l'aide sur la transformation
 * \return l'aide sur la transformation
 */
QString CDataTransformer_Mean::getHelp()
{
   return "Calcule la moyenne de la donnée d'entrée sur n échantillons";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie le template des paramètres d'entrés
 * \return le template
 * \remarks :
 *      1er paramètre : nombre d'échantillons pour le calcul de la moyenne
 *
 */
QString CDataTransformer_Mean::getParamsTemplate()
{
    return "10";
}
