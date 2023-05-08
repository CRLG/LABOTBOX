#ifndef _DATATRANSFORMER_MEAN_H_
#define _DATATRANSFORMER_MEAN_H_

#include <QQueue>

#include "datatransformer.h"

class CDataTransformer_Mean : public CDataTransformer
{
    Q_OBJECT
public:
    explicit CDataTransformer_Mean() {}
    explicit CDataTransformer_Mean(QString datas_lst_in_str,
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
    int m_sample_count;
    QQueue<double> m_fifo;
};

#endif // _DATATRANSFORMER_MEAN_H_
