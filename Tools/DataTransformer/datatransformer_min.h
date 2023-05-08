#ifndef _DATATRANSFORMER_MIN_H_
#define _DATATRANSFORMER_MIN_H_

#include <QVector>

#include "datatransformer.h"

class CDataTransformer_Min : public CDataTransformer
{
    Q_OBJECT
public:
    explicit CDataTransformer_Min() {}
    explicit CDataTransformer_Min(QString datas_lst_in_str,
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
    double m_min;
};

#endif // _DATATRANSFORMER_MIN_H_
