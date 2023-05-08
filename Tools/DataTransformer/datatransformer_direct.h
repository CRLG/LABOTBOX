#ifndef _DATATRANSFORMER_DIRECT_H_
#define _DATATRANSFORMER_DIRECT_H_

#include "datatransformer.h"

class CDataTransformer_Direct : public CDataTransformer
{
    Q_OBJECT
public:
    explicit CDataTransformer_Direct() {}
    explicit CDataTransformer_Direct(QString datas_lst_in_str,
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
};

#endif // _DATATRANSFORMER_DIRECT_H_
