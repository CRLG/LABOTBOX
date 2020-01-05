#include <qdebug.h>
#include <QDir>

#include "video_thread.h"

// ======================================================
// ======================================================
VideoWorker::VideoWorker()
    : m_dbg_active(true)
{
    iH=0;
    iL=0;
    parameterConfirmed=false;
    recordInitialized=false;
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
void VideoWorker::init(int video_id, QString parameter_file)
{
    //m_video_name = video_name;
    m_video_id=video_id;
    //ouverture de la vidéo
    capture= new cv::VideoCapture(m_video_id);
    if(!(capture->isOpened()))
        capture= new cv::VideoCapture(-1);

    if(capture->isOpened())
     {     
        qDebug()<<"La caméra est opérationnelle.";
        //infos de debug
        qDebug() << endl <<"camera choisie :\t\t" << video_id;
        int FourCC=capture->get(CV_CAP_PROP_FOURCC);
        qDebug("FOURCC code:\t\t%c%c%c%c", FourCC, FourCC>>8, FourCC>>16, FourCC>>24);
        qDebug() << "A convertir de BGR à RGB :\t" << ((capture->get(CV_CAP_PROP_CONVERT_RGB)==1) ? "OUI":"NON");
        qDebug() << "Mettre la hauteur à 480 :\t" << ((capture->set(CV_CAP_PROP_FRAME_HEIGHT,480)==1) ? "REUSSI" : "ERREUR");
        qDebug() << "Mettre la longueur à 640 :\t" << ((capture->set(CV_CAP_PROP_FRAME_WIDTH,640)==1) ? "REUSSI" : "ERREUR");
        //qDebug() << "Mettre l'exposition à 50 :\t" << ((capture->set( CV_CAP_PROP_EXPOSURE, 50)) ? "REUSSI" : "ERREUR") << endl;

        emit setCamState(1);

         //calibration de la caméra
         qDebug() << "Fichier de calibration choisi:"<<parameter_file;
         //cv::FileStorage fs(parameter_file.toStdString(),cv::FileStorage::READ);
         cv::FileStorage fs("cam_parameters_pc.txt", cv::FileStorage::READ);
         if(fs.isOpened())
         {
             fs["camera_matrix"] >> camMatrix;
             fs["distortion_coefficients"] >> distCoeffs;
             bCalibrated=true;
             qDebug() << "Camera calibrée";
         }
         else
             bCalibrated=false;
         markerLength=7;
     }
     else
    {
        emit setCamState(0);
        qDebug() << endl << "Caméra inopérante :-(" << endl;
    }
}

VideoWorker::~VideoWorker()
{
    //on ferme la camera
    if(capture->isOpened())
    {
        capture->release();
    }
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
    qDebug() << str.sprintf("Thread start on %s / with   %f   %f   %f", m_video_name.toStdString().c_str(), parameter.data1, parameter.data2, parameter.data3);;
    int i=0;
    const int total_count = 5;
    while(i++<total_count)
    {
        QThread::sleep(1);
        result.robot1_dist = parameter.data1;
        result.robot1_angle += parameter.data3 + 0.1 ;
                QImage img( 320, 240, QImage::Format_RGB888);
        emit resultReady(result,img);
        if (m_dbg_active) {
            qDebug() << "Thread is still running" << ((float)i/total_count)*100. << "%";
        }
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
    qDebug() << "Récupération des paramètres Vidéos";
    qDebug() << "Résolution " << iH << "x" << iL;
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
