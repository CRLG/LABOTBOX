/*! \file CImageProcessing.h
 * A brief file description.
 * A more elaborated file description.
 */
#ifndef _CPLUGIN_MODULE_ImageProcessing_H_
#define _CPLUGIN_MODULE_ImageProcessing_H_

#include <QMainWindow>
#include <QThread>
#include <QCameraInfo>
#include <QImage>

#include "CPluginModule.h"
#include "ui_ihm_ImageProcessing.h"

#include "video_thread.h"

 class Cihm_ImageProcessing : public QMainWindow
{
    Q_OBJECT
public:
    Cihm_ImageProcessing(QWidget *parent = 0)  : QMainWindow(parent) { ui.setupUi(this); }
    ~Cihm_ImageProcessing() { }

    Ui::ihm_ImageProcessing ui;

    CApplication *m_application;
 };



 /*! \addtogroup ImageProcessing
   * 
   *  @{
   */

	   
class VideoWorker;
/*! @brief class CImageProcessing
 */	   
class CImageProcessing : public CPluginModule
{
    Q_OBJECT
#define     VERSION_ImageProcessing   "2.0"
#define     AUTEUR_ImageProcessing    "Bichon"
#define     INFO_ImageProcessing      "Traitement d'images OpenCV"

public:
    CImageProcessing(const char *plugin_name);
    ~CImageProcessing();

    Cihm_ImageProcessing *getIHM(void) { return(&m_ihm); }

    virtual void init(CApplication *application);
    virtual void close(void);
    virtual bool hasGUI(void)           { return(true); }
    virtual QIcon getIcon(void)         { return(QIcon(":/icons/edit_add.png")); }  // Précise l'icône qui représente le module
    virtual QString getMenuName(void)   { return("PluginModule"); }                 // Précise le nom du menu de la fenêtre principale dans lequel le module apparaît

    static float getLensRatio(float angle);
private:
    Cihm_ImageProcessing m_ihm;
    QList<QCameraInfo> m_cameras;
    QThread m_video_worker_thread;
    VideoWorker *m_video_worker;
    QString m_camera_parameters;
    bool b_robStarted;
    bool m_auto_on;
    bool m_record;
    void refresh_camera_list();
    tVideoInput getCalibrationValues();
//    void showResultGobelets(int gob1, int gob2, int gob3, int gob4, int gob5);
//    int m_compteur_Nord;
//    int m_compteur_Sud;
private slots :
    void onRightClicGUI(QPoint pos);
    void video_worker_init(int video_source_id);
    void getCamState(int state);
    void setCalibration();
    void getCharucoCalibration();
    void setCharucoCalibration();
    void enableCharucoCalibration(int state);
    void setRecord(int state);
public slots:
     void videoHandleResults(tVideoResult result,QImage imgConst);
     void videoThreadStopped();
     void videoWorkStarted();
     void videoWorkFinished();

     void startVideoWork(void);
     void stopVideoWork();
     void changeVideoWork(QVariant mode);

     void initVideoThread();
     void killVideoThread();

     void activeDebug(int state);
//     void arucoValues(int v);
     void TpsMatch_changed(QVariant val);

signals:
    void operate(tVideoInput param);
    void stopWork();
    void matchStarted();
};

#endif // _CPLUGIN_MODULE_ImageProcessing_H_

/*! @} */

