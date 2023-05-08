#include <math.h>

#include "datatransformer_sinus.h"

CDataTransformer_Sinus::CDataTransformer_Sinus(QString datas_lst_in_str,
                                               QString data_out_str,
                                               CDataManager *data_mngr,
                                               QString event_connection,
                                               QString params,
                                               QString description,
                                               QObject *parent)
    : CDataTransformer(datas_lst_in_str, data_out_str, data_mngr, event_connection, params, description, parent),
      m_coefs_A(1),
      m_coefs_a(1),
      m_coefs_b(0)
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
    if (lst_params_str.size() >= 1) m_coefs_A = lst_params_str[0].toDouble();
    if (lst_params_str.size() >= 2) m_coefs_a = lst_params_str[1].toDouble();
    if (lst_params_str.size() >= 3) m_coefs_b = lst_params_str[2].toDouble();

    // initialise la donnée de sortie
    data_updated(m_data_lst_in[0]->read());
}

// _____________________________________________________________________
/*!
* \brief Calcule la valeur de la variable de sortie à partir de la variable d'entrée et des coefs du polynome d'ordre "n"
* \param val data_out = A * sin (a*x + b)
* \remarks  A : amplitude = 1er paramètre du tableau de coeficients
*           a : 2ème paramètre
*           b : 3ème paramètre
*           La donnée d'entrée doit être en radian
*/
void CDataTransformer_Sinus::data_updated(QVariant val)
{
    double new_val= m_coefs_A * sin(m_coefs_a * val.toDouble() + m_coefs_b);
    m_data_out->write(new_val);
}

// ___________________________________________________________________
/*!
 * \brief Renvoie le nom court de l'algo
 * \return le nom de l'algo
 */
QString CDataTransformer_Sinus::getShortAlgoName()
{
   return "sin";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie une formule simplifiée sous forme d'une chaine de caractère
 * \return la formule simplifiée
 */
QString CDataTransformer_Sinus::getFormula()
{
   return "y = p1 * sin(p2*x+p3)";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie l'aide sur la transformation
 * \return l'aide sur la transformation
 */
QString CDataTransformer_Sinus::getHelp()
{
   return "Calcule du sinus de la donnée d'entrée";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie le template des paramètres d'entrés
 * \return le template
 */
QString CDataTransformer_Sinus::getParamsTemplate()
{
    return "1.0, 1.0, 0.0";
}
