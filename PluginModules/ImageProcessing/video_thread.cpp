#include <qdebug.h>

#include "video_thread.h"

// ======================================================
// ======================================================
VideoWorker::VideoWorker()
    : m_dbg_active(true)
{

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
         qDebug() << endl <<"camera choisie :" << video_id;
         int FourCC=capture->get(CV_CAP_PROP_FOURCC);
         qDebug("FOURCC code: %c%c%c%c", FourCC, FourCC>>8, FourCC>>16, FourCC>>24);
         qDebug() << "Has to be converted to RGB :" << capture->get(CV_CAP_PROP_CONVERT_RGB); //((capture->get(CV_CAP_PROP_CONVERT_RGB)==1) ? "YES":"NO");
         qDebug() << "Set height to 240 :" << ((capture->set(CV_CAP_PROP_FRAME_HEIGHT,240)) ? "OK" : "NOK");
         qDebug() << "Set width to 320 :" << ((capture->set(CV_CAP_PROP_FRAME_WIDTH,320)) ? "OK" : "NOK") << endl;
         //cv::namedWindow( "capture", cv::WINDOW_AUTOSIZE );// Create a window for display.

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
         qDebug() << endl << "Caméra inopérante :-(" << endl;
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
            case VIDEO_PROCESS_ALGO1 :
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

    //capture d'une image du flux video
    capture->grab();

    //récupération de l'image
    bool captureOK=capture->retrieve(m_frame,0);

    //l'image a-t-elle bien été récupérée
    if (captureOK)
    {
        //clone de l'image pour la persistence des données
        m_frameCloned=m_frame.clone();

        //analyse de l'image

        cv::Mat inputImage=m_frameCloned.clone();

        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
        std::vector< cv::Vec3d > rvecs, tvecs;

        cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
        cv::aruco::detectMarkers(inputImage, dictionary, markerCorners, markerIds);


        if (markerIds.size() > 0)
        {

            //on dessine les marqueurs trouvés
            if (m_dbg_active)
            cv::aruco::drawDetectedMarkers(m_frameCloned, markerCorners, markerIds);
            //on estime leur position en 3D par rapport à la caméra
            cv::aruco::estimatePoseSingleMarkers(markerCorners, markerLength, camMatrix, distCoeffs, rvecs,tvecs);
            //on donne la valeur normale à la caméra dans la console pour cahque marqueur
            /*for(unsigned int i = 0; i < markerIds.size(); i++)
            {
            cv::aruco::drawAxis(m_frameCloned, camMatrix, distCoeffs, rvecs[i], tvecs[i], markerLength * 0.5f);
            qDebug() << 100*tvecs[0][2] << "cm";
            }*/
            for(unsigned int i = 0; i < markerIds.size(); i++)
            {
                if(markerIds.at(i)==1)
                {
                    cv::aruco::drawAxis(m_frameCloned, camMatrix, distCoeffs, rvecs[i], tvecs[i], markerLength * 0.5f);
                    result.result1 = tvecs[i][0];
                    result.result2 = tvecs[i][1];
                    result.result3 = tvecs[i][2];
                    result.markers_detected = markerIds;
                }

            }

        }
        else
        {
            result.result1 = 0;
            result.result2 = 0;
            result.result3 = 0;
            //result.markers_detected = markerIds;
        }

        //recuperation des hauteurs et largeur de l'image
        int iH = m_frameCloned.rows;
        int iL = m_frameCloned.cols;

        QImage imgConst(iL,iH,QImage::Format_RGB888); //image du buffer video au format Qt

        //on affiche l'image traitée
        if (m_dbg_active)
        {


        //recuperation des donnees de la frame
        //et referencement dans l'image Qt
        //ATTENTION: pas de copie des donnees -> garbage collector
        const uchar *qImageBuffer = (const uchar*)m_frameCloned.data;
        //cv::imwrite("tata.jpg",frame);
        QImage img(qImageBuffer, iL, iH, QImage::Format_RGB888);

        //copie complete de l'image pour eviter une desallocation sauvage de la frame
        imgConst=img.copy(QRect(0,0,iL,iH));
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
        result.result1 = parameter.data1;
        result.result2 += parameter.data3 + 0.1 ;
                QImage img( 320, 240, QImage::Format_RGB888);
        emit resultReady(result,img);
        if (m_dbg_active) {
            qDebug() << "Thread is still running" << ((float)i/total_count)*100. << "%";
        }
    }
}
