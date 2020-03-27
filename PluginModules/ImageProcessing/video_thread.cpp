#include <qdebug.h>
#include <QDir>

#include "video_thread.h"

// ======================================================
// ======================================================
VideoWorker::VideoWorker()
{
    iH=0;
    iL=0;
    parameterConfirmed=false;
    recordInitialized=false;
    m_dbg_active=false;
}

void VideoWorker::stopWork()
{
    m_stop_work_request = true;
}

void VideoWorker::activeDebug(bool on_off)
{
    m_dbg_active = on_off;
}

// ======================================================
// ======================================================
bool VideoWorker::init(int video_id, QString parameter_file)
{
    bool isInit=false;

    calibrationFixParameters="%YAML:1.0 \n\
            --- \n\
            calibration_time: \"ven. 28 sept. 2018 16:51:07 CEST\"\n\
            image_width: 640\n\
            image_height: 480\n\
            aspectRatio: 1.\n\
            flags: 2\n\
            camera_matrix: !!opencv-matrix\n\
               rows: 3\n\
               cols: 3\n\
               dt: d\n\
               data: [  745.    , 0.    , 311.,\n\
                        0.      ,745.   , 250.,\n\
                        0.      , 0.    , 1. ]\n\
            distortion_coefficients: !!opencv-matrix\n\
               rows: 1\n\
               cols: 5\n\
               dt: d\n\
               data: [ -0.3, 5.7,-0.02,0.05,-0.3]\n\
            avg_reprojection_error: 0.8";

    //m_video_name = video_name;
    m_video_id=video_id;

    //ouverture de la vidéo
    capture= new cv::VideoCapture(m_video_id);
    if(!(capture->isOpened()))
        capture= new cv::VideoCapture(-1);

    if(capture->isOpened())
    {
        int FourCC=capture->get(CV_CAP_PROP_FOURCC);
        int hasToBeConvertedRGB=capture->get(CV_CAP_PROP_CONVERT_RGB);
        int setHeight=capture->set(CV_CAP_PROP_FRAME_HEIGHT,480);
        int setWidth=capture->set(CV_CAP_PROP_FRAME_WIDTH,640);

        if(m_dbg_active)
        {
            unsigned char m_1_CC = FourCC;
            unsigned char m_2_CC = FourCC>>8;
            unsigned char m_3_CC = FourCC>>16;
            unsigned char m_4_CC = FourCC>>24;
            qDebug() << "[CImageProcessing] La caméra est opérationnelle.";
            qDebug() << "[CImageProcessing] Id de la camera choisie : " << video_id;
            qDebug() << "[CImageProcessing] Code FOURCC de l'image acquise:\t\t" << m_1_CC << m_2_CC << m_3_CC << m_4_CC;
            qDebug() << "[CImageProcessing] Image a convertir de BGR à RGB avant traitement:" << ((hasToBeConvertedRGB==1)? "OUI":"NON");
            qDebug() << "[CImageProcessing] Réglage de la hauteur de l'image à 480:\t" << ((setHeight==1) ? "REUSSI" : "ERREUR");
            qDebug() << "[CImageProcessing] Réglage de la longueur de l'image à 640 :\t" << ((setWidth==1) ? "REUSSI" : "ERREUR");
            //qDebug() << "[CImageProcessing] Mettre l'exposition à 50 :\t" << ((capture->set( CV_CAP_PROP_EXPOSURE, 50)) ? "REUSSI" : "ERREUR") << endl;
        }

        //calibration de la caméra
        QDir directory;
        QString path=directory.currentPath()+"/Config/"+parameter_file;
        std::string contents = "";

        //lecture du fichier de calibration
        QFile f(path);
        if(!f.open(QFile::ReadOnly))
        {
            if(m_dbg_active)
            {
                qDebug() << "[CImageProcessing] Ouverture impossible du fichier de calibration " << path;
                qDebug() << "[CImageProcessing] Données de calibration par défaut, le traitement peut être dégradé voire dysfonctionnel!";
            }
            contents=calibrationFixParameters;
        }
        else
        {
            contents = f.readAll().toStdString();
            f.close();
        }

        //Réglage de la caméra avec les données de calibration
        cv::FileStorage fSettings(contents, cv::FileStorage::READ|cv::FileStorage::MEMORY);
        if (!fSettings.isOpened())
        {
            fSettings.release();
            bCalibrated=false;
            if(m_dbg_active)
                qDebug() << "[CImageProcessing] Enregistrement impossible des données du fichier de calibration " << path;
        }
        else
        {
            fSettings["camera_matrix"] >> camMatrix;
            fSettings["distortion_coefficients"] >> distCoeffs;
            fSettings.release();
            bCalibrated=true;
            if(m_dbg_active)
                qDebug() << "[CImageProcessing] Camera calibrée";
        }

        //init de la taille des marqueurs AruCo
        markerLength=7;

        //on ne peut pas travailler avec une caméra non calibrée
        if(bCalibrated)
        {
            isInit=true;
            camState=true;
            emit camStateChanged(1);
        }
        else
        {
            capture->release();
            isInit=false;
            camState=false;
            emit camStateChanged(0);
            if(m_dbg_active)
                qDebug() << "[CImageProcessing] Caméra inopérante :-(";
        }
    }
    else
    {
        isInit=false;
        camState=false;
        emit camStateChanged(0);
        if(m_dbg_active)
            qDebug() << "[CImageProcessing] Caméra inopérante :-(";
    }

    return isInit;
}

VideoWorker::~VideoWorker()
{
    //on ferme la camera
    if(capture->isOpened())
    {
        emit camStateChanged(0);
        capture->release();
    }
}

bool VideoWorker::getCamState()
{
    return camState;
}

// _________________________________________________________________
void VideoWorker::doWork(tVideoInput parameter) {
    m_stop_work_request = false;

    emit workStarted();
    // ----------------------------------------------
    while (!m_stop_work_request)
    {
        switch(parameter.video_process_algo)
        {
            case VIDEO_PROCESS_BALISE_MAT :
                _video_process_algo1(parameter);
            break;

            case VIDEO_PROCESS_DUMMY :
                _video_process_dummy(parameter);
            break;

            // ...

            default :
                // nothing to do
                QThread::msleep(50);
            break;
        }
        QThread::msleep(5);
    }
    // ----------------------------------------------
    emit workFinished();
}


// ========================================================
void VideoWorker::_video_process_algo1(tVideoInput parameter)
{
    tVideoResult result;
    QTime t;
      t.start();


    //capture d'une image du flux video
    capture->grab();

    //récupération de l'image
    bool captureOK=capture->retrieve(m_frame,0);

    //l'image a-t-elle bien été récupérée
    if (captureOK)
    {
        //dans le cas ou la configuration de la caméra aurait mal fonctionné
        if(!parameterConfirmed)
            _video_confirm_parameters(m_frame);

        //clone de l'image pour la persistence des données
        m_frameCloned=m_frame.clone();

        if(parameter.record)
            _video_record(m_frame);//on enregistre le flux video

        //analyse de l'image
        cv::Mat inputImage=m_frameCloned.clone();

        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
        std::vector< cv::Vec3d > rvecs, tvecs;

        cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_100);
        cv::aruco::detectMarkers(inputImage, dictionary, markerCorners, markerIds);


        if (markerIds.size() > 0)
        {

            //on dessine les marqueurs trouvés
            if (m_dbg_active)
            cv::aruco::drawDetectedMarkers(m_frameCloned, markerCorners, markerIds);
            //on estime leur position en 3D par rapport à la caméra
            cv::aruco::estimatePoseSingleMarkers(markerCorners, markerLength, camMatrix, distCoeffs, rvecs,tvecs);
            //on donne la valeur normale à la caméra dans la console pour cahque marqueur
            /*
            for(unsigned int i = 0; i < markerIds.size(); i++)
            {
                cv::aruco::drawAxis(m_frameCloned, camMatrix, distCoeffs, rvecs[i], tvecs[i], markerLength * 0.5f);
                qDebug() << 100*tvecs[0][2] << "cm";
            }
            */
            for(unsigned int i = 0; i < markerIds.size(); i++)
            {
                if((markerIds.at(i)==1)||(markerIds.at(i)==2))
                {
                    cv::aruco::drawAxis(m_frameCloned, camMatrix, distCoeffs, rvecs[i], tvecs[i], markerLength * 0.5f);
                    double dist=sqrt(tvecs[i][0]*tvecs[i][0]+tvecs[i][1]*tvecs[i][1]+tvecs[i][2]*tvecs[i][2]);
                    result.robot1_dist = dist;
                    float teta=0;
                    if(tvecs[i][1]!=0)
                        teta=acos(tvecs[i][0]/tvecs[i][1]);
                    result.robot1_angle=teta;
                    /*result.markers_detected = markerIds;*/
                }

                if((markerIds.at(i)==3)||(markerIds.at(i)==4))
                {
                    cv::aruco::drawAxis(m_frameCloned, camMatrix, distCoeffs, rvecs[i], tvecs[i], markerLength * 0.5f);
                    double dist=sqrt(tvecs[i][0]*tvecs[i][0]+tvecs[i][1]*tvecs[i][1]+tvecs[i][2]*tvecs[i][2]);
                    result.robot2_dist = dist;
                    float teta=0;
                    if(tvecs[i][1]!=0)
                        teta=acos(tvecs[i][0]/tvecs[i][1]);
                    result.robot2_angle=teta;
                    /*result.markers_detected = markerIds;*/
                }

            }

        }
        else
        {
            result.robot1_dist = 0;
            result.robot2_dist = 0;
            result.robot3_dist = 0;
            result.robot1_angle = 0;
            result.robot2_angle = 0;
            result.robot3_angle = 0;
            //result.markers_detected = markerIds;
        }

        int fps=1000/(t.elapsed());
        result.m_fps=fps;

        //TIMESTAMP_MATCH.Timestamp

        QImage imgConst(iL,iH,QImage::Format_RGB888); //image du buffer video au format Qt

        //on affiche l'image traitée
        if (m_dbg_active)
        {
            //recuperation des donnees de la frame
            //et referencement dans l'image Qt
            //ATTENTION: pas de copie des donnees -> garbage collector
            //conversion de l'image en RGB
            cv::Mat frame_converted;
            frame_converted=m_frame.clone();
            cv::cvtColor(m_frameCloned,frame_converted, CV_BGR2RGB);

            //affichage overlay
            char str[200];
            sprintf(str,"%d fps",fps);
            cv::putText(frame_converted, str, cv::Point2f(20,20), cv::FONT_HERSHEY_PLAIN, 2,  cv::Scalar(255,0,0,0));

            const uchar *qImageBuffer = (const uchar*)frame_converted.data;
            //const uchar *qImageBuffer = (const uchar*)m_frameCloned.data;
            //cv::imwrite("tata.jpg",frame);
            QImage img(qImageBuffer, iL, iH, QImage::Format_RGB888);

            //copie complete de l'image pour eviter une desallocation sauvage de la frame
            imgConst=img.copy(QRect(0,0,iL,iH));
             frame_converted.release();
        }

        emit resultReady(result,imgConst);

        //QThread::msleep(5);
        inputImage.release();

       //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
    }
    else
    {
        //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
    }
}


// _________________________________________________________________
void VideoWorker::_video_process_dummy(tVideoInput parameter)
{
    QString str;
    tVideoResult result;
    //qDebug() << str.sprintf("Thread start on %s / with   %f   %f   %f", m_video_name.toStdString().c_str(), parameter.data1, parameter.data2, parameter.data3);;
    int i=0;
    const int total_count = 5;
    while(i++<total_count)
    {
        QThread::sleep(1);
        result.robot1_dist = parameter.data1;
        result.robot1_angle += parameter.data3 + 0.1 ;
                QImage img( 320, 240, QImage::Format_RGB888);
        emit resultReady(result,img);
        /*if (m_dbg_active) {
            qDebug() << "Thread is still running" << ((float)i/total_count)*100. << "%";
        }*/
    }
}

void VideoWorker::_video_record(cv::Mat frametoRecord)
{
    if(!recordInitialized)
    {
        //construction du nom de fichier d'enregistrement de la video
        QString pathfilename = getVideoLogFilename();

        //création de l'objet OpenCV qui enregistre des frame
        record= new cv::VideoWriter(pathfilename.toStdString(),CV_FOURCC('X','V','I','D'),15,cv::Size(iL,iH),true);
    }
    record->write(frametoRecord);//on enregistre le flux video
}

void VideoWorker::_video_confirm_parameters(cv::Mat frameSample)
{
    //recuperation des hauteurs et largeur de l'image
    iH = frameSample.rows;
    iL = frameSample.cols;
    //qDebug() << "Récupération des paramètres Vidéos";
    //qDebug() << "Résolution " << iH << "x" << iL;
    parameterConfirmed=true;
}

// _____________________________________________________________________
QString VideoWorker::getVideoLogFilename()
{
    QString path = "./Capture";
    QDir dir(path);
    if (!dir.exists()) {
        dir.mkdir(path);
    }

    QString pathfilename;
    for (int i=1; i<10000; i++) {
        pathfilename =  path +
                        "/Match_" +
                        QString::number(i) +
                        ".avi";
        QFileInfo fileinfo(pathfilename);
        if (!fileinfo.exists()) break;
    }
    return pathfilename;
}
