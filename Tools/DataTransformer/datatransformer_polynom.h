#ifndef _DATATRANSFORMER_POLYNOM_H_
#define _DATATRANSFORMER_POLYNOM_H_

#include <QVector>

#include "datatransformer.h"

class CDataTransformer_Polynom : public CDataTransformer
{
    Q_OBJECT
public:
    explicit CDataTransformer_Polynom() {}
    explicit CDataTransformer_Polynom(QString datas_lst_in_str,
                                      QString data_out_str,
                                      CDataManager *data_mngr,
                                      QString event_connection,
                                      QString params,
                                      QString description,
                                      QObject *parent = nullptr);

    /*virtual*/ QString getShortAlgoName();
    /*virtual*/ QString getFormula();
    /*virtual*/ QString getHelp();
    /*virtual*/ QString getParamsTemplate();

private slots :
   void data_updated(QVariant val);

private :
    QVector<double> m_coefs;
    int m_order;
};

#endif // _DATATRANSFORMER_POLYNOM_H_
