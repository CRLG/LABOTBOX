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
    VIDEO_PROCESS_NORD_SUD,
    VIDEO_PROCESS_SEQUENCE_COULEUR,
    VIDEO_PROCESS_CALIBRATION
}tVideoProcessAlgoType;

typedef enum {
    SEQUENCE_UNKNOWN=0,
    SEQUENCE_GRGGR,
    SEQUENCE_GRRGR,
    SEQUENCE_GGRGR,
    SEQUENCE_GRGRR,
    SEQUENCE_GGGRR,
    SEQUENCE_GGRRR
}tVideoSequence;

typedef struct
{
    tVideoProcessAlgoType video_process_algo;
    float value[20];
    bool record;
}tVideoInput;

typedef struct
{
    std::vector <int>markers_detected;
    int m_fps;
    float value[20];
}tVideoResult;

typedef enum {
    IDX_PARAM_01=0,
    IDX_PARAM_02,
    IDX_PARAM_03,
    IDX_PARAM_PURETE_VERT,
    IDX_PARAM_ECART_VERT,
    IDX_PARAM_PURETE_ROUGE,
    IDX_PARAM_ECART_ROUGE,
    IDX_PARAM_PIXEL_MIN,
    IDX_PARAM_PIXEL_MAX,
    IDX_PARAM_CALIB_TYPE,
    IDX_PARAM_X_FENETRE,
    IDX_PARAM_Y_FENETRE,
    IDX_PARAM_LARGEUR_FENETRE,
    IDX_PARAM_HAUTEUR_FENETRE //14 sur 20
} tIdxParam;

typedef enum {
    IDX_ROBOT1_DIST=0,
    IDX_ROBOT1_ANGLE,
    IDX_ROBOT2_DIST,
    IDX_ROBOT2_ANGLE,
    IDX_ROBOT3_DIST,
    IDX_ROBOT3_ANGLE,
    IDX_NORD,
    IDX_SUD,
    IDX_SEQUENCE //9 sur 20
} tIdxResult;

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
    float m_internal_param[20];

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
    int m_iH;
    int m_iL;
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
    void _video_process_Balise(tVideoInput parameter);
    void _video_process_NordSud(tVideoInput parameter);
    void _video_process_ColorSequence(tVideoInput parameter);
    void _video_process_Calibration(tVideoInput parameter);
    void _video_record(cv::Mat frametoRecord);
    void _video_confirm_parameters(cv::Mat frameSample);

    QString getVideoLogFilename();
    void _init_tResult(tVideoResult *parameter);
};

#endif // VIDEO_THREAD_H
