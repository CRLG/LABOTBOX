/*! \file CHILEngine.cpp
 * Implémentation du moteur HIL (Hardware In the Loop) pour BlockBotLab.
 *
 * Exécute les actions via le DataManager et évalue les transitions
 * (convergence asservissement, convergence rapide, timeout, attente pure)
 * pour progresser d'état en état.
 *
 * Les patterns d'envoi d'actions (paires TxSync) reproduisent fidèlement
 * ceux de CActuatorSequencer::Slot_Play().
 */
#include "CHILEngine.h"
#include "CApplication.h"
#include "CDataManager.h"
#include "CData.h"

// Valeur de convergence asservissement (cf CActuatorSequencer.h)
static const int cCONVERGENCE_OK_VALUE = 1;
// Commande servo AX position (cf CActuatorElectrobot.h)
static const int cSERVO_AX_POSITION_VALUE = 0;
// Délai minimal (ms) avant d'écouter la convergence, pour laisser le temps
// à l'asservissement de démarrer le mouvement et de quitter l'état "convergé".
// Équivalent au cmptTe > 10 de CActuatorSequencer (~400ms à 25Hz).
static const int CONVERGENCE_GUARD_DELAY_MS = 500;


// _____________________________________________________________________
CHILEngine::CHILEngine(QObject *parent)
    : QObject(parent)
{
}

// _____________________________________________________________________
void CHILEngine::setApplication(CApplication *application)
{
    m_application = application;
}

// _____________________________________________________________________
/*!
 * Lance le HIL depuis un état de départ.
 * Émet requestState() pour que CBlockBotLab demande la description
 * de l'état à BlockBot via QWebChannel.
 */
void CHILEngine::startFromState(const QString &nomEtat)
{
    if (m_running) {
        emit logMessage("[HIL] Deja en cours d'execution, arret prealable");
        stop();
    }

    m_running = true;
    m_currentState = nomEtat;

    emit logMessage(QString("[HIL] DEMARRAGE depuis %1").arg(nomEtat));

    // Demande la description de l'état à BlockBot
    emit requestState(nomEtat);
}

// _____________________________________________________________________
/*!
 * Reçoit la description JSON d'un état depuis BlockBot.
 * Parse les actions, les exécute immédiatement, puis arme les transitions.
 *
 * Format attendu :
 * {
 *   "nom": "ETAT_1",
 *   "actions": [ { "type": "set_servo_expert", ... }, ... ],
 *   "transitions": [ { "type": "convergence_expert", "timeout_ms": 5000, "etat_cible": "ETAT_2" }, ... ]
 * }
 */
void CHILEngine::feedState(const QString &stateJson)
{
    if (!m_running) return;

    if (stateJson.isEmpty()) {
        emit logMessage("[HIL] Etat vide recu — arret");
        stop();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(stateJson.toUtf8());
    if (!doc.isObject()) {
        emit logMessage("[HIL] JSON invalide — arret");
        stop();
        return;
    }

    QJsonObject stateObj = doc.object();
    QString nomEtat = stateObj["nom"].toString();
    QJsonArray actions = stateObj["actions"].toArray();
    QJsonArray transitions = stateObj["transitions"].toArray();

    // Surlignage Blockly
    emit stateChanged(nomEtat);

    emit logMessage(QString("[HIL] ETAT %1 : %2 action(s), %3 transition(s)")
                    .arg(nomEtat)
                    .arg(actions.size())
                    .arg(transitions.size()));

    // Exécution des actions
    executeActions(actions);

    // Armement des transitions
    if (transitions.isEmpty()) {
        emit logMessage(QString("[HIL]   Aucune transition — reste dans %1").arg(nomEtat));
    } else {
        emit logMessage("[HIL]   ATTENTE transitions...");
        armTransitions(transitions);
    }
}

// _____________________________________________________________________
/*!
 * Exécute une action unique (mode actionPlayOnlyOneHIL).
 * Pas de transition, pas de changement d'état.
 */
void CHILEngine::executeSingleAction(const QString &actionJson)
{
    if (actionJson.isEmpty()) {
        emit logMessage("[HIL] Aucune action selectionnee");
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(actionJson.toUtf8());
    if (!doc.isObject()) {
        emit logMessage("[HIL] JSON action invalide");
        return;
    }

    QJsonObject action = doc.object();
    QString type = action["type"].toString();

    if (action["ignored"].toBool()) {
        emit logMessage(QString("[HIL] ACTION UNIQUE IGNOREE : %1 (non supporte en HIL)").arg(type));
        return;
    }

    emit logMessage(QString("[HIL] ACTION UNIQUE %1").arg(type));
    executeOneAction(action);
}

// _____________________________________________________________________
void CHILEngine::executeActions(const QJsonArray &actions)
{
    for (int i = 0; i < actions.size(); i++) {
        QJsonObject action = actions[i].toObject();
        executeOneAction(action);
    }
}

// _____________________________________________________________________
/*!
 * Exécute une action via le DataManager.
 * Les patterns d'envoi (paires TxSync) reproduisent CActuatorSequencer::Slot_Play().
 */
void CHILEngine::executeOneAction(const QJsonObject &action)
{
    if (!m_application) return;

    QString type = action["type"].toString();

    // Les blocs action_perso et inconnus sont ignorés
    if (action["ignored"].toBool()) {
        emit logMessage(QString("[HIL]   ACTION IGNOREE : %1 (code C libre non supporte en HIL)").arg(type));
        return;
    }

    if (type == "set_servo_expert") {
        int id    = action["id"].toInt();
        int pos   = action["pos"].toInt();
        int speed = action["speed"].toInt(255);
        sendServo(id, pos, speed);
        emit logMessage(QString("[HIL]   ACTION set_servo_expert : servo=%1 pos=%2 vit=%3").arg(id).arg(pos).arg(speed));
    }
    else if (type == "set_ax_expert") {
        int id  = action["id"].toInt();
        int pos = action["pos"].toInt();
        sendServoAX(id, pos);
        emit logMessage(QString("[HIL]   ACTION set_ax_expert : ax=%1 pos=%2").arg(id).arg(pos));
    }
    else if (type == "set_motor") {
        int id  = action["id"].toInt();
        int pwm = action["pwm"].toInt();
        sendMotor(id, pwm);
        emit logMessage(QString("[HIL]   ACTION set_motor : moteur=%1 pwm=%2").arg(id).arg(pwm));
    }
    else if (type == "set_switch") {
        int id   = action["id"].toInt();
        int etat = action["etat"].toInt();
        sendSwitch(id, etat);
        emit logMessage(QString("[HIL]   ACTION set_switch : switch=%1 etat=%2").arg(id).arg(etat));
    }
    else if (type == "set_pos") {
        QString mode = action["mode"].toString();
        float val1 = static_cast<float>(action["val1"].toDouble());
        float val2 = static_cast<float>(action["val2"].toDouble());
        float val3 = static_cast<float>(action["val3"].toDouble());

        if (mode == "XY") {
            sendMovementXY(val1, val2);
            emit logMessage(QString("[HIL]   ACTION set_pos : mode=XY x=%1 y=%2").arg(val1).arg(val2));
        } else if (mode == "XYT") {
            sendMovementXYT(val1, val2, val3);
            emit logMessage(QString("[HIL]   ACTION set_pos : mode=XYT x=%1 y=%2 theta=%3").arg(val1).arg(val2).arg(val3));
        } else if (mode == "DA") {
            sendMovementDA(val1, val3);
            emit logMessage(QString("[HIL]   ACTION set_pos : mode=DA dist=%1 angle=%2").arg(val1).arg(val3));
        }
    }
    else if (type == "set_pos_static") {
        float val1 = static_cast<float>(action["val1"].toDouble());
        float val2 = static_cast<float>(action["val2"].toDouble());
        float val3 = static_cast<float>(action["val3"].toDouble());
        sendMovementXYT(val1, val2, val3);
        emit logMessage(QString("[HIL]   ACTION set_pos_static : x=%1 y=%2 theta=%3").arg(val1).arg(val2).arg(val3));
    }
    else {
        emit logMessage(QString("[HIL]   ACTION IGNOREE : type inconnu %1").arg(type));
    }
}

// _____________________________________________________________________
/*!
 * Arme les transitions d'un état.
 *
 * Pour chaque transition :
 *   - attendre_expert       → timer pur (timeout_ms)
 *   - convergence_expert    → écoute "Convergence" DataManager + timer timeout
 *   - convergence_rapide_expert → écoute "convergence_rapide" DataManager + timer timeout
 *   - transition_perso      → ignorée
 *
 * La première condition vraie déclenche le changement d'état.
 */
void CHILEngine::armTransitions(const QJsonArray &transitions)
{
    // Nettoyage préalable
    disarmTransitions();

    for (int i = 0; i < transitions.size(); i++) {
        QJsonObject trans = transitions[i].toObject();
        QString type = trans["type"].toString();

        if (trans["ignored"].toBool()) {
            emit logMessage(QString("[HIL]   TRANSITION IGNOREE : %1 (code C libre non supporte en HIL)").arg(type));
            continue;
        }

        int timeoutMs = trans["timeout_ms"].toInt(0);
        QString etatCible = trans["etat_cible"].toString("FIN_MISSION");

        if (type == "attendre_expert") {
            // Transition pure timeout — pas de condition DataManager
            if (timeoutMs > 0) {
                QTimer *timer = new QTimer(this);
                timer->setSingleShot(true);
                connect(timer, &QTimer::timeout, this, [this, etatCible]() {
                    triggerTransition(etatCible, "timeout");
                });
                timer->start(timeoutMs);
                m_transitionTimers.append(timer);
            }
        }
        else if (type == "convergence_expert") {
            // Écoute la convergence asservissement + timeout de sécurité.
            // La connexion au signal DataManager est différée de CONVERGENCE_GUARD_DELAY_MS
            // pour laisser le temps à l'asservissement de démarrer le mouvement
            // (sinon la convergence résiduelle de l'état précédent déclenche immédiatement).
            PendingTransition pt;
            pt.type = type;
            pt.etatCible = etatCible;
            pt.timeoutMs = timeoutMs;
            m_pendingTransitions.append(pt);

            // Connexion différée au signal valueChanged de la donnée "Convergence"
            QTimer::singleShot(CONVERGENCE_GUARD_DELAY_MS, this, [this, etatCible]() {
                if (!m_running) return;
                CData *dataConvergence = m_application->m_data_center->getData("Convergence", true);
                if (dataConvergence) {
                    m_convergenceConnection = connect(dataConvergence, QOverload<QVariant>::of(&CData::valueChanged),
                        this, [this, etatCible](QVariant value) {
                            if (value.toInt() == cCONVERGENCE_OK_VALUE) {
                                triggerTransition(etatCible, "convergence");
                            }
                        });
                }
            });

            // Timer timeout de sécurité
            if (timeoutMs > 0) {
                QTimer *timer = new QTimer(this);
                timer->setSingleShot(true);
                connect(timer, &QTimer::timeout, this, [this, etatCible]() {
                    triggerTransition(etatCible, "timeout (convergence)");
                });
                timer->start(timeoutMs);
                m_transitionTimers.append(timer);
            }
        }
        else if (type == "convergence_rapide_expert") {
            // Écoute la convergence rapide + timeout de sécurité.
            // Même délai de garde que convergence_expert.
            PendingTransition pt;
            pt.type = type;
            pt.etatCible = etatCible;
            pt.timeoutMs = timeoutMs;
            m_pendingTransitions.append(pt);

            // Connexion différée au signal valueChanged de la donnée "convergence_rapide"
            QTimer::singleShot(CONVERGENCE_GUARD_DELAY_MS, this, [this, etatCible]() {
                if (!m_running) return;
                CData *dataConvRapide = m_application->m_data_center->getData("convergence_rapide", true);
                if (dataConvRapide) {
                    m_convergenceRapideConnection = connect(dataConvRapide, QOverload<QVariant>::of(&CData::valueChanged),
                        this, [this, etatCible](QVariant value) {
                            if (value.toInt() == 1) {
                                triggerTransition(etatCible, "convergence rapide");
                            }
                        });
                }
            });

            // Timer timeout de sécurité
            if (timeoutMs > 0) {
                QTimer *timer = new QTimer(this);
                timer->setSingleShot(true);
                connect(timer, &QTimer::timeout, this, [this, etatCible]() {
                    triggerTransition(etatCible, "timeout (convergence rapide)");
                });
                timer->start(timeoutMs);
                m_transitionTimers.append(timer);
            }
        }
    }
}

// _____________________________________________________________________
void CHILEngine::disarmTransitions()
{
    // Arrêt et suppression de tous les timers
    for (QTimer *timer : m_transitionTimers) {
        timer->stop();
        delete timer;
    }
    m_transitionTimers.clear();

    // Déconnexion des signaux DataManager
    if (m_convergenceConnection) {
        disconnect(m_convergenceConnection);
        m_convergenceConnection = QMetaObject::Connection();
    }
    if (m_convergenceRapideConnection) {
        disconnect(m_convergenceRapideConnection);
        m_convergenceRapideConnection = QMetaObject::Connection();
    }

    m_pendingTransitions.clear();
}

// _____________________________________________________________________
/*!
 * Déclenche la transition vers l'état cible.
 * Désarme toutes les transitions en cours avant de demander l'état suivant.
 */
void CHILEngine::triggerTransition(const QString &etatCible, const QString &raison)
{
    if (!m_running) return;

    // Désarmer les transitions de l'état courant
    disarmTransitions();

    emit logMessage(QString("[HIL]   TRANSITION : %1 -> %2").arg(raison).arg(etatCible));

    // FIN_MISSION → arrêt
    if (etatCible == "FIN_MISSION") {
        emit logMessage("[HIL] FIN_MISSION");
        stop();
        return;
    }

    // Passage à l'état suivant
    m_currentState = etatCible;
    emit requestState(etatCible);
}

// _____________________________________________________________________
void CHILEngine::stop()
{
    if (!m_running && m_transitionTimers.isEmpty()) return;

    bool wasRunning = m_running;
    m_running = false;

    disarmTransitions();
    safetyShutdown();

    if (wasRunning) {
        emit logMessage("[HIL] ARRET SECURITE effectue");
    }

    emit hilFinished();
}

// _____________________________________________________________________
/*!
 * Séquence d'arrêt de sécurité.
 * Reproduit fidèlement la séquence de CActuatorSequencer::Slot_Play() (lignes 1712-1738).
 */
void CHILEngine::safetyShutdown()
{
    if (!m_application) return;

    // Arrêt moteurs
    m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 1);
    m_application->m_data_center->write("cde_moteur_1", 0);
    m_application->m_data_center->write("cde_moteur_2", 0);
    m_application->m_data_center->write("cde_moteur_3", 0);
    m_application->m_data_center->write("cde_moteur_4", 0);
    m_application->m_data_center->write("cde_moteur_5", 0);
    m_application->m_data_center->write("cde_moteur_6", 0);
    m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 0);

    // Arrêt power switch
    m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 1);
    m_application->m_data_center->write("PowerSwitch_xt1", 0);
    m_application->m_data_center->write("PowerSwitch_xt2", 0);
    m_application->m_data_center->write("PowerSwitch_xt3", 0);
    m_application->m_data_center->write("PowerSwitch_xt4", 0);
    m_application->m_data_center->write("PowerSwitch_xt5", 0);
    m_application->m_data_center->write("PowerSwitch_xt6", 0);
    m_application->m_data_center->write("PowerSwitch_xt7", 0);
    m_application->m_data_center->write("PowerSwitch_xt8", 0);
    m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 0);

    // Arrêt mouvement manuel
    m_application->m_data_center->write("COMMANDE_MVT_MANUEL_TxSync", 1);
    m_application->m_data_center->write("PuissanceMotD", 0);
    m_application->m_data_center->write("PuissanceMotG", 0);
    m_application->m_data_center->write("COMMANDE_MVT_MANUEL_TxSync", 0);

    // Arrêt servos
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", 1);
    m_application->m_data_center->write("NumeroServoMoteur1", 50);
    m_application->m_data_center->write("PositionServoMoteur1", 10);
    m_application->m_data_center->write("VitesseServoMoteur1", 0);
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", 0);
}


// =====================================================================
//           ENVOI DES ACTIONS VIA DATAMANAGER (paires TxSync)
// =====================================================================

// _____________________________________________________________________
void CHILEngine::sendServo(int id, int pos, int speed)
{
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", true);
    m_application->m_data_center->write("NumeroServoMoteur1", id);
    m_application->m_data_center->write("PositionServoMoteur1", pos);
    m_application->m_data_center->write("VitesseServoMoteur1", speed);
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_TxSync", false);
}

// _____________________________________________________________________
void CHILEngine::sendServoAX(int id, int pos)
{
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", true);
    m_application->m_data_center->write("num_servo_ax", id);
    m_application->m_data_center->write("commande_ax", cSERVO_AX_POSITION_VALUE);
    m_application->m_data_center->write("valeur_commande_ax", pos);
    m_application->m_data_center->write("ELECTROBOT_CDE_SERVOS_AX_TxSync", false);
}

// _____________________________________________________________________
void CHILEngine::sendMotor(int id, int pwm)
{
    QString str_name = QString("cde_moteur_%1").arg(id);
    m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 1);
    m_application->m_data_center->write(str_name, pwm);
    m_application->m_data_center->write("ELECTROBOT_CDE_MOTEURS_TxSync", 0);
}

// _____________________________________________________________________
void CHILEngine::sendSwitch(int id, int etat)
{
    // id est l'index 0-based de eATTRIBUTION_POWER_ELECTROBOT
    // Les clés DataManager sont PowerSwitch_xt1..8 (1-based)
    QString str_name = QString("PowerSwitch_xt%1").arg(id + 1);
    bool bvalue = (etat != 0);
    m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 1);
    m_application->m_data_center->write(str_name, bvalue);
    m_application->m_data_center->write("ELECTROBOT_CDE_POWER_SWITCH_TxSync", 0);
}

// _____________________________________________________________________
void CHILEngine::sendMovementXY(float x, float y)
{
    m_application->m_data_center->write("COMMANDE_MVT_XY_TxSync", 1);
    m_application->m_data_center->write("X_consigne", static_cast<double>(x));
    m_application->m_data_center->write("Y_consigne", static_cast<double>(y));
    m_application->m_data_center->write("COMMANDE_MVT_XY_TxSync", 0);
}

// _____________________________________________________________________
void CHILEngine::sendMovementXYT(float x, float y, float theta)
{
    m_application->m_data_center->write("COMMANDE_MVT_XY_TETA_TxSync", 1);
    m_application->m_data_center->write("XYT_X_consigne", static_cast<double>(x));
    m_application->m_data_center->write("XYT_Y_consigne", static_cast<double>(y));
    m_application->m_data_center->write("XYT_angle_consigne", static_cast<double>(theta));
    m_application->m_data_center->write("COMMANDE_MVT_XY_TETA_TxSync", 0);
}

// _____________________________________________________________________
void CHILEngine::sendMovementDA(float distance, float angle)
{
    m_application->m_data_center->write("COMMANDE_DISTANCE_ANGLE_TxSync", 1);
    m_application->m_data_center->write("distance_consigne", static_cast<double>(distance));
    m_application->m_data_center->write("angle_consigne", static_cast<double>(angle));
    m_application->m_data_center->write("COMMANDE_DISTANCE_ANGLE_TxSync", 0);
}
