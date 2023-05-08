#include <math.h>

#include "datatransformer_polynom.h"

CDataTransformer_Polynom::CDataTransformer_Polynom(QString datas_lst_in_str,
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
    }

    foreach (QString param, simplifyParams(params)) {
        m_coefs.append(param.toDouble());
    }
    m_order = m_coefs.size() - 1;

    // initialise la donnée de sortie
    data_updated(m_data_lst_in[0]->read());
}

// _____________________________________________________________________
/*!
* \brief Calcule la valeur de la variable de sortie à partir de la variable d'entrée et des coefs du polynome d'ordre "n"
* \param val data_out = A*data_in² + B*data_in + C (exemple pour ordre n=2)
* \remarks le premier paramètre du vecteur est le coef de l'ordre du polynome
*/
void CDataTransformer_Polynom::data_updated(QVariant val)
{
    if (m_order < 0) return;
    double new_val=0;
    for (int i=0; i<=m_order; i++) {
        new_val += m_coefs[i] * pow(val.toDouble(), m_order-i);
    }
    m_data_out->write(new_val);
}


// ___________________________________________________________________
/*!
 * \brief Renvoie le nom court de l'algo
 * \return le nom de l'algo
 */
QString CDataTransformer_Polynom::getShortAlgoName()
{
   return "polynom";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie une formule simplifiée sous forme d'une chaine de caractère
 * \return la formule simplifiée
 */
QString CDataTransformer_Polynom::getFormula()
{
   return "y = p1*x² + p2*x + p3";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie l'aide sur la transformation
 * \return l'aide sur la transformation
 */
QString CDataTransformer_Polynom::getHelp()
{
   return "Calcul suivant un polynôme d'ordre n";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie le template des paramètres d'entrés
 * \return le template
 */
QString CDataTransformer_Polynom::getParamsTemplate()
{
    return "1.0, 1.0, 1.0";
}
