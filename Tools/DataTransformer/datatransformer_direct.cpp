#include "datatransformer_direct.h"

CDataTransformer_Direct::CDataTransformer_Direct(QString datas_lst_in_str,
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

    // Connexion de la donnée d'entrée vers la donnée de sortie directement
    if (m_data_lst_in[0]) {
        if (interpretEventConnection(event_connection) == CDataTransformer::DATA_CHANGED) {
            connect(m_data_lst_in[0], SIGNAL(valueChanged(QVariant)), m_data_out, SLOT(write(QVariant)));
        }
        else {
            connect(m_data_lst_in[0], SIGNAL(valueUpdated(QVariant)), m_data_out, SLOT(write(QVariant)));
        }
    }

    // initialise la donnée de sortie
    m_data_out->write(m_data_lst_in[0]->read());
}

// ___________________________________________________________________
/*!
 * \brief Renvoie le nom court de l'algo
 * \return le nom de l'algo
 */
QString CDataTransformer_Direct::getShortAlgoName()
{
   return "direct";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie une formule simplifiée sous forme d'une chaine de caractère
 * \return la formule simplifiée
 */
QString CDataTransformer_Direct::getFormula()
{
   return "y = x";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie l'aide sur la transformation
 * \return l'aide sur la transformation
 */
QString CDataTransformer_Direct::getHelp()
{
   return "Recopie directe";
}

// ___________________________________________________________________
/*!
 * \brief Renvoie le template des paramètres d'entrés
 * \return le template
 */
QString CDataTransformer_Direct::getParamsTemplate()
{
    return "--";
}
