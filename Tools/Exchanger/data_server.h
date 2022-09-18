#ifndef DATA_SERVER_H
#define DATA_SERVER_H

#include "exchangerserver.h"
#include "exchanger_data.h"

class DataServer : public CExchangerServer
{
    Q_OBJECT
public:
    explicit DataServer(CExchangerServer *parent = nullptr);

    CExchangerData m_exchanger;
};

#endif // DATA_SERVER_H
