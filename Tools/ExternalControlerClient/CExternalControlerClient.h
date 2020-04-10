#ifndef _EXTERNALCONTROLERCLIENT_H_
#define _EXTERNALCONTROLERCLIENT_H_

#include <QTcpSocket>

// Cette classe est un client TCP pour le module serveur TCP ExternalControler
// Le client permet de dialoguer avec le serveur ExternalControler d'un autre outil

// Utilisation :
// #include "CExternalControlerClient.h"
// ....
// Création d'une instance du client
//   CExternalControlerClient m_external_controler_client;

// Initialisation du client :
// Indiquer l'addresse IP et le port du serveur
//   m_external_controler_client.open("127.0.0.1", 1234);

// API du client :
//   int err;
//   Lecture d'une data distante
//   qDebug() << m_external_controler_client.readData("PosY_robot", &err).toString();
//   if (err != 0) qDebug() << "Error code" << err;

//   Ecriture d'une data distante
//   m_external_controler_client.writeData("PosY_robot", "136.9", &err);
//   if (err != 0) qDebug() << "Error code" << err;

//   Création d'une nouvelle data dans le datamanager distant
//   m_external_controler_client.createData("new_data", "Hello 123", &err);
//   if (err != 0) qDebug() << "Error code" << err;

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
    bool isConnected();
    void close();

private :
    QTcpSocket  m_socket;
    bool m_available_data;

    bool waitForResponse();
    QString analyzeReponse(QByteArray response, int *err_code);

public slots:
    void onReadyRead();
signals :
    void connected();
    void disconnected();
};

#endif // _EXTERNALCONTROLERCLIENT_H_
