#include "datatransformer_factory.h"
#include "datatransformer_direct.h"
#include "datatransformer_polynom.h"
#include "datatransformer_min.h"
#include "datatransformer_max.h"
#include "datatransformer_sinus.h"
#include "datatransformer_mean.h"
#include "datatransformer_multiply2D.h"
#include "datatransformer_addition2D.h"
#include "CDataManager.h"

CDataTransformerFactory::CDataTransformerFactory()
{
}

// ______________________________________________________________________
/*!
 * \brief Crée une instance de DataTransformer suivant l'algo
 * \return un pointeur sur le nouveau Transformer créé
 * \remarks Dans ce cas, les data sont passées par leur noms
 */
CDataTransformer *CDataTransformerFactory::createInstance(QString data_in_name, QString data_out_name, CDataManager *data_mgr, QString event_connection, QString algo, QString params, QString description, QObject *parent)
{
    QString algo_simplified = algo.simplified().toLower();

    if (algo_simplified.contains("direct"))         return new CDataTransformer_Direct(data_in_name, data_out_name, data_mgr, event_connection, params, description, parent);
    if (algo_simplified.contains("polynom"))        return new CDataTransformer_Polynom(data_in_name, data_out_name, data_mgr, event_connection, params, description, parent);
    if (algo_simplified.contains("min"))            return new CDataTransformer_Min(data_in_name, data_out_name, data_mgr, event_connection, params, description, parent);
    if (algo_simplified.contains("max"))            return new CDataTransformer_Max(data_in_name, data_out_name, data_mgr, event_connection, params, description, parent);
    if (algo_simplified.contains("sin"))            return new CDataTransformer_Sinus(data_in_name, data_out_name, data_mgr, event_connection, params, description, parent);
    if (algo_simplified.contains("mean"))           return new CDataTransformer_Mean(data_in_name, data_out_name, data_mgr, event_connection, params, description, parent);
    if (algo_simplified.contains("mul") && algo_simplified.contains("2d"))  return new CDataTransformer_Multiply2D(data_in_name, data_out_name, data_mgr, event_connection, params, description, parent);
    if (algo_simplified.contains("add") && algo_simplified.contains("2d"))  return new CDataTransformer_Addition2D(data_in_name, data_out_name, data_mgr, event_connection, params, description, parent);

    return Q_NULLPTR;
}

// ______________________________________________________________________
/*!
 * \brief Renvoie la liste des Transformers possibles
 * \return la liste
 */
QStringList CDataTransformerFactory::getAvailableAlgo()
{
    QStringList lst;
    lst << CDataTransformer_Direct().getShortAlgoName()
        << CDataTransformer_Polynom().getShortAlgoName()
        << CDataTransformer_Min().getShortAlgoName()
        << CDataTransformer_Max().getShortAlgoName()
        << CDataTransformer_Sinus().getShortAlgoName()
        << CDataTransformer_Mean().getShortAlgoName()
        << CDataTransformer_Multiply2D().getShortAlgoName()
        << CDataTransformer_Addition2D().getShortAlgoName();
    return lst;
}

// ______________________________________________________________________
/*!
 * \brief Renvoie la liste des paramètres pour un algo donné
 * \param algo l'algo
 * \return la liste des paramètres
 */
QString CDataTransformerFactory::getParamsTemplate(QString algo)
{
    QString algo_simplified = algo.simplified().toLower();
    if (algo_simplified.contains("direct"))         return CDataTransformer_Direct().getParamsTemplate();
    if (algo_simplified.contains("polynom"))        return CDataTransformer_Polynom().getParamsTemplate();
    if (algo_simplified.contains("min"))            return CDataTransformer_Min().getParamsTemplate();
    if (algo_simplified.contains("max"))            return CDataTransformer_Max().getParamsTemplate();
    if (algo_simplified.contains("sin"))            return CDataTransformer_Sinus().getParamsTemplate();
    if (algo_simplified.contains("mean"))           return CDataTransformer_Mean().getParamsTemplate();
    if (algo_simplified.contains("mul") && algo_simplified.contains("2d"))  return CDataTransformer_Multiply2D().getParamsTemplate();
    if (algo_simplified.contains("add") && algo_simplified.contains("2d"))  return CDataTransformer_Addition2D().getParamsTemplate();

    return "";
}

// ______________________________________________________________________
/*!
 * \brief Renvoie la formule pour une algo donné
 * \param algo l'algo
 * \return la formule
 */
QString CDataTransformerFactory::getFormula(QString algo)
{
    QString algo_simplified = algo.simplified().toLower();
    if (algo_simplified.contains("direct"))         return CDataTransformer_Direct().getFormula();
    if (algo_simplified.contains("polynom"))        return CDataTransformer_Polynom().getFormula();
    if (algo_simplified.contains("min"))            return CDataTransformer_Min().getFormula();
    if (algo_simplified.contains("max"))            return CDataTransformer_Max().getFormula();
    if (algo_simplified.contains("sin"))            return CDataTransformer_Sinus().getFormula();
    if (algo_simplified.contains("mean"))            return CDataTransformer_Mean().getFormula();
    if (algo_simplified.contains("mul") && algo_simplified.contains("2d"))  return CDataTransformer_Multiply2D().getFormula();
    if (algo_simplified.contains("add") && algo_simplified.contains("2d"))  return CDataTransformer_Addition2D().getFormula();

    return "";
}

// ______________________________________________________________________
/*!
 * \brief Renvoie l'aide pour une algo donné
 * \param algo l'algo
 * \return l'aide
 */
QString CDataTransformerFactory::getHelp(QString algo)
{
    QString algo_simplified = algo.simplified().toLower();
    if (algo_simplified.contains("direct"))         return CDataTransformer_Direct().getHelp();
    if (algo_simplified.contains("polynom"))        return CDataTransformer_Polynom().getHelp();
    if (algo_simplified.contains("min"))            return CDataTransformer_Min().getHelp();
    if (algo_simplified.contains("max"))            return CDataTransformer_Max().getHelp();
    if (algo_simplified.contains("sin"))            return CDataTransformer_Sinus().getHelp();
    if (algo_simplified.contains("mean"))           return CDataTransformer_Mean().getHelp();
    if (algo_simplified.contains("mul") && algo_simplified.contains("2d"))  return CDataTransformer_Multiply2D().getHelp();
    if (algo_simplified.contains("add") && algo_simplified.contains("2d"))  return CDataTransformer_Addition2D().getHelp();

    return "";
}
