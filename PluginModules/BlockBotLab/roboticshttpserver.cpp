#include "roboticshttpserver.h"
#include <QDebug>
#include <QHostAddress>

RoboticsHttpServer::RoboticsHttpServer(QObject* parent)
    : QObject(parent)
{
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection,
            this, &RoboticsHttpServer::handleNewConnection);
}

bool RoboticsHttpServer::listen(int port)
{
    bool success = server->listen(QHostAddress::LocalHost, port);
    if (success) {
        qDebug() << "Serveur HTTP démarré sur le port" << port;
    } else {
        qDebug() << "Erreur lors du démarrage du serveur:" << server->errorString();
    }
    return success;
}

void RoboticsHttpServer::handleNewConnection()
{
    QTcpSocket* socket = server->nextPendingConnection();
    connect(socket, &QTcpSocket::readyRead,
            this, [this, socket]() { handleRequest(socket); });
    connect(socket, &QTcpSocket::disconnected,
            socket, &QTcpSocket::deleteLater);

    qDebug() << "Nouvelle connexion de BlockBot reçue";
}

void RoboticsHttpServer::handleRequest(QTcpSocket* socket)
{
    QByteArray data = socket->readAll();
    QString request = QString::fromUtf8(data);

    // Pour le debug
    //qDebug() << "Requête reçue:" << request.left(100) << "..."; // Afficher les 100 premiers caractères

    // Router les requêtes selon leur type
    if (request.startsWith("POST /data")) {
        processBlockBotRequest(socket, request);
    } else if (request.startsWith("OPTIONS")) {
        handleCorsOptions(socket);
    } else {
        qDebug() << "Route non trouvée";
        send404(socket);
    }
}

void RoboticsHttpServer::processBlockBotRequest(QTcpSocket* socket, const QString& request)
{
    // Extraire le JSON du body de la requête HTTP
    int bodyStart = request.indexOf("\r\n\r\n");
    if (bodyStart == -1) {
        qDebug() << "Requête HTTP malformée: pas de body trouvé";
        sendError(socket, "Invalid request format");
        return;
    }

    QString jsonBody = request.mid(bodyStart + 4);
    //pour le debug
    //qDebug() << "JSON reçu:" << jsonBody;

    // Parser le JSON
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonBody.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "Erreur de parsing JSON:" << parseError.errorString();
        sendError(socket, "Invalid JSON format");
        return;
    }

    QJsonObject jsonObject = doc.object();

    // Extraire le code généré (format simple: {"code": "..."})
    QString receivedBlockBotData = jsonObject["code"].toString();

    if (receivedBlockBotData.isEmpty()) {
        qDebug() << "Aucun code reçu de BlockBot";
        sendError(socket, "No code received");
        return;
    }

    //Pour le debug (pour le cas actuel ou seul le code est envoyé)
    //qDebug() << "Code généré reçu, taille:" << receivedBlockBotData.length() << "caractères";
    //qDebug() << "Aperçu du code:" << receivedBlockBotData.left(100) << "...";

    //Traite les données brutes reçues dans la couche supérieure
    emit processData(receivedBlockBotData);
    sendSuccess(socket);  // s'il y a des erreurs dans le bloc de réponse, elles seront traitées et affichées par la couche supérieure
}

void RoboticsHttpServer::handleCorsOptions(QTcpSocket* socket)
{
    qDebug() << "Requête CORS OPTIONS reçue";

    QString response = "HTTP/1.1 200 OK\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "Access-Control-Allow-Methods: POST, OPTIONS\r\n"
                      "Access-Control-Allow-Headers: Content-Type\r\n"
                      "Content-Length: 0\r\n\r\n";

    socket->write(response.toUtf8());
    socket->disconnectFromHost();
}

void RoboticsHttpServer::sendSuccess(QTcpSocket* socket)
{
    QJsonObject response;
    response["status"] = "success";
    response["message"] = "Files saved successfully";
    QByteArray jsonResponse = QJsonDocument(response).toJson();

    QString httpResponse = QString("HTTP/1.1 200 OK\r\n"
                                 "Content-Type: application/json\r\n"
                                 "Access-Control-Allow-Origin: *\r\n"
                                 "Content-Length: %1\r\n\r\n%2")
                                 .arg(jsonResponse.size())
                                 .arg(QString::fromUtf8(jsonResponse));

    socket->write(httpResponse.toUtf8());
    socket->disconnectFromHost();
}

void RoboticsHttpServer::sendError(QTcpSocket* socket, const QString& error)
{
    QJsonObject response;
    response["status"] = "error";
    response["message"] = error;
    QByteArray jsonResponse = QJsonDocument(response).toJson();

    QString httpResponse = QString("HTTP/1.1 500 Internal Server Error\r\n"
                                 "Content-Type: application/json\r\n"
                                 "Access-Control-Allow-Origin: *\r\n"
                                 "Content-Length: %1\r\n\r\n%2")
                                 .arg(jsonResponse.size())
                                 .arg(QString::fromUtf8(jsonResponse));

    socket->write(httpResponse.toUtf8());
    socket->disconnectFromHost();
}

void RoboticsHttpServer::send404(QTcpSocket* socket)
{
    QString response = "HTTP/1.1 404 Not Found\r\n"
                      "Access-Control-Allow-Origin: *\r\n"
                      "Content-Length: 0\r\n\r\n";

    socket->write(response.toUtf8());
    socket->disconnectFromHost();
}
