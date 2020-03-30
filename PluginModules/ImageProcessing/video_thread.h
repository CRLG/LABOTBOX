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
    VIDEO_PROCESS_SEQUENCE_COULEUR
}tVideoProcessAlgoType;

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
    IDX_ROBOT1_DIST=0,
    IDX_ROBOT1_ANGLE,
    IDX_ROBOT2_DIST,
    IDX_ROBOT2_ANGLE,
    IDX_ROBOT3_DIST,
    IDX_ROBOT3_ANGLE,
    IDX_NORD,
    IDX_SUD,
    IDX_X_FENETRE,
    IDX_Y_FENETRE,
    IDX_LARGEUR_FENETRE,
    IDX_HAUTEUR_FENETRE,
    IDX_S_RED,
    IDX_V_RED,
    IDX_ECART_RED
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
    void _video_record(cv::Mat frametoRecord);
    void _video_confirm_parameters(cv::Mat frameSample);

    QString getVideoLogFilename();
    void _seuillageImage(cv::Mat *frameHSV, cv::Mat *frameGray, int Couleur, int Saturation, int Purete, int EcartCouleur);
    cv::Point _isColor(cv::Mat *frameGray, int ROIx, int ROIy, int ROIh, int ROIl, int seuil);
    void _init_tResult(tVideoResult *parameter);
};

#endif // VIDEO_THREAD_H
