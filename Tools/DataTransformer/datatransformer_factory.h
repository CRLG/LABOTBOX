#ifndef DATATRANSFORMER_FACTORY_H
#define DATATRANSFORMER_FACTORY_H

#include <QObject>

class CDataTransformer;
class CData;
class CDataManager;

class CDataTransformerFactory
{
public:
    CDataTransformerFactory();

    static CDataTransformer *createInstance(QString data_in_name,
                                            QString data_out_name,
                                            CDataManager *data_mgr,
                                            QString event_connection,
                                            QString algo,
                                            QString params,
                                            QString description,
                                            QObject *parent = nullptr);

    static QStringList getAvailableAlgo();
    static QString getParamsTemplate(QString algo);
    static QString getFormula(QString algo);
    static QString getHelp(QString algo);

};

#endif // DATATRANSFORMER_FACTORY_H
