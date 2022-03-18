#ifndef VIDEO_THREAD_H
#define VIDEO_THREAD_H
//CBY
#include <QObject>
#include <QThread>
#include <QImage>
#include <QDebug>
#include <QDateTime>
#include <QTime>
#include "opencv2/opencv.hpp"
#include <opencv2/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/aruco/charuco.hpp>

typedef struct{
    float _a; //Fix aspect ratio (fx/fy) to this value
    int _w; //Number of squares in X direction
    int _h; //Number of squares in Y direction
    float _sl; //Square side length (in meters)
    float _ml; //Marker side length (in meters)
    int _d;  //dictionary of tags
                /*DICT_4X4_50=0, DICT_4X4_100=1, DICT_4X4_250=2,
                DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, DICT_5X5_250=6, DICT_5X5_1000=7,
                DICT_6X6_50=8, DICT_6X6_100=9, DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12,
                DICT_7X7_100=13, DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL=16*/
    bool _zt; //Assume zero tangential distortion
    bool _pc; //Fix the principal point at the center
    bool _rs; //Apply refind strategy
}tCharucoParam;

typedef struct{
    int calibrationFlags;
    float aspectRatio;
    std::vector< std::vector< std::vector< cv::Point2f > > > allCorners;
    std::vector< std::vector< int > > allIds;
    std::vector< cv::Mat > allImgs;
    cv::Size imgSize;
    cv::Ptr<cv::aruco::DetectorParameters> detectorParams;
    cv::Ptr<cv::aruco::Dictionary> dictionary;
    cv::Ptr<cv::aruco::CharucoBoard> charucoboard;
    cv::Ptr<cv::aruco::Board> board;
}tCharucoProcessing;

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
    IDX_PARAM_HAUTEUR_FENETRE,
    IDX_PARAM_GET_CHARUCO_FRAME,
    IDX_PARAM_SET_CHARUCO_FRAME //16 sur 20
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
    IDX_SEQUENCE,
    IDX_CHARUCO_IS_SET//9 sur 20
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
    QString m_path_parameter_file;

    std::string calibrationFixParameters;

    //pour calibration Charuco
    bool m_charucoInit;
    bool m_charucoFinish;
    bool m_charucoIsSet;
    tCharucoParam m_charucoCalibParam;
    tCharucoProcessing m_charucoProcessing;

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
    cv::Mat3b _charucoProcessing(cv::Mat3b inputFrame);
    void _charucoFinish();
    void _charucoInit();
    bool _saveCameraParams(cv::Size imageSize, float aspectRatio, int flags, const cv::Mat &cameraMatrix, const cv::Mat &distCoeffs, double totalAvgErr);
};

#endif // VIDEO_THREAD_H
