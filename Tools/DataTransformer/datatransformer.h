#ifndef DATATRANSFORMER_H
#define DATATRANSFORMER_H

#include <QObject>
#include <QVector>
#include <CData.h>


class CDataManager;
//!
//! \brief Classe de base pour toutes les transformations d'une data en une autre
//!
class CDataTransformer : public QObject
{
    Q_OBJECT
public:
    explicit CDataTransformer() {}
    explicit CDataTransformer(QString datas_lst_in_str,
                              QString data_out_str,
                              CDataManager *data_mngr,
                              QString event_connection,
                              QString params,
                              QString description,
                              QObject *parent = nullptr);

    virtual QString getShortAlgoName() = 0;
    virtual QString getFormula() = 0;
    virtual QString getHelp() = 0;
    virtual QString getParamsTemplate() = 0;

    virtual QString getDescription();
    virtual QStringList getInputDataName();
    virtual QString getOutputDataName();
    virtual QString getEventConnection();
    virtual QString getParams();

protected :
    CData *m_data_out;
    QVector <CData*> m_data_lst_in;
    QStringList m_params_lst_in;
    QString m_event_connection;
    QString m_params;
    QString m_description;

    QStringList simplifyParams(QString params);
    const QString PARAMS_START_TAG = "{";
    const QString PARAMS_END_TAG = "}";
    const QString PARAMS_SEPARATOR = ",";

    typedef enum {
        DATA_UPDATED = 0,
        DATA_CHANGED,
    }tEventConnectionType;
    tEventConnectionType interpretEventConnection(QString event_connection);

signals:

public slots:
};

#endif // DATATRANSFORMER_H
