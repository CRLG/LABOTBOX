#include <qdebug.h>
#include <QDir>

#include "video_thread.h"

// ======================================================
// ======================================================
VideoWorker::VideoWorker()
{
    m_iH=0;
    m_iL=0;
    parameterConfirmed=false;
    recordInitialized=false;
    m_dbg_active=false;
    m_charucoFinish=false;
    m_charucoInit=false;
    m_charucoIsSet=false;
    for(int i=0;i<20;i++)
        m_internal_param[i]=0;

    /*charuco_calibration common parameters for CRLG (the charuco chess board is with the source files
     *  -a=1 --ci=1 -d=10 -h=4 -ml=0.0022 -sc=true -sl=0.0045 -w=6
     */
    m_charucoCalibParam._a=1.0; //Fix aspect ratio (fx/fy) to this value
    m_charucoCalibParam._w=6; //Number of squares in X direction
    m_charucoCalibParam._h=4; //Number of squares in Y direction
    m_charucoCalibParam._sl=0.00456; //Square side length (in meters)
    m_charucoCalibParam._ml=0.00347; //Marker side length (in meters)
    m_charucoCalibParam._d=10;  //dictionary of tags
                /*DICT_4X4_50=0, DICT_4X4_100=1, DICT_4X4_250=2,
                DICT_4X4_1000=3, DICT_5X5_50=4, DICT_5X5_100=5, DICT_5X5_250=6, DICT_5X5_1000=7,
                DICT_6X6_50=8, DICT_6X6_100=9, DICT_6X6_250=10, DICT_6X6_1000=11, DICT_7X7_50=12,
                DICT_7X7_100=13, DICT_7X7_250=14, DICT_7X7_1000=15, DICT_ARUCO_ORIGINAL=16*/
    m_charucoCalibParam._zt=false; //Assume zero tangential distortion
    m_charucoCalibParam._pc=false; //Fix the principal point at the center
    m_charucoCalibParam._rs=true; //Apply refind strategy
}

void VideoWorker::stopWork()
{
    m_stop_work_request = true;
}

void VideoWorker::activeDebug(bool on_off)
{
    m_dbg_active = on_off;
}

void VideoWorker::setArucoValues(int taille, int tag1, int tag2)
{
    m_aruco_size_lat = taille;
    m_aruco_taglat_1 = tag1;
    m_aruco_taglat_1 = tag2;
    qDebug() << "Aruco param" << m_aruco_size_lat << m_aruco_taglat_1 << m_aruco_taglat_2 ;
}
// ======================================================
// ======================================================
bool VideoWorker::init(int video_id, QString parameter_file)
{
    bool isInit=false;
  //  QVariant val;

   // val = m_application->m_eeprom->read(getName(), "aruco_taille", QVariant(4.5));
   // m_aruco_size=val.toDouble();
    qDebug() << "VideoWorker::init";// << m_aruco_size;

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
    m_dbg_active = true;
    if(capture->isOpened())
    {
        int FourCC=capture->get(cv::CAP_PROP_FOURCC);
        int hasToBeConvertedRGB=capture->get(cv::CAP_PROP_CONVERT_RGB);
        int setHeight=capture->set(cv::CAP_PROP_FRAME_HEIGHT,480);
        int setWidth=capture->set(cv::CAP_PROP_FRAME_WIDTH,640);
            qDebug() << "capture opened" << m_dbg_active;
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
            //qDebug() << "[CImageProcessing] Mettre l'exposition à 50 :\t" << ((capture->set( cv::CAP_PROP_EXPOSURE, 50)) ? "REUSSI" : "ERREUR") << endl;
        }

        //calibration de la caméra
        QDir directory;
        QString path=directory.currentPath()+"/Config/"+parameter_file;
        m_path_parameter_file=path;
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
        markerLength=4.25;

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

    if(record->isOpened())
        record->release();
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
                _video_process_Balise(parameter);
            break;

//            case VIDEO_PROCESS_SEQUENCE_COULEUR :
//                _video_process_ColorSequence(parameter);
//            break;

            case VIDEO_PROCESS_BALISE_LATERALE :
               // _video_process_NordSud(parameter);
            break;

            case VIDEO_PROCESS_CAM_ROBOT : //CBY
                _video_process_CameraRobot(parameter);
            break;

            case VIDEO_PROCESS_CALIBRATION :
                _video_process_Calibration(parameter);
            break;

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
void VideoWorker::_video_process_CameraRobot(tVideoInput parameter)
{
    tVideoResult result;
    _init_tResult(&result);
    QTime t;
      t.start();


    //capture d'une image du flux video
    capture->grab();

    //récupération de l'image
    bool captureOK=capture->retrieve(m_frame,0);

    //initialisation des resultats, des fois que ça ne marche pas comme on voulait CBY
    result.value[IDX_ROBOT1_DIST] = -32000;
    result.value[IDX_ROBOT1_ANGLE] = -32000;
    result.value[IDX_ROBOT2_DIST] = -32000;
    result.value[IDX_ROBOT2_ANGLE] = -32000;

    //l'image a-t-elle bien été récupérée
    if (captureOK)
    {
        cv::Mat frame_converted;
        cv::Mat3b frame_record;

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
            // variables qui servent à savoir combien on a trouvé de tag1 ou 2
            int nb_tag1=0;
            int nb_tag2=0;
            //on dessine les marqueurs trouvés
            if (m_dbg_active)
                cv::aruco::drawDetectedMarkers(m_frameCloned, markerCorners, markerIds);
            //on estime leur position en 3D par rapport à la caméra
            cv::aruco::estimatePoseSingleMarkers(markerCorners, parameter.value[IDX_PARAM_ARUCO_TAILLE], camMatrix, distCoeffs, rvecs,tvecs); //param à la place de MarkerLength
            //tvecs[i][2] est la distance par rapport à la caméra
            //tvecs[i][0] est la l'abscisse, centre = 0, gauche négatif, droitep ositif
            //tvecs[i][1] est la hauteur. centre = 0, haut négatif, bas positif
            //on donne la valeur normale à la caméra dans la console pour cahque marqueur
            for(unsigned int i = 0; i < markerIds.size(); i++)
            {
                if(markerIds.at(i)==(int)(parameter.value[IDX_PARAM_ARUCO_TAG1]))
                {
                    if (m_dbg_active)
                        cv::drawFrameAxes(m_frameCloned, camMatrix, distCoeffs, rvecs[i], tvecs[i], parameter.value[IDX_PARAM_ARUCO_TAILLE] * 0.5f);
                   // double l_center = tvecs[i][2]*TAILLE_BALISE*cos()
                    double dist=sqrt(tvecs[i][0]*tvecs[i][0]+tvecs[i][2]*tvecs[i][2]);
                    float teta=0;
                    if(tvecs[i][2]!=0)
                        teta=atan(-tvecs[i][0]/tvecs[i][2]);
                   // teta =rvecs[i][0];
                   // qDebug() << rvecs[i][0] << rvecs[i][1] << rvecs[i][2];
                   // qDebug() << dist << teta*180.0/3.1415;
                    if (nb_tag1 >0)// si on en a déjà trouvé un, on fait le barycentre pour affiner le résultat
                    {
                        result.value[IDX_ROBOT1_DIST] = (result.value[IDX_ROBOT1_DIST] + dist)/2.0f;
                        result.value[IDX_ROBOT1_ANGLE] = (result.value[IDX_ROBOT1_ANGLE]+teta)/2.0f;
                    }
                    else // sinon on prend le résultat trouvé
                    {
                        result.value[IDX_ROBOT1_DIST] = dist;
                        result.value[IDX_ROBOT1_ANGLE]=teta;
                    }
                    nb_tag1++;
                    /*result.markers_detected = markerIds;*/
                }

                if(markerIds.at(i)==(int)(parameter.value[IDX_PARAM_ARUCO_TAG2]))
                {
                    if (m_dbg_active)
                        cv::drawFrameAxes(m_frameCloned, camMatrix, distCoeffs, rvecs[i], tvecs[i], parameter.value[IDX_PARAM_ARUCO_TAILLE] * 0.5f);
                    double dist=sqrt(tvecs[i][0]*tvecs[i][0]+tvecs[i][2]*tvecs[i][2]);
                    //result.value[IDX_ROBOT2_DIST] = dist;
                    float teta=0;
                    if(tvecs[i][2]!=0)
                        teta=atan(-tvecs[i][0]/tvecs[i][2]);
                    if (nb_tag2 >0)// si on en a déjà trouvé un, on fait le barycentre pour affiner le résultat
                    {
                        result.value[IDX_ROBOT2_DIST] = (result.value[IDX_ROBOT2_DIST] + dist)/2.0f;
                        result.value[IDX_ROBOT2_ANGLE] = (result.value[IDX_ROBOT2_ANGLE]+teta)/2.0f;
                    }
                    else // sinon on prend le résultat trouvé
                    {
                        result.value[IDX_ROBOT2_DIST] = dist;
                        result.value[IDX_ROBOT2_ANGLE]=teta;
                    }
                    nb_tag2++;
                    /*result.markers_detected = markerIds;*/
                }
            }
        }

        int fps=1000/(t.elapsed());
        result.m_fps=fps;

        //TIMESTAMP_MATCH.Timestamp

        QImage imgConst(m_iL,m_iH,QImage::Format_RGB888); //image du buffer video au format Qt

        //on affiche l'image traitée
        if (m_dbg_active)
        {
            //recuperation des donnees de la frame
            //et referencement dans l'image Qt
            //ATTENTION: pas de copie des donnees -> garbage collector
            //conversion de l'image en RGB
            frame_converted=m_frame.clone();
            cv::cvtColor(m_frameCloned,frame_converted, cv::COLOR_BGR2RGB);

            //affichage des droites de reperage
            //centre
            cv::line(frame_converted,cv::Point2f(m_iL/2,0),cv::Point2f(m_iL/2,m_iH),cv::Scalar(255,0,0),2);

            //affichage overlay
            char str[200];
            sprintf(str,"%d fps",fps);
            cv::putText(frame_converted, str, cv::Point2f(20,20), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(255,0,0,0));

            const uchar *qImageBuffer = (const uchar*)frame_converted.data;
            //const uchar *qImageBuffer = (const uchar*)m_frameCloned.data;
            //cv::imwrite("tata.jpg",frame);
            QImage img(qImageBuffer, m_iL, m_iH, QImage::Format_RGB888);

            //copie complete de l'image pour eviter une desallocation sauvage de la frame
            imgConst=img.copy(QRect(0,0,m_iL,m_iH));

        }

        if(parameter.record)
        {
            if (m_dbg_active)
            {
                cv::cvtColor(frame_converted,frame_record, cv::COLOR_RGB2BGR);
                _video_record(frame_record);//on enregistre le flux video
            }
            else
                _video_record(m_frame);
        }

        emit resultReady(result,imgConst);

        //QThread::msleep(5);
        inputImage.release();

       //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
        frame_converted.release();
        frame_record.release();
    }
    else
    {
        //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
    }
}

// ========================================================
void VideoWorker::_video_process_Balise(tVideoInput parameter)
{
    tVideoResult result;
    _init_tResult(&result);
    QTime t;
      t.start();


    //capture d'une image du flux video
    capture->grab();

    //récupération de l'image
    bool captureOK=capture->retrieve(m_frame,0);

    //l'image a-t-elle bien été récupérée
    if (captureOK)
    {
        cv::Mat frame_converted;
        cv::Mat3b frame_record;

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
            for(unsigned int i = 0; i < markerIds.size(); i++)
            {
                if((markerIds.at(i)==91)||(markerIds.at(i)==92))
                {
                    cv::drawFrameAxes(m_frameCloned, camMatrix, distCoeffs, rvecs[i], tvecs[i], markerLength * 0.5f);
                    double dist=sqrt(tvecs[i][0]*tvecs[i][0]+tvecs[i][2]*tvecs[i][2]);
                    result.value[IDX_ROBOT1_DIST] = dist;
                    float teta=0;
                    if(tvecs[i][2]!=0)
                        teta=atan(-tvecs[i][0]/tvecs[i][2]);
                    result.value[IDX_ROBOT1_ANGLE]=teta;
                    /*result.markers_detected = markerIds;*/
                }

                if((markerIds.at(i)==93)||(markerIds.at(i)==94))
                {
                    cv::drawFrameAxes(m_frameCloned, camMatrix, distCoeffs, rvecs[i], tvecs[i], markerLength * 0.5f);
                    double dist=sqrt(tvecs[i][0]*tvecs[i][0]+tvecs[i][2]*tvecs[i][2]);
                    result.value[IDX_ROBOT2_DIST] = dist;
                    float teta=0;
                    if(tvecs[i][2]!=0)
                        teta=atan(-tvecs[i][0]/tvecs[i][2]);
                    result.value[IDX_ROBOT2_ANGLE]=teta;
                    /*result.markers_detected = markerIds;*/
                }
            }
        }

        int fps=1000/(t.elapsed());
        result.m_fps=fps;

        //TIMESTAMP_MATCH.Timestamp

        QImage imgConst(m_iL,m_iH,QImage::Format_RGB888); //image du buffer video au format Qt

        //on affiche l'image traitée
        if (m_dbg_active)
        {
            //recuperation des donnees de la frame
            //et referencement dans l'image Qt
            //ATTENTION: pas de copie des donnees -> garbage collector
            //conversion de l'image en RGB
            frame_converted=m_frame.clone();
            cv::cvtColor(m_frameCloned,frame_converted, cv::COLOR_BGR2RGB);

            //affichage des droites de reperage
            //centre
            cv::line(frame_converted,cv::Point2f(m_iL/2,0),cv::Point2f(m_iL/2,m_iH),cv::Scalar(255,0,0),2);

            //affichage overlay
            char str[200];
            sprintf(str,"%d fps",fps);
            cv::putText(frame_converted, str, cv::Point2f(20,20), cv::FONT_HERSHEY_PLAIN, 2,  cv::Scalar(255,0,0,0));

            const uchar *qImageBuffer = (const uchar*)frame_converted.data;
            //const uchar *qImageBuffer = (const uchar*)m_frameCloned.data;
            //cv::imwrite("tata.jpg",frame);
            QImage img(qImageBuffer, m_iL, m_iH, QImage::Format_RGB888);

            //copie complete de l'image pour eviter une desallocation sauvage de la frame
            imgConst=img.copy(QRect(0,0,m_iL,m_iH));

        }

        if(parameter.record)
        {
            if (m_dbg_active)
            {
                cv::cvtColor(frame_converted,frame_record, cv::COLOR_RGB2BGR);
                _video_record(frame_record);//on enregistre le flux video
            }
            else
                _video_record(m_frame);
        }

        emit resultReady(result,imgConst);

        //QThread::msleep(5);
        inputImage.release();

       //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
        frame_converted.release();
        frame_record.release();
    }
    else
    {
        //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
    }
}

// ========================================================
//void VideoWorker::_video_process_ColorSequence(tVideoInput parameter)
//{
//    /*
//    http://fr.wikipedia.org/wiki/Teinte_Saturation_Valeur
//    Le modèle TSV pour Teinte Saturation Valeur (en anglais HSV pour Hue Saturation Value),
//    est un système de gestion des couleurs en informatique.

//    Teinte ou "H":
//    La teinte est codée suivant l'angle qui lui correspond sur le cercle des couleurs ==>
//    0° ou 360° : rouge ;
//    60° : jaune ;
//    120° : vert ;
//    180° : cyan ;
//    240° : bleu ;
//    300° : magenta;
//    ATTENTION ramené de 0 à 180 dans opencv

//    Saturation ou "S":
//    La saturation est l'« intensité » de la couleur :
//    elle varie entre 0 et 100 % ;
//    elle est parfois appelée « pureté » ;
//    plus la saturation d'une couleur est faible, plus l'image sera « grisée » et plus elle apparaîtra fade
//    ATTENTION codé sur un octet dans opencv (0/255)

//    Valeur ou "V"
//    La valeur est la « brillance » de la couleur :
//    elle varie entre 0 et 100 %
//    plus la valeur d'une couleur est faible, plus la couleur est sombre. Une valeur de 0 correspond au noir.
//    ATTENTION codé sur un octet dans opencv (0/255)
//    */

//    tVideoResult result;
//    _init_tResult(&result);
//    QTime t;
//    t.start();

//    //capture d'une image du flux video
//    capture->grab();

//    //récupération de l'image
//    bool captureOK=capture->retrieve(m_frame,0);

//    //par défaut : aucune séquence
// //    result.value[IDX_SEQUENCE]=SEQUENCE_UNKNOWN;

//    //l'image a-t-elle bien été récupérée
//    if (captureOK)
//    {
//        //images temporaires pour le traitement video
//        cv::Mat inputImage;
//        cv::Mat3b frameHSV;
//        cv::Mat3b inputRGB;
//        cv::Mat3b maskedRGB;
//        cv::Mat3b maskedRed;
//        cv::Mat3b maskedGreen;
//        cv::Mat1b maskRGB;
//        cv::Mat1b maskRed;
//        cv::Mat1b maskRed_1;
//        cv::Mat1b maskRed_2;
//        cv::Mat1b maskGreen;
//        cv::Mat3b frameBlur;
//        cv::Mat3b frameAffichage;

//        //dans le cas ou la configuration de la caméra aurait mal fonctionné
//        if(!parameterConfirmed)
//            _video_confirm_parameters(m_frame);

//        //clone de l'image pour la persistence des données
//        m_frameCloned=m_frame.clone();

//        if(parameter.record)
//            _video_record(m_frame);//on enregistre le flux video

//        //clone de l'image acquise de l'image
//        inputImage=m_frameCloned.clone();

//        //conversion en RGB pour l'affichage
//        cv::cvtColor(inputImage,inputRGB,CV_BGR2RGB);

//        //init des images filtrées
//        maskedRGB=cv::Mat::zeros(inputImage.rows, inputImage.cols, CV_32F);
//        maskedRed=cv::Mat::zeros(inputImage.rows, inputImage.cols, CV_32F);
//        maskedGreen=cv::Mat::zeros(inputImage.rows, inputImage.cols, CV_32F);

//        //floutage de l'image couleur pour enlever les pixels parasites
//        cv::GaussianBlur(inputImage,frameBlur,cv::Size(9,9),10.0,0,cv::BORDER_DEFAULT);

//        //Conversion de l'image floutée dans l'espace colorimetrique HSV
//        cv::cvtColor(frameBlur,frameHSV,CV_BGR2HSV);

//        //pour stocker les résultats
//        QList<int> combinaison_color;
//        QList<int> combinaison_x;
//        QList<int> combinaison_width;

//        //MAJ du zonage
//        //TODO récupérer des paramètres
//        /*int x_traitement=0;
//        int y_traitement=0;
//        int H_traitement=m_iH;
//        int L_traitement=m_iL;*/

//        //Nombre de pixels minimum à détecter
//        int nb_pixels_minimum=1000;
//        int nb_pixels_maximum=50000;
//        int iPureteRed=parameter.value[IDX_PARAM_PURETE_ROUGE];
//        int iEcartTeinteRed=parameter.value[IDX_PARAM_ECART_ROUGE];
//        int iPureteGreen=parameter.value[IDX_PARAM_PURETE_VERT];
//        int iEcartTeinteGreen=parameter.value[IDX_PARAM_ECART_VERT];

//        cv::Scalar minHSV = cv::Scalar(60-iEcartTeinteGreen, 255-iPureteGreen, 50);
//        cv::Scalar maxHSV = cv::Scalar(60+iEcartTeinteGreen, 255, 255);
//        cv::inRange(frameHSV, minHSV, maxHSV, maskGreen);
//        cv::bitwise_and(inputImage, inputImage, maskedGreen, maskGreen);

//        minHSV = cv::Scalar(0, 255-iPureteRed, 50);
//        maxHSV = cv::Scalar(0+iEcartTeinteRed, 255, 255);
//        cv::inRange(frameHSV, minHSV, maxHSV, maskRed_1);
//        minHSV = cv::Scalar(180-iEcartTeinteRed, 255-iPureteRed, 50);
//        maxHSV = cv::Scalar(180, 255, 255);
//        cv::inRange(frameHSV, minHSV, maxHSV, maskRed_2);
//        maskRed=maskRed_1|maskRed_2;
//        cv::bitwise_and(inputImage, inputImage, maskedRed, maskRed);

//        maskRGB = maskGreen | maskRed;
//        cv::bitwise_and(inputRGB, inputRGB, maskedRGB, maskRGB);

//        //séquence qui contiendra les contours des éléments verts trouvés
//        std::vector < std::vector<cv::Point> > contoursGreen;
//        cv::findContours(maskGreen,contoursGreen,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE);

//        for (size_t contourIdx = 0; contourIdx < contoursGreen.size(); contourIdx++)
//        {
//            std::vector<cv::Point> approx;
//            cv::approxPolyDP(contoursGreen[contourIdx], approx, 5, true);
//            double aire_detectee = cv::contourArea(approx);
//            if((aire_detectee>nb_pixels_minimum)&&(aire_detectee<nb_pixels_maximum))
//            {
//                //Rectangle circonscrit de la forme détectée
//                cv::Rect RectCirconscrit=cv::boundingRect( approx );

//                //barycentre du rectangle
//                int centre_x=RectCirconscrit.x+RectCirconscrit.width/2;
//                combinaison_x << centre_x;
//                combinaison_color << Qt::green;
//                combinaison_width << RectCirconscrit.width;
//                //float centre_y=RectCirconscrit.y+RectCirconscrit.height/2;
//                //QPoint centre=QPoint((centre_x/m_iL)*100,(centre_y/m_iH)*100);

//                cv::rectangle(inputRGB,RectCirconscrit,cv::COLOR_RGB(0,255,0),5,8,0);
//            }
//        }

//        //séquence qui contiendra les contours des éléments verts trouvés
//        std::vector < std::vector<cv::Point> > contoursRed;
//        cv::findContours(maskRed,contoursRed,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE);

//        for (size_t contourIdx = 0; contourIdx < contoursRed.size(); contourIdx++)
//        {
//            std::vector<cv::Point> approx;
//            cv::approxPolyDP(contoursRed[contourIdx], approx, 5, true);
//            double aire_detectee = cv::contourArea(approx);
//            if((aire_detectee>nb_pixels_minimum)&&(aire_detectee<nb_pixels_maximum))
//            {
//                //Rectangle circonscrit de la forme détectée
//                cv::Rect RectCirconscrit=cv::boundingRect( approx );

//                //barycentre du rectangle
//                int centre_x=RectCirconscrit.x+RectCirconscrit.width/2;
//                combinaison_x << centre_x;
//                combinaison_color << Qt::red;
//                combinaison_width << RectCirconscrit.width;
//                //float centre_y=RectCirconscrit.y+RectCirconscrit.height/2;
//                //QPoint centre=QPoint((centre_x/m_iL)*100,(centre_y/m_iH)*100);

//                cv::rectangle(inputRGB,RectCirconscrit,cv::COLOR_RGB(0,0,255),5,8,0);
//            }
//        }


//        //on analyse les résultats
//        if(!combinaison_x.isEmpty())
//        {
//            //on réordonne selon les x croissants
//            for(int i=combinaison_x.count()-1;i>0;i--)
//                for(int j=0;j<i;j++)
//                {
//                    if(combinaison_x[j+1] < combinaison_x[j])
//                    {
//                        combinaison_x.swap(j+1,j);
//                        combinaison_color.swap(j+1,j);
//                        combinaison_width.swap(j+1,j);
//                    }
//                }

//            /*for(int i=0;i<combinaison_x.count();i++)
//                qDebug() << (combinaison_color[i]==Qt::green?"VERT ":"ROUGE ") << combinaison_width[i];
//            qDebug() << "\n\n";*/

//            //coupe 2020 : cf règlement page 11
//            //CAS N°1:
//            //côté BLEU: GRGGR   côté JAUNE: GRRGR
//            //CAS N°2:
//            //côté BLEU: GGRGR   côté JAUNE: GRGRR
//            //CAS N°3:
//            //côté BLEU: GGGRR   côté JAUNE: GGRRR

//            //largeur de tous les gobelets détectés
//            int largeur_totale=0;
//            for(int i=0;i<combinaison_width.count();i++)
//                largeur_totale=largeur_totale+combinaison_width[i];
//            //largeur moyenne d'un gobelet
//            int largeur_gobelet = largeur_totale/5;
//            QString str_combinaison;
//            for(int i=0;i<combinaison_width.count();i++)
//            {
//                float nb_gobelet_estim = combinaison_width[i] / largeur_gobelet;
//                if(nb_gobelet_estim < 1.4)
//                    str_combinaison=str_combinaison+(combinaison_color[i]==Qt::green?"G":"R");
//                else if((nb_gobelet_estim > 1.6)&&(nb_gobelet_estim < 2.4))
//                    str_combinaison=str_combinaison+(combinaison_color[i]==Qt::green?"GG":"RR");
//                else
//                    str_combinaison=str_combinaison+(combinaison_color[i]==Qt::green?"GGG":"RRR");
//            }

//            //qDebug() << str_combinaison <<"\n";

//            if(str_combinaison=="GRGGR")
//                result.value[IDX_SEQUENCE]=SEQUENCE_GRGGR;
//            if(str_combinaison=="GRRGR")
//                result.value[IDX_SEQUENCE]=SEQUENCE_GRRGR;
//            if(str_combinaison=="GGRGR")
//                result.value[IDX_SEQUENCE]=SEQUENCE_GGRGR;
//            if(str_combinaison=="GRGRR")
//                result.value[IDX_SEQUENCE]=SEQUENCE_GRGRR;
//            if(str_combinaison=="GGGRR")
//                result.value[IDX_SEQUENCE]=SEQUENCE_GGGRR;
//            if(str_combinaison=="GGRRR")
//                result.value[IDX_SEQUENCE]=SEQUENCE_GGRRR;
//        }

//        int fps=1000/(t.elapsed());
//        result.m_fps=fps;

//        //TIMESTAMP_MATCH.Timestamp

//        QImage imgConst(m_iL,m_iH,QImage::Format_RGB888); //image du buffer video au format Qt

//        //on affiche l'image traitée
//        if (m_dbg_active)
//        {
//            //recuperation des donnees de la frame
//            //et referencement dans l'image Qt
//            //ATTENTION: pas de copie des donnees -> garbage collector

//            //affichage overlay
//            char str[200];
//            sprintf(str,"%d fps",fps);
//            cv::putText(maskedRGB, str, cv::Point2f(20,20), cv::FONT_HERSHEY_PLAIN, 2,  cv::Scalar(255,0,0,0));

//            const uchar *qImageBuffer = (const uchar*)inputRGB.data;
//            QImage img(qImageBuffer, m_iL, m_iH, QImage::Format_RGB888);
//            //const uchar *qImageBuffer = (const uchar*)frameGrey_green.data;
//            //QImage img(qImageBuffer, m_iL, m_iH, QImage::Format_Grayscale8);

//            //copie complete de l'image pour eviter une desallocation sauvage de la frame
//            imgConst=img.copy(QRect(0,0,m_iL,m_iH));
//        }

//        emit resultReady(result,imgConst);

//        //QThread::msleep(5);
//        inputImage.release();

//       //opencv ne profite pas du garbage collector de Qt
//        m_frame.release();
//        m_frameCloned.release();
//        inputImage.release();
//        inputRGB.release();
//        frameBlur.release();
//        frameHSV.release();
//        maskedRGB.release();
//        maskedRed.release();
//        maskedGreen.release();
//        maskRGB.release();
//        maskRed.release();
//        maskRed_1.release();
//        maskRed_2.release();
//        maskGreen.release();
//        frameAffichage.release();
//    }
//    else
//    {
//        //opencv ne profite pas du garbage collector de Qt
//        m_frame.release();
//        m_frameCloned.release();
//    }
//}

void VideoWorker::_video_process_Calibration(tVideoInput parameter)
{
    tVideoResult result;
    _init_tResult(&result);
    QTime t;
    t.start();

    //capture d'une image du flux video
    capture->grab();

    //récupération de l'image
    bool captureOK=capture->retrieve(m_frame,0);

    //l'image a-t-elle bien été récupérée
    if (captureOK)
    {
        //images temporaires pour le traitement video
        cv::Mat inputImage;
        cv::Mat3b frameHSV;
        cv::Mat3b inputRGB;
        cv::Mat3b maskedRGB;
        cv::Mat3b maskedRed;
        cv::Mat3b maskedGreen;
        cv::Mat1b maskRGB;
        cv::Mat1b maskRed;
        cv::Mat1b maskRed_1;
        cv::Mat1b maskRed_2;
        cv::Mat1b maskGreen;
        cv::Mat3b frameBlur;
        cv::Mat3b frameAffichage;
        cv::Mat3b charucoFrame;

        //dans le cas ou la configuration de la caméra aurait mal fonctionné
        if(!parameterConfirmed)
            _video_confirm_parameters(m_frame);

        if(m_charucoIsSet)
            result.value[IDX_CHARUCO_IS_SET]=1.;
        else
            result.value[IDX_CHARUCO_IS_SET]=0.;

        if((m_internal_param[IDX_PARAM_CALIB_TYPE]==1.)&&!m_charucoIsSet)
        {
            m_charucoFinish=(m_internal_param[IDX_PARAM_SET_CHARUCO_FRAME]==1.);
            if(!m_charucoInit)
            {
                _charucoInit();
                m_charucoInit=true;
            }
            if(m_charucoInit)
            {
                charucoFrame=m_frame.clone();
                maskedRGB=_charucoProcessing(charucoFrame);
            }
            if(m_charucoFinish && m_charucoInit)
            {
                _charucoFinish();
                m_charucoIsSet=true;
            }
        }
        else
        {
            //clone de l'image pour la persistence des données
            m_frameCloned=m_frame.clone();

            if(parameter.record)
                _video_record(m_frame);//on enregistre le flux video

            //clone de l'image acquise de l'image
            inputImage=m_frameCloned.clone();

            //conversion en RGB pour l'affichage
            cv::cvtColor(inputImage,inputRGB,cv::COLOR_BGR2RGB);

            //init des images filtrées
            maskedRGB=cv::Mat::zeros(inputImage.rows, inputImage.cols, CV_32F);
            maskedRed=cv::Mat::zeros(inputImage.rows, inputImage.cols, CV_32F);
            maskedGreen=cv::Mat::zeros(inputImage.rows, inputImage.cols, CV_32F);

            //floutage de l'image couleur pour enlever les pixels parasites
            cv::GaussianBlur(inputImage,frameBlur,cv::Size(9,9),10.0,0,cv::BORDER_DEFAULT);

            //Conversion de l'image floutée dans l'espace colorimetrique HSV
            cv::cvtColor(frameBlur,frameHSV,cv::COLOR_BGR2HSV);

            //Nombre de pixels minimum à détecter
            int nb_pixels_minimum=m_internal_param[IDX_PARAM_PIXEL_MIN];
            int nb_pixels_maximum=m_internal_param[IDX_PARAM_PIXEL_MAX];
            int iPureteRed=m_internal_param[IDX_PARAM_PURETE_ROUGE];
            int iEcartTeinteRed=m_internal_param[IDX_PARAM_ECART_ROUGE];
            int iPureteGreen=m_internal_param[IDX_PARAM_PURETE_VERT];
            int iEcartTeinteGreen=m_internal_param[IDX_PARAM_ECART_VERT];

            cv::Scalar minHSV = cv::Scalar(60-iEcartTeinteGreen, 255-iPureteGreen, 50);
            cv::Scalar maxHSV = cv::Scalar(60+iEcartTeinteGreen, 255, 255);
            cv::inRange(frameHSV, minHSV, maxHSV, maskGreen);
            cv::bitwise_and(inputImage, inputImage, maskedGreen, maskGreen);

            minHSV = cv::Scalar(0, 255-iPureteRed, 50);
            maxHSV = cv::Scalar(0+iEcartTeinteRed, 255, 255);
            cv::inRange(frameHSV, minHSV, maxHSV, maskRed_1);
            minHSV = cv::Scalar(180-iEcartTeinteRed, 255-iPureteRed, 50);
            maxHSV = cv::Scalar(180, 255, 255);
            cv::inRange(frameHSV, minHSV, maxHSV, maskRed_2);
            maskRed=maskRed_1|maskRed_2;
            cv::bitwise_and(inputImage, inputImage, maskedRed, maskRed);

            maskRGB = maskGreen | maskRed;
            cv::bitwise_and(inputRGB, inputRGB, maskedRGB, maskRGB);

            //séquence qui contiendra les contours des éléments verts trouvés
            std::vector < std::vector<cv::Point> > contoursGreen;
            cv::findContours(maskGreen,contoursGreen,cv::RETR_CCOMP,cv::CHAIN_APPROX_SIMPLE);

            for (size_t contourIdx = 0; contourIdx < contoursGreen.size(); contourIdx++)
            {
                std::vector<cv::Point> approx;
                cv::approxPolyDP(contoursGreen[contourIdx], approx, 5, true);
                double aire_detectee = cv::contourArea(approx);
                if((aire_detectee>nb_pixels_minimum)&&(aire_detectee<nb_pixels_maximum))
                {
                    //Rectangle circonscrit de la forme détectée
                    cv::Rect RectCirconscrit=cv::boundingRect( approx );
                    cv::rectangle(maskedRGB,RectCirconscrit,cv::Scalar(0,255,0),5,8,0);
                }
            }

            //séquence qui contiendra les contours des éléments verts trouvés
            std::vector < std::vector<cv::Point> > contoursRed;
            cv::findContours(maskRed,contoursRed,cv::RETR_CCOMP,cv::CHAIN_APPROX_SIMPLE);

            for (size_t contourIdx = 0; contourIdx < contoursRed.size(); contourIdx++)
            {
                std::vector<cv::Point> approx;
                cv::approxPolyDP(contoursRed[contourIdx], approx, 5, true);
                double aire_detectee = cv::contourArea(approx);
                if((aire_detectee>nb_pixels_minimum)&&(aire_detectee<nb_pixels_maximum))
                {
                    //Rectangle circonscrit de la forme détectée
                    cv::Rect RectCirconscrit=cv::boundingRect( approx );
                    cv::rectangle(maskedRGB,RectCirconscrit,cv::Scalar(0,0,255),5,8,0);
                }
            }
        }

        int fps=1000/(t.elapsed());
        result.m_fps=fps;

        QImage imgConst(m_iL,m_iH,QImage::Format_RGB888); //image du buffer video au format Qt
        //on affiche l'image traitée
        if (m_dbg_active)
        {
            //recuperation des donnees de la frame
            //et referencement dans l'image Qt
            //ATTENTION: pas de copie des donnees -> garbage collector

            //affichage overlay
            char str[200];
            sprintf(str,"%d fps",fps);
            cv::putText(maskedRGB, str, cv::Point2f(20,20), cv::FONT_HERSHEY_PLAIN, 2,  cv::Scalar(255,0,0,0));

            const uchar *qImageBuffer = (const uchar*)maskedRGB.data;
            QImage img(qImageBuffer, m_iL, m_iH, QImage::Format_RGB888);

            //copie complete de l'image pour eviter une desallocation sauvage de la frame
            imgConst=img.copy(QRect(0,0,m_iL,m_iH));
        }

        emit resultReady(result,imgConst);

       //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
        inputImage.release();
        inputRGB.release();
        frameBlur.release();
        frameHSV.release();
        maskedRGB.release();
        maskedRed.release();
        maskedGreen.release();
        maskRGB.release();
        maskRed.release();
        maskRed_1.release();
        maskRed_2.release();
        maskGreen.release();
        frameAffichage.release();
    }
    else
    {
        //opencv ne profite pas du garbage collector de Qt
        m_frame.release();
        m_frameCloned.release();
    }
}

void VideoWorker::_video_record(cv::Mat frametoRecord)
{
    if(!recordInitialized)
    {
        //construction du nom de fichier d'enregistrement de la video
        QString pathfilename = getVideoLogFilename();

        //création de l'objet OpenCV qui enregistre des frame
        record= new cv::VideoWriter(pathfilename.toStdString(),cv::VideoWriter::fourcc('X','V','I','D'),15,cv::Size(m_iL,m_iH),true);

        if(record->isOpened())
        {
            if(m_dbg_active)
                qDebug() << "[CImageProcessing] Enregistrement dans " << pathfilename;
            recordInitialized=true;
        }
        else
            if(m_dbg_active)
                qDebug() << "[CImageProcessing] Enregistrement impossible dans" << pathfilename;
    }
    record->write(frametoRecord);//on enregistre le flux video
}

void VideoWorker::_video_confirm_parameters(cv::Mat frameSample)
{
    //recuperation des hauteurs et largeur de l'image
    m_iH = frameSample.rows;
    m_iL = frameSample.cols;
    //qDebug() << "Récupération des paramètres Vidéos";
    //qDebug() << "Résolution " << m_iH << "x" << m_iL;
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

void VideoWorker::_init_tResult(tVideoResult * parameter)
{
    parameter->m_fps=0;
    for(int i=0;i<20;i++)
        parameter->value[i]=0.;
}

void VideoWorker::_charucoInit(void)
{
    //nettoyage des std::vector
    m_charucoProcessing.allCorners.clear();
    m_charucoProcessing.allIds.clear();
    m_charucoProcessing.allImgs.clear();

    //bitfield de la calibration charuco
    m_charucoProcessing.calibrationFlags = 0;
    m_charucoProcessing.aspectRatio = 1;
    if(m_charucoCalibParam._a>0.) {
        m_charucoProcessing.calibrationFlags |= cv::CALIB_FIX_ASPECT_RATIO;
        m_charucoProcessing.aspectRatio = m_charucoCalibParam._a;
    }
    if(m_charucoCalibParam._zt) m_charucoProcessing.calibrationFlags |= cv::CALIB_ZERO_TANGENT_DIST;
    if(m_charucoCalibParam._pc) m_charucoProcessing.calibrationFlags |= cv::CALIB_FIX_PRINCIPAL_POINT;

    //Initialisation des paramètres de détection
    m_charucoProcessing.detectorParams = cv::aruco::DetectorParameters::create();
    //Initialisation du dictionnaire utilisé
    m_charucoProcessing.dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(m_charucoCalibParam._d));

    //Création de l'échiquier charuco
    m_charucoProcessing.charucoboard = cv::aruco::CharucoBoard::create(m_charucoCalibParam._w, m_charucoCalibParam._h,
                                                                       m_charucoCalibParam._sl, m_charucoCalibParam._ml,
                                                                       m_charucoProcessing.dictionary);
    m_charucoProcessing.board = m_charucoProcessing.charucoboard.staticCast<cv::aruco::Board>();
}

cv::Mat3b VideoWorker::_charucoProcessing(cv::Mat3b inputFrame)
{
    cv::Mat3b imageCopy;
    bool refindStrategy =m_charucoCalibParam._rs;

    std::vector< int > ids;
    std::vector< std::vector< cv::Point2f > > corners, rejected;

    //détection des marqueurs aruco
    cv::aruco::detectMarkers(inputFrame, m_charucoProcessing.dictionary, corners, ids, m_charucoProcessing.detectorParams, rejected);

    //seconde passe (si activée) pour retrouver plus de marqueurs
    if(refindStrategy) cv::aruco::refineDetectedMarkers(inputFrame, m_charucoProcessing.board, corners, ids, rejected);

    //inerpolation des coins de l'échiquier charuco
    cv::Mat currentCharucoCorners, currentCharucoIds;
    if(ids.size()>0)
        cv::aruco::interpolateCornersCharuco(corners, ids, inputFrame, m_charucoProcessing.charucoboard, currentCharucoCorners,currentCharucoIds);

    //on affiche les marqueurs
    inputFrame.copyTo(imageCopy);
    if(ids.size()>0)
        cv::aruco::drawDetectedMarkers(imageCopy, corners);

    //si on bien trouvé les coins de l'échiquier on les affiche
    if(currentCharucoCorners.total()>0)
        cv::aruco::drawDetectedCornersCharuco(imageCopy, currentCharucoCorners, currentCharucoIds);

    //on affiche un rond vert pour indiquer que l'image est correcte pour la calibration
    cv::circle( imageCopy, cv::Point(10, 20), 20, cv::Scalar(0,255,0), -1, 8, 0 );


    //si on veut enregistrer l'image
    if((m_internal_param[IDX_PARAM_GET_CHARUCO_FRAME]==1) && ids.size() > 0)
    {
        m_charucoProcessing.allCorners.push_back(corners);
        m_charucoProcessing.allIds.push_back(ids);
        m_charucoProcessing.allImgs.push_back(inputFrame);
        m_charucoProcessing.imgSize = inputFrame.size();
        m_internal_param[IDX_PARAM_GET_CHARUCO_FRAME]=0;
    }

    return imageCopy;
}

void VideoWorker::_charucoFinish(void)
{
    //Vérification qu'on a assez de marqueurs pour cobstruire le fichier de calibration
    if(m_charucoProcessing.allIds.size() < 1)
    {
        qDebug() << "[CImageProcessing] Pas assez de marqueurs aruco trouvés pour construire le fichier de calibration";
        return;
    }

    cv::Mat cameraMatrix, distCoeffs;
    std::vector< cv::Mat > rvecs, tvecs;

    if(m_charucoProcessing.calibrationFlags & cv::CALIB_FIX_ASPECT_RATIO) {
        cameraMatrix = cv::Mat::eye(3, 3, CV_64F);
        cameraMatrix.at< double >(0, 0) = m_charucoProcessing.aspectRatio;
    }

    // prepare data for calibration
    std::vector< std::vector< cv::Point2f > > allCornersConcatenated;
    std::vector< int > allIdsConcatenated;
    std::vector< int > markerCounterPerFrame;
    markerCounterPerFrame.reserve(m_charucoProcessing.allCorners.size());
    for(unsigned int i = 0; i < m_charucoProcessing.allCorners.size(); i++)
    {
        markerCounterPerFrame.push_back((int)m_charucoProcessing.allCorners[i].size());
        for(unsigned int j = 0; j < m_charucoProcessing.allCorners[i].size(); j++)
        {
            allCornersConcatenated.push_back(m_charucoProcessing.allCorners[i][j]);
            allIdsConcatenated.push_back(m_charucoProcessing.allIds[i][j]);
        }
    }

    // calibrate camera using aruco markers
    double arucoRepErr;
    arucoRepErr = cv::aruco::calibrateCameraAruco(allCornersConcatenated, allIdsConcatenated,
                                              markerCounterPerFrame, m_charucoProcessing.board, m_charucoProcessing.imgSize, cameraMatrix,
                                              distCoeffs, cv::noArray(), cv::noArray(), m_charucoProcessing.calibrationFlags);

    // prepare data for charuco calibration
    int nFrames = (int)m_charucoProcessing.allCorners.size();
    std::vector< cv::Mat > allCharucoCorners;
    std::vector< cv::Mat > allCharucoIds;
    std::vector< cv::Mat > filteredImages;
    allCharucoCorners.reserve(nFrames);
    allCharucoIds.reserve(nFrames);

    for(int i = 0; i < nFrames; i++) {
        // interpolate using camera parameters
        cv::Mat currentCharucoCorners, currentCharucoIds;
        cv::aruco::interpolateCornersCharuco(m_charucoProcessing.allCorners[i], m_charucoProcessing.allIds[i], m_charucoProcessing.allImgs[i], m_charucoProcessing.charucoboard,
                                         currentCharucoCorners, currentCharucoIds, cameraMatrix,
                                         distCoeffs);

        allCharucoCorners.push_back(currentCharucoCorners);
        allCharucoIds.push_back(currentCharucoIds);
        filteredImages.push_back(m_charucoProcessing.allImgs[i]);
    }

    if(allCharucoCorners.size() < 4)
    {
        qDebug() << "[CImageProcessing] Pas assez de coins d'échiquier trouvés pour construire le fichier de calibration";
        //return 0;
    }

    // calibrate camera using charuco
    double repError;
    repError =
        cv::aruco::calibrateCameraCharuco(allCharucoCorners, allCharucoIds, m_charucoProcessing.charucoboard, m_charucoProcessing.imgSize,
                                      cameraMatrix, distCoeffs, rvecs, tvecs, m_charucoProcessing.calibrationFlags);

    //enregistrement du fichier de calibration
    bool saveOk =  _saveCameraParams(m_charucoProcessing.imgSize, m_charucoProcessing.aspectRatio, m_charucoProcessing.calibrationFlags,
                                    cameraMatrix, distCoeffs, repError);
    if(!saveOk)
        qDebug() << "[CImageProcessing] Impossible de sauvegarder le fichier de calibration";

    qDebug() << "[CImageProcessing] Valeur de retour calibration Charuco: " << repError;
    qDebug() << "[CImageProcessing] Valeur de retour calibration aruco: " << arucoRepErr;
}

bool VideoWorker::_saveCameraParams(cv::Size imageSize, float aspectRatio, int flags,
                                           const cv::Mat &cameraMatrix, const cv::Mat &distCoeffs, double totalAvgErr)
{
    QString outputFile = m_path_parameter_file;
    cv::FileStorage fs(outputFile.toStdString(), cv::FileStorage::WRITE);
    if(!fs.isOpened())
        return false;

    time_t tt;
    time(&tt);
    struct tm *t2 = localtime(&tt);
    char buf[1024];
    strftime(buf, sizeof(buf) - 1, "%c", t2);

    fs << "calibration_time" << buf;

    fs << "image_width" << imageSize.width;
    fs << "image_height" << imageSize.height;

    if(flags & cv::CALIB_FIX_ASPECT_RATIO) fs << "aspectRatio" << aspectRatio;

    if(flags != 0) {
        sprintf(buf, "flags: %s%s%s%s",
                flags & cv::CALIB_USE_INTRINSIC_GUESS ? "+use_intrinsic_guess" : "",
                flags & cv::CALIB_FIX_ASPECT_RATIO ? "+fix_aspectRatio" : "",
                flags & cv::CALIB_FIX_PRINCIPAL_POINT ? "+fix_principal_point" : "",
                flags & cv::CALIB_ZERO_TANGENT_DIST ? "+zero_tangent_dist" : "");
    }

    fs << "flags" << flags;

    fs << "camera_matrix" << cameraMatrix;
    fs << "distortion_coefficients" << distCoeffs;

    fs << "avg_reprojection_error" << totalAvgErr;

    qDebug() << "[CImageProcessing] Fichier de calibration sauvegardé dans " << outputFile;

    return true;
}
