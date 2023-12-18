#ifndef _LIDAR_DATA_PLAYER_H_
#define _LIDAR_DATA_PLAYER_H_

#include <QObject>
#include <QTimer>
#include <QVector>

//#include "lidar_data.h"
class CLidarData;

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

private :
    QVector<CLidarData> m_datas;
    QTimer  m_timer;
    int m_current_step;
    int m_state;

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

private slots :
    void timer_tick();

signals :
    void data_pending(int step);
    void new_data(const CLidarData &);
    void started();
    void played(int step);
    void finished();
    void paused();
};

#endif // _LIDAR_DATA_PLAYER_H_
