/*! \file CBotCam.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_BotCam_H_
#define _CPLUGIN_MODULE_BotCam_H_

//#define OPENCV_IHM

#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include "CPluginModule.h"
#include "ui_ihm_BotCam.h"

#define NB_FRAME_BUFFER 45
#define NB_ELEMENTS 10

#include <QObject>
#include <QTimer>
#include <QImage>
#include <QDebug>
#include <QDateTime>
#include "opencv2/opencv.hpp"
//#include "opencv2/core/operations.hpp"
//#include "opencv2/core/mat.hpp"
#include <QHash>
#include <QMap>
#include <QPoint>
#include <QtAlgorithms>
#include <QPointer>

#define orange 20



enum type_Affichage{
        affichage_Couleur_Origine,
        affichage_Couleur_Detection,
        affichage_NetB_Origine,
        affichage_NetB_Reglage_HSV
};

enum type_Traitement{
        calibration=0,
    cherche_combinaison,
        tracking,
        balise
};

typedef struct
{
    int X;
    int Y;
    double distance;
}
tCoordonneesElements;

//using namespace cv;


 class Cihm_BotCam : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_BotCam(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_BotCam() { }

    Ui::ihm_BotCam ui;

    CLaBotBox *m_application;
 };



 /*! \addtogroup BotCam
   * 
   *  @{
   */

	   
/*! @brief class CBotCam
 */	   
class CBotCam : public CPluginModule
{
    Q_OBJECT
#define     VERSION_BotCam   "1.0"
#define     AUTEUR_BotCam    "Laguiche"
#define     INFO_BotCam      "Ability to use a Cam with OpenCV lib"

public:
    CBotCam(const char *plugin_name);
    ~CBotCam();

    Cihm_BotCam *getIHM(void) { return(&m_ihm); }

    virtual void init(CLaBotBox *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Pr�cise l'ic�ne qui repr�sente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Pr�cise le nom du menu de la fen�tre principale dans lequel le module appara�t
	
	bool CamIsOK;

    bool closeCam(void);
    void analyseCam(cv::Mat frame);
    void afficheCam(cv::Mat frameToQt, bool Colored, int type_Image);
   // void runCam(bool b_activate);
    double S_min;
    double H_min;
    double H_max;
    double H_median;
    double superficieObjetMax;
    double superficieObjetMin;
    int compteurImages;

    int x_traitement;
    int y_traitement;
    int L_traitement;
    int H_traitement;

    void initMotifSeuil();
    cv::Point isColor(cv::Mat *frameGray, int ROIx, int ROIy, int ROIh, int ROIl, int seuil);

private:
    Cihm_BotCam m_ihm;
	
    cv::VideoCapture * capture;
    cv::VideoWriter * enregistreur;
    cv::Mat framesBuffer[NB_FRAME_BUFFER];
    int nb_frames_in_buffer;
    bool isRecording;
    tCoordonneesElements elementsDetectes[NB_ELEMENTS];
    //QPointer<QCamera> camera;
    //QPointer<QCameraImageCapture> imageCapture;
    //QCameraViewfinder *viewfinder;
    bool cameraEnabled;
	int camUsed;
    QTimer *schedulerCam;
    int camNumber;
    int confirmation_detection;

    void seuillageImage(cv::Mat *frameHSV, cv::Mat *frameGray, int Couleur, int Saturation, int Purete, int EcartCouleur);
    //bool triColor(const QPair<QString, int> &e1, const QPair<QString, int> &e2);
signals:
    void  frameCaptured(QImage frame,int type_Image);
	
private slots :
    //void updateTooltip();
    void onRightClicGUI(QPoint pos);
     //void setCamera(int numDevice);
    //void enableCamera(int state);
    //void displayCameraError();
    //void takePicture();
    bool getCam(void);
    void calibrateCam(void);

public slots:
    void setSeuil(double H_angle_detection, double S_purete_detection, double area_detection);
    void setCouleur(QString couleur);
    void displayFrame(QImage imgConst, int type);
    void runCam(bool b_activate);
    void recording(bool flag);
};

#endif // _CPLUGIN_MODULE_BotCam_H_

/*! @} */

