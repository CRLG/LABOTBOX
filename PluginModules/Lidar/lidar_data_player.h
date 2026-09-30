#ifndef _LIDAR_DATA_PLAYER_H_
#define _LIDAR_DATA_PLAYER_H_

#include <QObject>
#include <QTimer>
#include <QVector>

//#include "lidar_data.h"
class CLidarData;

// Format d'enregistrement "brut" (une ligne auto-descriptive par tour lidar)
// Ecrit par CLidar::log_data_brut(), reconnu par CLidarDataPlayer::parse() a sa 2eme colonne
// En-tete : les 4 colonnes fixes, puis le nom de chacune des K donnees associees (lues dans le
//           DataManager a chaque tour : TempsMatch, x_pos...), puis "mesures [mm]"
// Lignes  : timestamp ; angle du 1er point ; pas angulaire ; nombre de mesures N ;
//           valeurs des K donnees associees (vide si absente) ; puis les N distances
#define LIDAR_LOG_BRUT_COLONNES_FIXES     "timestamp [usec];angle debut [deg];resolution [deg];nombre mesures"
#define LIDAR_LOG_BRUT_NBRE_COLONNES_FIXES  4
#define LIDAR_LOG_BRUT_COLONNE_MESURES    "mesures [mm]"

class CLidarDataPlayer : public QObject
{
    Q_OBJECT
public:
    explicit CLidarDataPlayer(QObject *parent = Q_NULLPTR);
    ~CLidarDataPlayer();

    typedef enum {
        PLAYER_STOP,
        PLAYER_IN_PROGRESS,
        PLAYER_PAUSE
    }tPlayerState;

    int get_step_count();

    const int STEP_DURATION_FROM_FILE = -1;

private :
    bool parse_format_brut(QString pathfilename);
    QVector<CLidarData> m_datas;
    QTimer  m_timer;
    int m_current_step;
    int m_state;
    int m_step_duration;

public slots :
    bool parse(QString pathfilename);
    void play(int step=-1);
    void play_current_step();
    int next_step();
    void start();
    void stop();
    void pause();
    void goto_step(int step);
    int current_step();
    void get_step(int step, CLidarData *out_data);
    void clear();
    void set_steps_duration(int rate_msec);

private slots :
    void timer_tick();

signals :
    void new_data(const CLidarData &);
    void started();
    void played(int step);
    void finished();
    void paused();
};

#endif // _LIDAR_DATA_PLAYER_H_
