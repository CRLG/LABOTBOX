#ifndef _EXTERNALCONTROLERCLIENT_H_
#define _EXTERNALCONTROLERCLIENT_H_

#include <QTcpSocket>

class CExternalControlerClient : public QObject
{
    Q_OBJECT
public:
    explicit CExternalControlerClient(QObject *parent = Q_NULLPTR);
    virtual ~CExternalControlerClient();

    bool open(char *hostname, int port);
    QVariant readData(QString dataname, int *error_code = Q_NULLPTR);
    void writeData(QString dataname, QVariant val, int *error_code = Q_NULLPTR);
    void createData(QString dataname, QVariant val, int *error_code = Q_NULLPTR);

private :
    QTcpSocket  m_socket;
    bool m_available_data;

    bool waitForResponse();
    QString analyzeReponse(QByteArray response, int *err_code);

public slots:
    void onReadyRead();
};

#endif // _EXTERNALCONTROLERCLIENT_H_
