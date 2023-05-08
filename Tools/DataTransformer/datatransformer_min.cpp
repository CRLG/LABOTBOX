#include <math.h>

#include "datatransformer_min.h"

CDataTransformer_Min::CDataTransformer_Min(QString datas_lst_in_str,
                                           QString data_out_str,
                                           CDataManager *data_mngr,
                                           QString event_connection,
                                           QString params,
                                           QString description,
                                           QObject *parent)
    : CDataTransformer(datas_lst_in_str, data_out_str, data_mngr, event_connection, params, description, parent)
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
        m_min = m_data_lst_in[0]->read().toDouble(); // initialise la valeur max avec la valeur de la variable d'entrée la première fois
        m_data_out->write(m_min);  // initialise la donnée de sortie
    }
}

// _____________________________________________________________________
/*!
* \brief Calcule la valeur de la variable de sortie à partir de la variable d'entrée et des coefs du polynome
* \param val data_out = A*data_in² + B*data_in + C (exemple pour ordre 2)
* \remarks le premier paramètre du vecteur est le coef de l'ordre du polynome
*/
void CDataTransformer_Min::data_updated(QVariant val)
{
    double dval = val.toDouble();
    if ( dval < m_min) {
        m_min = dval;
        m_data_out->write(dval);
    }
}

// ___________________________________________________________________
/*!
 * \brief Renvoie le nom court de l'algo
 * \return le nom de l'algo
 */
QString CDataTransformer_Min::getShortAlgoName()
{
   return "min";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie une formule simplifiée sous forme d'une chaine de caractère
 * \return la formule simplifiée
 */
QString CDataTransformer_Min::getFormula()
{
   return "y = MIN(x)";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie l'aide sur la transformation
 * \return l'aide sur la transformation
 */
QString CDataTransformer_Min::getHelp()
{
   return "Conserve la valeur minimum de la donnée d'entrée";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie le template des paramètres d'entrés
 * \return le template
 */
QString CDataTransformer_Min::getParamsTemplate()
{
    return "--";
}
