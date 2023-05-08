#ifndef _DATATRANSFORMER_ADDITION2D_H_
#define _DATATRANSFORMER_ADDITION2D_H_

#include <QVector>
#include <CDataManager.h>
#include "datatransformer.h"

class CDataTransformer_Addition2D : public CDataTransformer
{
    Q_OBJECT
public:
    explicit CDataTransformer_Addition2D() {}
    explicit CDataTransformer_Addition2D(QString datas_lst_in_str,
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
   // data_out = coefs (a1*data_in1 + b1) * (a2*data_in2 + b2)
    void data_updated();

private :
   double m_a1;
   double m_b1;
   double m_a2;
   double m_b2;
};

#endif // _DATATRANSFORMER_ADDITION2D_H_
