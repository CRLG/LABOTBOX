#ifndef ROBOTICSHTTPSERVER_H
#define ROBOTICSHTTPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QTextStream>

/**
 * @brief Serveur HTTP léger pour recevoir les requêtes POST de BlockBot
 *
 * Cette classe implémente un serveur HTTP basique utilisant QTcpServer pour
 * communiquer avec l'interface BlockBot. Elle gère spécifiquement les requêtes
 * POST contenant les données qu'on souhaite envoyer de BlockBot et les requêtes CORS.
 */
class RoboticsHttpServer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Constructeur du serveur HTTP
     * @param parent Objet parent Qt
     *
     * Initialise le serveur TCP et configure les connexions de signaux
     */
    explicit RoboticsHttpServer(QObject* parent = nullptr);

    /**
     * @brief Démarre l'écoute du serveur sur un port donné
     * @param port Port d'écoute (par défaut 3001)
     * @return true si le serveur a démarré avec succès, false sinon
     *
     * Lance le serveur en mode écoute sur localhost pour accepter les connexions de BlockBot
     */
    bool listen(int port = 3001);

private slots:
    /**
     * @brief Gère les nouvelles connexions TCP entrantes
     *
     * Appelé automatiquement quand une nouvelle connexion arrive.
     * Configure les signaux pour traiter les données reçues.
     */
    void handleNewConnection();

    /**
     * @brief Traite une requête HTTP complète
     * @param socket Socket TCP de la connexion cliente
     *
     * Parse la requête HTTP reçue et dirige vers le bon gestionnaire
     * selon le type de requête (POST, OPTIONS, etc.)
     */
    void handleRequest(QTcpSocket* socket);

private:
    /**
     * @brief Traite les requêtes POST /upload contenant les données de BlockBot
     * @param socket Socket pour la réponse
     * @param request Texte complet de la requête HTTP
     *
     * Extrait le JSON contenant les données envoyées par BlockBot,
     * les traite et renvoie une réponse de succès/échec.
     */
    void processBlockBotRequest(QTcpSocket* socket, const QString& request);

    /**
     * @brief Gère les requêtes OPTIONS pour CORS (Cross-Origin Resource Sharing)
     * @param socket Socket pour la réponse
     *
     * Répond aux requêtes de pré-validation CORS nécessaires pour permettre
     * aux requêtes JavaScript de BlockBot d'accéder au serveur.
     */
    void handleCorsOptions(QTcpSocket* socket);

    /**
     * @brief Envoie une réponse HTTP de succès au format JSON
     * @param socket Socket pour la réponse
     *
     * Construit et envoie une réponse HTTP 200 OK avec un JSON
     * indiquant que l'opération s'est déroulée avec succès.
     */
    void sendSuccess(QTcpSocket* socket);

    /**
     * @brief Envoie une réponse HTTP d'erreur au format JSON
     * @param socket Socket pour la réponse
     * @param error Message d'erreur à inclure
     *
     * Construit et envoie une réponse HTTP 500 avec un JSON
     * contenant le message d'erreur détaillé.
     */
    void sendError(QTcpSocket* socket, const QString& error);

    /**
     * @brief Envoie une réponse HTTP 404 Not Found
     * @param socket Socket pour la réponse
     *
     * Utilisé quand la route demandée n'existe pas sur le serveur.
     */
    void send404(QTcpSocket* socket);


private:
    QTcpServer* server;  ///< Serveur TCP principal pour l'écoute des connexions

signals :
    void processData(const QString& code);
};

#endif // ROBOTICSHTTPSERVER_H
