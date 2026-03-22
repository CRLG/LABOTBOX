/*! \file CHILEngine.h
 * Moteur d'exécution HIL (Hardware In the Loop) pour BlockBotLab.
 *
 * Reçoit la description JSON d'un état (actions + transitions) depuis BlockBot,
 * exécute les actions via le DataManager et évalue les transitions (convergence,
 * timeout, attente) pour progresser d'état en état jusqu'à FIN_MISSION ou arrêt.
 *
 * Inspiré fidèlement de CActuatorSequencer::Slot_Play() pour la mécanique
 * d'envoi des actions et la séquence d'arrêt de sécurité.
 */
#ifndef CHILENGINE_H
#define CHILENGINE_H

#include <QObject>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

class CApplication;

/*! @brief Moteur d'exécution HIL pour rejouer une machine à états expert
 *         depuis LaBotBox sans compilation STM32.
 *
 *  Cycle de vie :
 *    1. setApplication() une seule fois à l'init
 *    2. startFromState(nomEtat) pour lancer le HIL
 *    3. feedState(json) à chaque réponse de BlockBot
 *    4. stop() pour arrêter manuellement
 *
 *  Signaux émis :
 *    - requestState(nomEtat) : demande à CBlockBotLab de récupérer l'état suivant via JS
 *    - stateChanged(nomEtat) : feedback pour le surlignage Blockly et le log
 *    - hilFinished()         : fin de mission ou arrêt — déclenche l'arrêt de sécurité
 *    - logMessage(msg)       : message formaté [HIL] pour printView
 */
class CHILEngine : public QObject
{
    Q_OBJECT

public:
    explicit CHILEngine(QObject *parent = nullptr);

    //! Associe l'application (accès au DataManager). Appelé une seule fois à l'init.
    void setApplication(CApplication *application);

    //! Lance le HIL depuis un état de départ. Émet requestState() pour obtenir sa description.
    void startFromState(const QString &nomEtat);

    //! Reçoit la description JSON d'un état (réponse de BlockBot à get_hil_state).
    //! Parse les actions, les exécute, puis arme les transitions.
    void feedState(const QString &stateJson);

    //! Exécute une action unique (pas de transitions). Émet hilFinished() après exécution.
    void executeSingleAction(const QString &actionJson);

    //! Arrête le HIL proprement (désarme les timers, arrêt de sécurité).
    void stop();

    //! Retourne true si le moteur HIL est en cours d'exécution.
    bool isRunning() const { return m_running; }

signals:
    //! Demande à CBlockBotLab de récupérer la description de l'état nomEtat via JS.
    void requestState(const QString &nomEtat);

    //! Émis à chaque entrée dans un nouvel état — pour le surlignage Blockly.
    void stateChanged(const QString &nomEtat);

    //! Émis en fin de mission (FIN_MISSION) ou après un arrêt utilisateur.
    void hilFinished();

    //! Message formaté [HIL] pour affichage dans printView.
    void logMessage(const QString &msg);

private:
    //! Exécute toutes les actions d'un état (tableau JSON "actions").
    void executeActions(const QJsonArray &actions);

    //! Exécute une seule action décrite par un objet JSON.
    void executeOneAction(const QJsonObject &action);

    //! Arme les transitions d'un état (tableau JSON "transitions").
    //! La première condition vraie ou le premier timeout déclenche le changement d'état.
    void armTransitions(const QJsonArray &transitions);

    //! Désarme tous les timers de transition actifs et déconnecte les signaux DataManager.
    void disarmTransitions();

    //! Déclenche la transition vers l'état cible.
    //! Désarme les transitions en cours, puis demande l'état suivant ou termine.
    void triggerTransition(const QString &etatCible, const QString &raison);

    //! Séquence d'arrêt de sécurité — reproduit fidèlement CActuatorSequencer::Slot_Play().
    void safetyShutdown();

    // ── Envoi des actions via DataManager ────────────────────────────
    void sendServo(int id, int pos, int speed);
    void sendServoAX(int id, int pos);
    void sendMotor(int id, int pwm);
    void sendSwitch(int id, int etat);
    void sendMovementXY(float x, float y);
    void sendMovementXYT(float x, float y, float theta);
    void sendMovementDA(float distance, float angle);

    CApplication *m_application = nullptr;
    bool m_running = false;
    QString m_currentState;

    //! Timers de timeout pour chaque transition armée
    QList<QTimer*> m_transitionTimers;

    //! Connexion au signal dataChanged du DataManager pour évaluer les convergences
    QMetaObject::Connection m_convergenceConnection;
    QMetaObject::Connection m_convergenceRapideConnection;

    //! Informations sur les transitions en attente de convergence
    struct PendingTransition {
        QString type;       // "convergence_expert" ou "convergence_rapide_expert"
        QString etatCible;
        int timeoutMs;
    };
    QList<PendingTransition> m_pendingTransitions;
};

#endif // CHILENGINE_H
