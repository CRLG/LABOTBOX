#ifndef _DATATRANSFORMER_SINUS_H_
#define _DATATRANSFORMER_SINUS_H_

#include <QVector>

#include "datatransformer.h"

class CDataTransformer_Sinus : public CDataTransformer
{
    Q_OBJECT
public:
    explicit CDataTransformer_Sinus() {}
    explicit CDataTransformer_Sinus(QString datas_lst_in_str,
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
    double m_coefs_A;
    double m_coefs_a;
    double m_coefs_b;
};

#endif // _DATATRANSFORMER_SINUS_H_
