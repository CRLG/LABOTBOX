#include <QDebug>
#include <CData.h>
#include <math.h>

#include "datatransformer_addition2D.h"

CDataTransformer_Addition2D::CDataTransformer_Addition2D(QString datas_lst_in_str,
                                                       QString data_out_str,
                                                       CDataManager *data_mngr,
                                                       QString event_connection,
                                                       QString params,
                                                       QString description,
                                                       QObject *parent)
    : CDataTransformer(datas_lst_in_str, data_out_str, data_mngr, event_connection, params, description, parent),
      m_a1(1),
      m_b1(0),
      m_a2(1),
      m_b2(0)
{
    if (!m_data_out) return;
    if (m_data_lst_in.size() != 2) return;

    foreach (CData *data, m_data_lst_in) {
        // Connexion de la donnée d'entrée vers la fonction de mise à jour de la donnée de sortie
        if (data) {
            if (interpretEventConnection(event_connection) == CDataTransformer::DATA_CHANGED) {
                connect(data, SIGNAL(valueChanged(QVariant)), this, SLOT(data_updated()));
            }
            else {
                connect(data, SIGNAL(valueUpdated(QVariant)), this, SLOT(data_updated()));
            }
        }
    }

    // les paramètres d'entrés s'ils existent
    if (m_params_lst_in.size() >= 1) m_a1 = m_params_lst_in[0].toDouble();
    if (m_params_lst_in.size() >= 2) m_b1 = m_params_lst_in[1].toDouble();
    if (m_params_lst_in.size() >= 3) m_a2 = m_params_lst_in[2].toDouble();
    if (m_params_lst_in.size() >= 4) m_b2 = m_params_lst_in[3].toDouble();

    // met à jour la donnée de sortie pour la première fois
    data_updated();
}

// ________________________________________________________________________________
/*!
* \brief Calcule la valeur de la variable de sortie à partir de la variable d'entrée et des coefs du polynome d'ordre "n"
* \param val data_out = (a1 * data1 + b1) + (a2 * data2 + b2)
* \remarks le premier paramètre du vecteur est le coef de l'ordre du polynome
*/
void CDataTransformer_Addition2D::data_updated()
{
    double new_val;

    new_val = (m_a1 * m_data_lst_in[0]->read().toDouble() + m_b1) +
              (m_a2 * m_data_lst_in[1]->read().toDouble() + m_b2);
    m_data_out->write(new_val);
}


// ___________________________________________________________________
/*!
 * \brief Renvoie le nom court de l'algo
 * \return le nom de l'algo
 */
QString CDataTransformer_Addition2D::getShortAlgoName()
{
   return "addition2D";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie une formule simplifiée sous forme d'une chaine de caractère
 * \return la formule simplifiée
 */
QString CDataTransformer_Addition2D::getFormula()
{
   return "y = (p1*x1+p2) + (p3*x2+p4)";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie l'aide sur la transformation
 * \return l'aide sur la transformation
 */
QString CDataTransformer_Addition2D::getHelp()
{
    return "Additionne 2 données entre elles";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie le template des paramètres d'entrés
 * \return le template
 */
QString CDataTransformer_Addition2D::getParamsTemplate()
{
    return "1.0, 0.0, 1.0, 0.0";
}
