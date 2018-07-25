/*! \file CTracePlayer.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CTracePlayer_H_
#define _CTracePlayer_H_

#include <QMainWindow>
#include <QThread>
#include <QMap>
#include <QVector>
#include <QVariant>


 /*! \addtogroup DataPlayer
   * 
   *  @{
   */


class CDataManager;

typedef QMap<QString, QVariant>tVariableValue;  // QString=nom de la variable - QVariant=valeur de la variable

typedef struct {
    tVariableValue  variables_values;
    long int duration; //[msec]
    double timestamp_sec;
}tStep;

typedef QVector<tStep *>tSteps;

typedef enum {
    C_PLAYER_IDLE_NO_DATA = 0,          // player en veille car la liste des steps est vide
    C_PLAYER_STOP,                      // player au repos prêt à être démarré
    C_PLAYER_RUN,                       // le player joue la trace
    C_PLAYER_PAUSE,                     // le player est en pause et le dernier step joué est un step qui n'est ni le premier, ni le dernier
    C_PLAYER_PAUSE_FIRST_STEP_INDEX,    // le player est en pause et le dernier step joué est le premier step de la liste
    C_PLAYER_PAUSE_LAST_STEP_INDEX      // le player est en pause et le dernier step joué est le dernier step de la liste
}tPlayerState;

/*! @brief class CTracePlayer
 */	   
class CTracePlayer : public QThread
{
    Q_OBJECT

#define C_PERIODIC_SIGNAL           (-1)
#define C_COMMON_DURATION           (-1)
#define C_ALL_STEPS                 (-1)
#define C_STEP_DURATION_IN_FILE     (-1)


public:
    CTracePlayer(CDataManager *data_manager, QString player_name="");
    ~CTracePlayer();

    // Accesseurs
    void setTraceFilename(QString trace_filename);
    QString getTraceFilename(void)                  { return(m_trace_filename); }

    void setCommonStepDuration(int duration_msec)   { m_common_step_duration = duration_msec; }
    int  getCommonStepDuration(void)                { return(m_common_step_duration); }

    void setCycleNumber(int cycle_number)           { m_cycle_number = cycle_number; }
    int getCycleNumber(void)                        { return(m_cycle_number); }
    
    int getStepsSize(void)                          { return(m_steps.size()); }
    int getCurrentStepIndex(void)                   { return(m_step_index); } // renvoie "-1" pour indiquer que le player est ré-initialisé

    tPlayerState getState(void)                     { return(m_player_state); }


private :
    QString             m_player_name;
    CDataManager        *m_data_manager;
    QString             m_trace_filename;
    int                 m_common_step_duration; // durée d'un pas de temps. Valeur spéciale C_STEP_DURATION_IN_FILE pour indiquer que la durée provient du fichier
    int                 m_step_index;    // Pointe sur le dernier pas joué
    int                 m_cycle_number;  // Nombre de période à effectuer. Valeur spéciale C_PERIODIC_SIGNAL pour indiquer que le signal doit tournner en boucle (periodique)
    int                 m_cycle_count;   // Comptabilise le nombre de période effectuées
    tSteps              m_steps;
    tPlayerState        m_player_state; // Etat de fonctionnement du player

    // Hériatage de QThread
    virtual void run(void);
    void clearSteps(void);
    void playStep(int step_index);
    void changePlayerState(tPlayerState new_state, bool enable_emit=true);
signals :
    void oneStepFinished(QString player_name);
    void playerStarted(QString player_name);
    void playerFinished(QString player_name);
    void playerPaused(QString player_name);
    void stateChanged(QString player_name/*, tPlayerState state*/); // Problème rencontré en passant des signaux à 2 paramètres depuis le thread (les signaux ne sont pas émis : pourquoi ??)

public slots:
    void startPlayer(void);
    void stopPlayer(void);
    void pausePlayer(void);
    void nextStep(void);
    void previousStep(void);
    void activeTrace(bool state);
    void selectAndPlayStep(int step_index);
};

#endif // _CTracePlayer_H_

/*! @} */

