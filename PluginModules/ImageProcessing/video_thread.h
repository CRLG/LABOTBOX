#ifndef VIDEO_THREAD_H
#define VIDEO_THREAD_H

#include <QObject>
#include <QThread>
#include <QImage>
#include <QDebug>
#include <QDateTime>
#include <QTime>
#include "opencv2/opencv.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>

typedef enum {
    VIDEO_PROCESS_BALISE_MAT = 0,
    VIDEO_PROCESS_RECO_ARUCO,
    VIDEO_PROCESS_RECO_COULEURS,
    VIDEO_PROCESS_DUMMY
}tVideoProcessAlgoType;

typedef struct
{
    tVideoProcessAlgoType video_process_algo;
    float data1;
    float data2;
    float data3;
    bool record;

}tVideoInput;

typedef struct
{
    double robot1_dist;
    float robot1_angle;
    double robot2_dist;
    float robot2_angle;
    double robot3_dist;
    float robot3_angle;
    std::vector <int>markers_detected;
    int m_fps;
}tVideoResult;

Q_DECLARE_METATYPE(tVideoInput)
Q_DECLARE_METATYPE(tVideoResult)

// ======================================================
// ======================================================
class VideoWorker : public QObject
{
    Q_OBJECT
public :
    VideoWorker();
    bool init(int video_id, QString parameter_file);
    ~VideoWorker();
    bool getCamState(void);

private :
    bool camState;

    QString m_video_name;
    int m_video_id;
    bool m_stop_work_request;
    bool m_dbg_active;

    cv::VideoCapture * capture;
    cv::VideoWriter * record;
    cv::Mat m_frame; //image du buffer video
    cv::Mat m_frameCloned;
    cv::Mat camMatrix;
    cv::Mat distCoeffs;
    bool bCalibrated;
    float markerLength;
    int iH;
    int iL;
    bool parameterConfirmed;
    bool recordInitialized;

    std::string calibrationFixParameters;



public slots:
    void doWork(tVideoInput parameter);
    void stopWork();
    void activeDebug(bool on_off=true);

signals:
    void resultReady(tVideoResult result,QImage imgConst);
    void workStarted();
    void workFinished();
    void camStateChanged(int state);

private:
    void _video_process_algo1(tVideoInput parameter);
    void _video_process_dummy(tVideoInput parameter);
    void _video_record(cv::Mat frametoRecord);
    void _video_confirm_parameters(cv::Mat frameSample);

    QString getVideoLogFilename();
};

#endif // VIDEO_THREAD_H
