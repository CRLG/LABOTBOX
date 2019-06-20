#include <QCoreApplication>
#include <QHostAddress>
#include <QElapsedTimer>
#include <QStringList>
#include <QString>
#include "CExternalControlerClient.h"

CExternalControlerClient::CExternalControlerClient(QObject *parent)
    : QObject(parent)
{
}

CExternalControlerClient::~CExternalControlerClient()
{
}

// ______________________________________________________
void CExternalControlerClient::onReadyRead()
{
    // lève un flag qui sera exploité dans la méthode waitForResponse
    m_available_data = true;
}

// ______________________________________________________
bool CExternalControlerClient::waitForResponse()
{
    m_available_data = false;  // ce flag sera mis à "1" par le slot onReadyRead une fois la donnée reçue
    QElapsedTimer timer;
    timer.start();
    while ((m_available_data==false) && (timer.elapsed() < 300) ) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    }
    return (m_socket.bytesAvailable() != 0);
}

// ______________________________________________________
bool CExternalControlerClient::open(char *hostname, int port)
{
    if (m_socket.isOpen()) m_socket.close();
    m_socket.connectToHost(QHostAddress(hostname), port);
    if (!m_socket.waitForConnected(5000)){
            qDebug() << "Socket::connection() Error "<< m_socket.errorString();
            return false;
    }
    connect(&m_socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    if (waitForResponse()) {
        qDebug() << m_socket.readAll();
    }
    return true;
}

// ______________________________________________________
QString CExternalControlerClient::analyzeReponse(QByteArray response, int *err_code)
{
    QString str = response;
    //qDebug() << str;
    QStringList str_list = str.split("|");
    if (str_list.size() > 1) {
        *err_code = str_list.at(0).trimmed().toInt();
        return str_list.at(1);
    }
    else if (str_list.size() == 1) {
        *err_code = str_list.at(0).trimmed().toInt();
    }
    return "";
}


// ______________________________________________________
QVariant CExternalControlerClient::readData(QString dataname, int *error_code)
{
    QString cmd = "getData(" + dataname + ")";
    int err;
    QVariant ret ="";
    m_socket.write(cmd.toStdString().c_str());
    if (waitForResponse()) {
        ret = analyzeReponse(m_socket.readAll(), &err);
        if (error_code) *error_code = err;
    }
    return ret;
}

// ______________________________________________________
void  CExternalControlerClient::writeData(QString dataname, QVariant val, int *error_code)
{
    QString cmd = "setData(" + dataname + "," + val.toString() + ")";
    int err;
    m_socket.write(cmd.toStdString().c_str());
    if (waitForResponse()) {
        analyzeReponse(m_socket.readAll(), &err);
        if (error_code) *error_code = err;
    }
}

// ______________________________________________________
void  CExternalControlerClient::createData(QString dataname, QVariant val, int *error_code)
{
    QString cmd = "createData(" + dataname + "," + val.toString() + ")";
    int err;
    m_socket.write(cmd.toStdString().c_str());
    if (waitForResponse()) {
        analyzeReponse(m_socket.readAll(), &err);
        if (error_code) *error_code = err;
    }
}
